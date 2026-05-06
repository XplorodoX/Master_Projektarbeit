#!/usr/bin/env python3
"""Graphical launcher for Stoneforge RL — with live training metrics."""
from __future__ import annotations

import glob
import os
import platform
import queue
import re
import shlex
import shutil
import tempfile
import subprocess
import sys
import threading
import tkinter as tk
from tkinter import filedialog, scrolledtext, ttk
from typing import Optional

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Windows uses Scripts/python.exe, Unix uses bin/python3
_IS_WIN = platform.system() == "Windows"
if _IS_WIN:
    VENV_PY = os.path.join(ROOT, ".venv", "Scripts", "python.exe")
else:
    VENV_PY = os.path.join(ROOT, ".venv", "bin", "python3")

PY = VENV_PY if os.path.exists(VENV_PY) else shutil.which("python3") or "python"
BUILD_DIR = os.path.join(ROOT, "build")

# Windows executables have .exe extension
_EXE_NAME = "stoneforge_client.exe" if _IS_WIN else "stoneforge_client"
GAME_BINARY = os.path.join(BUILD_DIR, _EXE_NAME)

DEFAULT_DQN = os.path.join(ROOT, "best_models_dqn", "best_model.zip")
DEFAULT_PPO = os.path.join(ROOT, "best_models_ppo", "best_model.zip")


def _quote(path: str) -> str:
    """Quote a path for the current platform's shell.
    
    Windows uses double quotes, Unix uses single quotes with shlex.quote().
    This is critical for paths with spaces.
    """
    if _IS_WIN:
        # Windows: use double quotes (don't use shlex.quote, it uses Unix rules)
        return f'"{path}"'
    else:
        # Unix/macOS/Linux: use shlex.quote for proper escaping
        return shlex.quote(path)


def _make_env() -> dict[str, str]:
    env = os.environ.copy()
    extra = os.pathsep.join([BUILD_DIR, os.path.join(ROOT, "python")])
    cur = env.get("PYTHONPATH", "")
    env["PYTHONPATH"] = f"{extra}{os.pathsep}{cur}" if cur else extra
    return env


def _ensure_requirements() -> bool:
    """Ensure all Python requirements and C++ bindings are available. Returns True if successful."""
    # First check if stoneforge_sim module exists
    if _find_so() is None:
        print("[launcher] Python binding (stoneforge_sim) not found. Building...")
        if not _build_bindings_cli():
            print("[launcher] ✗ Failed to build Python bindings", file=sys.stderr)
            return False
    
    # Check for critical Python modules
    req_file = os.path.join(ROOT, "python", "requirements.txt")
    if os.path.exists(req_file):
        critical_modules = ["gymnasium", "stable_baselines3", "numpy", "tensorboard"]
        missing_modules = []
        
        for module in critical_modules:
            try:
                __import__(module)
            except ImportError:
                missing_modules.append(module)
        
        if missing_modules:
            # Install requirements
            print(f"[launcher] Missing modules: {', '.join(missing_modules)}")
            print(f"[launcher] Installing requirements from {req_file}...")
            
            import subprocess as sp
            result = sp.run(
                [PY, "-m", "pip", "install", "-r", req_file],
                cwd=ROOT,
                encoding='utf-8',
                errors='replace',
            )
            
            if result.returncode != 0:
                print("[launcher] ✗ Failed to install requirements", file=sys.stderr)
                return False
    
    print("[launcher] ✓ All requirements satisfied")
    return True


def _build_bindings_cli() -> bool:
    """Build Python bindings from command line (non-GUI). Returns True if successful."""
    import subprocess as sp
    
    # Configure
    rc = sp.run(
        f"cmake -S {_quote(ROOT)} -B {_quote(BUILD_DIR)}"
        " -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON",
        shell=True,
        cwd=ROOT,
        encoding='utf-8',
        errors='replace',
    ).returncode
    
    if rc != 0:
        return False
    
    # Build
    build_cmd = f"cmake --build {_quote(BUILD_DIR)} --target stoneforge_sim -j4"
    if _IS_WIN:
        build_cmd += " --config Release"

    rc = sp.run(
        build_cmd,
        shell=True,
        cwd=ROOT,
        encoding='utf-8',
        errors='replace',
    ).returncode
    
    if rc != 0:
        return False
    
    # Copy to python/ folder
    built = _find_so()
    if built:
        dest_dir = os.path.join(ROOT, "python")
        dest = os.path.join(dest_dir, os.path.basename(built))
        try:
            if os.path.abspath(built) != os.path.abspath(dest):
                shutil.copy2(built, dest)
                print(f"[launcher] ✓ Copied {os.path.basename(built)} to python/")
        except Exception as e:
            print(f"[launcher] ⚠ Failed to copy module: {e}")
            return False
    else:
        ext = ".pyd" if _IS_WIN else ".so"
        print(f"[launcher] ✗ Build succeeded but {ext} not found.", file=sys.stderr)
        return False
    
    return True


def _find_so() -> Optional[str]:
    """Find the built stoneforge_sim module (.pyd on Windows, .so on Unix)."""
    if _IS_WIN:
        # Windows: look for .pyd (Python Dynamic module)
        patterns = [
            os.path.join(BUILD_DIR, "Release", "stoneforge_sim*.pyd"),
            os.path.join(BUILD_DIR, "Debug", "stoneforge_sim*.pyd"),
            os.path.join(BUILD_DIR, "stoneforge_sim*.pyd"),
            os.path.join(ROOT, "python", "stoneforge_sim*.pyd"),
        ]
    else:
        # Unix: look for .so (shared object)
        patterns = [
            os.path.join(BUILD_DIR, "stoneforge_sim*.so"),
            os.path.join(ROOT, "python", "stoneforge_sim*.so"),
        ]
    
    for pattern in patterns:
        m = glob.glob(pattern)
        if m:
            return m[0]
    return None


def _scan_models() -> list[tuple[str, str]]:
    """Scan the project for trained .zip models. Returns (display_name, abs_path) pairs."""
    seen: set[str] = set()
    results: list[tuple[str, str]] = []

    search_patterns = [
        # best_models_dqn/ , best_models_ppo/  — bestes Modell pro Lauf
        os.path.join(ROOT, "best_models_*", "*.zip"),
        # checkpoints/ — Snapshots während des Trainings
        os.path.join(ROOT, "checkpoints", "*.zip"),
        # Root-Level Modelle (alte Konvention)
        os.path.join(ROOT, "*.zip"),
    ]

    for pattern in search_patterns:
        for path in sorted(glob.glob(pattern)):
            abs_path = os.path.abspath(path)
            if abs_path in seen:
                continue
            seen.add(abs_path)

            rel = os.path.relpath(abs_path, ROOT)
            parts = rel.split(os.sep)
            stem = os.path.splitext(parts[-1])[0]

            if len(parts) == 2:
                folder = parts[0]
                # "best_models_dqn" → "DQN", "best_models_ppo" → "PPO"
                algo = (folder.replace("best_models_", "")
                              .replace("models_", "")
                              .upper())
                label = f"{algo}  —  {stem}"
            elif len(parts) == 1:
                # root-level zip: guess algo from filename
                low = stem.lower()
                if "dqn" in low:
                    label = f"DQN  —  {stem}"
                elif "ppo" in low:
                    label = f"PPO  —  {stem}"
                else:
                    label = stem
            else:
                label = rel

            results.append((label, abs_path))

    return results


# ---------------------------------------------------------------------------
# Theme
# ---------------------------------------------------------------------------
BG    = "#1e1e2e"
BG2   = "#181825"
BG3   = "#313244"
BG4   = "#45475a"
ACCENT  = "#cba6f7"
ACCENT2 = "#89b4fa"
GREEN   = "#a6e3a1"
RED     = "#f38ba8"
ORANGE  = "#fab387"
YELLOW  = "#f9e2af"
TEXT    = "#cdd6f4"
DIM     = "#6c7086"
BTN_FG  = "#1e1e2e"

SPINNER_FRAMES = ("⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏")

# Regex patterns to parse SB3 / curriculum output
_RE_SB3_KV   = re.compile(r"\|\s*([\w/]+)\s*\|\s*([-\d.]+)\s*\|")
_RE_EVAL     = re.compile(r"episode_reward=\s*([-\d.]+)")
_RE_CURR     = re.compile(r"\[Curriculum\] Stufe (\d+)/(\d+).*?(\d+).*?(\d+) Tiles")
_RE_BEST     = re.compile(r"New best mean reward", re.IGNORECASE)
_RE_50SEED   = re.compile(r"success=(\d+)/50.*?mean_len=([\d.]+)")


# ---------------------------------------------------------------------------
# Stat Card Widget
# ---------------------------------------------------------------------------
class StatCard(ttk.Frame):
    """A small metric display card with a large value and a label below."""

    def __init__(self, parent: tk.Widget, label: str, unit: str = "",
                 color: str = TEXT) -> None:
        super().__init__(parent, style="Card.TFrame", padding=(14, 10))
        self._unit = unit
        self._color = color

        self._val_var = tk.StringVar(value="—")
        val_lbl = tk.Label(self, textvariable=self._val_var,
                           bg=BG3, fg=color,
                           font=("Helvetica", 22, "bold"))
        val_lbl.pack()

        tk.Label(self, text=label, bg=BG3, fg=DIM,
                 font=("Helvetica", 9)).pack()

    def set(self, value: str) -> None:
        self._val_var.set(value)


# ---------------------------------------------------------------------------
# Model Picker Widget
# ---------------------------------------------------------------------------
class ModelPicker(tk.Frame):
    """Dropdown that lists all trained .zip models found in the project.

    Usage:
        picker = ModelPicker(parent)
        path = picker.get_path()   # None if nothing found/selected
    """

    def __init__(self, parent: tk.Widget, bg_color: str = BG3) -> None:
        super().__init__(parent, bg=bg_color)
        self.columnconfigure(0, weight=1)

        self._paths: list[str] = []
        self._names: list[str] = []

        # Combobox (read-only — user picks from list or uses … for custom)
        self._combo_var = tk.StringVar()
        self._combo = ttk.Combobox(
            self, textvariable=self._combo_var,
            state="readonly", font=("Helvetica", 10),
        )
        self._combo.grid(row=0, column=0, sticky="ew", padx=(0, 4))

        # Refresh button
        ttk.Button(self, text="↻", style="Secondary.TButton", width=3,
                   command=self.refresh).grid(row=0, column=1, padx=(0, 4))

        # Browse for a file not in the list
        ttk.Button(self, text="…", style="Secondary.TButton", width=3,
                   command=self._browse).grid(row=0, column=2)

        self.refresh()

    # ------------------------------------------------------------------

    def refresh(self) -> None:
        """Re-scan the project for model files and update the dropdown."""
        models = _scan_models()
        if models:
            self._names = [m[0] for m in models]
            self._paths = [m[1] for m in models]
            self._combo.config(values=self._names, state="readonly")
            # Keep current selection if still valid, else pick first entry.
            if self._combo_var.get() not in self._names:
                self._combo_var.set(self._names[0])
        else:
            self._names = []
            self._paths = []
            self._combo.config(values=["— Kein Modell gefunden —"], state="disabled")
            self._combo_var.set("— Kein Modell gefunden —")

    def get_path(self) -> Optional[str]:
        """Return the absolute path of the currently selected model, or None."""
        try:
            idx = self._names.index(self._combo_var.get())
            return self._paths[idx]
        except (ValueError, IndexError):
            return None

    def _browse(self) -> None:
        """Let the user pick an arbitrary .zip file not in the scanned list."""
        p = filedialog.askopenfilename(
            initialdir=ROOT, title="Modell wählen",
            filetypes=[("Modell (zip)", "*.zip"), ("Alle Dateien", "*")],
        )
        if not p:
            return
        abs_p = os.path.abspath(p)
        rel = os.path.relpath(abs_p, ROOT)
        # Add to list so it stays selectable this session.
        if abs_p not in self._paths:
            self._names.append(rel)
            self._paths.append(abs_p)
            self._combo.config(values=self._names, state="readonly")
        self._combo_var.set(self._names[self._paths.index(abs_p)])


# ---------------------------------------------------------------------------
# Main Application
# ---------------------------------------------------------------------------
class App(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("Stoneforge RL Launcher")
        self.geometry("980x720")
        self.minsize(820, 600)
        self.configure(bg=BG)

        self._proc: Optional[subprocess.Popen] = None
        self._out_queue: queue.Queue[tuple[str, object]] = queue.Queue()
        self._running = False
        self._spinner_idx = 0
        self._total_ts = 1_000_000      # set from training form
        self._current_mode = ""         # "train" | "build" | "eval" | ""
        self._eval_tmp_script: Optional[str] = None

        self._setup_styles()
        self._build_ui()
        self._poll()
        self._animate_spinner()

    # ------------------------------------------------------------------
    # Styles
    # ------------------------------------------------------------------
    def _setup_styles(self) -> None:
        s = ttk.Style(self)
        s.theme_use("clam")
        s.configure(".", background=BG, foreground=TEXT,
                    fieldbackground=BG3, bordercolor=BG3,
                    troughcolor=BG3, selectbackground=ACCENT,
                    selectforeground=BTN_FG)

        # Sidebar
        for name, bg_c, fg_c, bold in [
            ("Nav.TButton",       BG2,  DIM,    False),
            ("NavActive.TButton", BG3,  ACCENT, True),
        ]:
            s.configure(name, background=bg_c, foreground=fg_c, anchor="w",
                        font=("Helvetica", 12, "bold" if bold else "normal"),
                        padding=(16, 11), relief="flat", borderwidth=0)
            s.map(name, background=[("active", BG3), ("pressed", BG3)],
                  foreground=[("active", TEXT)])

        # Buttons
        s.configure("Action.TButton",    background=ACCENT,  foreground=BTN_FG,
                    font=("Helvetica", 11, "bold"), padding=(20, 10), relief="flat")
        s.map("Action.TButton",
              background=[("active", "#b4befe"), ("disabled", BG3)],
              foreground=[("disabled", DIM)])

        s.configure("Secondary.TButton", background=BG3, foreground=TEXT,
                    font=("Helvetica", 10), padding=(12, 8), relief="flat")
        s.map("Secondary.TButton",
              background=[("active", BG4), ("disabled", BG2)],
              foreground=[("disabled", DIM)])

        s.configure("Stop.TButton", background=RED, foreground=BTN_FG,
                    font=("Helvetica", 10, "bold"), padding=(12, 8), relief="flat")
        s.map("Stop.TButton", background=[("active", "#eba0ac")])

        s.configure("Ghost.TButton", background=BG2, foreground=DIM,
                    font=("Helvetica", 9), padding=(8, 5), relief="flat")
        s.map("Ghost.TButton", background=[("active", BG3)], foreground=[("active", TEXT)])

        # Labels
        s.configure("H1.TLabel",      background=BG,  foreground=TEXT,
                    font=("Helvetica", 20, "bold"))
        s.configure("Section.TLabel", background=BG,  foreground=ACCENT,
                    font=("Helvetica", 10, "bold"))
        s.configure("Body.TLabel",    background=BG,  foreground=TEXT,
                    font=("Helvetica", 10))
        s.configure("Dim.TLabel",     background=BG,  foreground=DIM,
                    font=("Helvetica", 9))
        s.configure("Card.TLabel",    background=BG3, foreground=TEXT,
                    font=("Helvetica", 10))

        # Frames
        s.configure("TFrame",         background=BG)
        s.configure("Sidebar.TFrame", background=BG2)
        s.configure("Card.TFrame",    background=BG3)
        s.configure("Bottom.TFrame",  background=BG2)

        # Checkbox / Radio
        for w in ("TCheckbutton", "TRadiobutton"):
            s.configure(w, background=BG, foreground=TEXT, font=("Helvetica", 10))
            s.map(w, background=[("active", BG)])

        # Scale / Progressbar
        s.configure("TScale", background=BG, troughcolor=BG3,
                    sliderlength=18, sliderrelief="flat")
        s.configure("Accent.Horizontal.TProgressbar",
                    troughcolor=BG3, background=ACCENT,
                    thickness=8, relief="flat", borderwidth=0)
        s.configure("Green.Horizontal.TProgressbar",
                    troughcolor=BG3, background=GREEN,
                    thickness=8, relief="flat", borderwidth=0)

        # Entry
        s.configure("TEntry", fieldbackground=BG3, foreground=TEXT,
                    insertcolor=TEXT, bordercolor=BG3, relief="flat", padding=6)

        # Separator
        s.configure("TSeparator", background=BG4)

    # ------------------------------------------------------------------
    # Root layout
    # ------------------------------------------------------------------
    def _build_ui(self) -> None:
        self.columnconfigure(1, weight=1)
        self.rowconfigure(0, weight=1)

        # ── Sidebar ──────────────────────────────────────────────────
        sidebar = ttk.Frame(self, style="Sidebar.TFrame", width=190)
        sidebar.grid(row=0, column=0, sticky="nsew")
        sidebar.grid_propagate(False)

        tk.Label(sidebar, text="⚙ Stoneforge", bg=BG2, fg=ACCENT,
                 font=("Helvetica", 15, "bold")).pack(pady=(22, 2), padx=18, anchor="w")
        tk.Label(sidebar, text="RL Launcher", bg=BG2, fg=DIM,
                 font=("Helvetica", 9)).pack(padx=18, anchor="w")

        ttk.Separator(sidebar, orient="horizontal").pack(fill="x", pady=14)

        sections = [
            ("🔧  Build",          "build"),
            ("🧠  Training",       "training"),
            ("▶   Abspielen",     "play"),
            ("📊  Evaluation",    "eval"),
            ("🎮  Spiel starten", "game"),
        ]
        self._nav_btns: dict[str, ttk.Button] = {}
        for label, key in sections:
            btn = ttk.Button(sidebar, text=label, style="Nav.TButton",
                             command=lambda k=key: self._show_section(k))
            btn.pack(fill="x", pady=1)
            self._nav_btns[key] = btn

        # Version tag at bottom
        ttk.Separator(sidebar, orient="horizontal").pack(fill="x", pady=14)
        tk.Label(sidebar, text="v2.1", bg=BG2, fg=DIM,
                 font=("Helvetica", 8)).pack(padx=18, anchor="w")

        # ── Content area ─────────────────────────────────────────────
        right = ttk.Frame(self)
        right.grid(row=0, column=1, sticky="nsew")
        right.columnconfigure(0, weight=1)
        right.rowconfigure(0, weight=1)  # content
        right.rowconfigure(1, weight=0)  # metrics + progress
        right.rowconfigure(2, weight=0)  # log

        # Content pages
        container = ttk.Frame(right)
        container.grid(row=0, column=0, sticky="nsew")
        container.columnconfigure(0, weight=1)
        container.rowconfigure(0, weight=1)

        self._sections: dict[str, ttk.Frame] = {}
        for _, key in sections:
            f = ttk.Frame(container)
            f.grid(row=0, column=0, sticky="nsew")
            self._sections[key] = f

        self._build_section_build(self._sections["build"])
        self._build_section_training(self._sections["training"])
        self._build_section_play(self._sections["play"])
        self._build_section_eval(self._sections["eval"])
        self._build_section_game(self._sections["game"])

        # ── Live Metrics panel (row 1) ────────────────────────────────
        self._metrics_outer = ttk.Frame(right, style="Bottom.TFrame")
        self._metrics_outer.grid(row=1, column=0, sticky="ew")
        self._metrics_outer.grid_remove()          # hidden until training starts
        self._build_metrics_panel(self._metrics_outer)

        # ── Output log (row 2) ───────────────────────────────────────
        log_frame = ttk.Frame(right, style="Bottom.TFrame")
        log_frame.grid(row=2, column=0, sticky="ew")
        log_frame.columnconfigure(0, weight=1)
        self._build_log(log_frame)

        self._show_section("training")

    # ------------------------------------------------------------------
    # Live Metrics Panel
    # ------------------------------------------------------------------
    def _build_metrics_panel(self, parent: ttk.Frame) -> None:
        ttk.Separator(parent, orient="horizontal").pack(fill="x")

        header = tk.Frame(parent, bg=BG2)
        header.pack(fill="x", padx=12, pady=(8, 4))
        tk.Label(header, text="Live Statistiken", bg=BG2, fg=ACCENT,
                 font=("Helvetica", 10, "bold")).pack(side="left")
        self._best_badge = tk.Label(header, text="", bg=BG2, fg=YELLOW,
                                    font=("Helvetica", 9, "bold"))
        self._best_badge.pack(side="left", padx=12)

        # Stat cards row
        cards_row = tk.Frame(parent, bg=BG2)
        cards_row.pack(fill="x", padx=12, pady=(0, 8))

        self._card_ts     = StatCard(cards_row, "Timesteps",  color=ACCENT2)
        self._card_reward = StatCard(cards_row, "∅ Reward",   color=ORANGE)
        self._card_eplen  = StatCard(cards_row, "∅ Ep.Länge", color=TEXT)
        self._card_eval   = StatCard(cards_row, "Eval Reward",color=GREEN)
        self._card_fps    = StatCard(cards_row, "FPS",        color=DIM)
        for c in (self._card_ts, self._card_reward, self._card_eplen,
                  self._card_eval, self._card_fps):
            c.pack(side="left", padx=4)

        # Curriculum stage indicators
        self._curr_frame = tk.Frame(parent, bg=BG2)
        self._curr_frame.pack(fill="x", padx=12, pady=(0, 4))
        tk.Label(self._curr_frame, text="Curriculum:", bg=BG2, fg=DIM,
                 font=("Helvetica", 9)).pack(side="left", padx=(0, 6))
        self._curr_dots: list[tk.Label] = []
        for _ in range(4):
            dot = tk.Label(self._curr_frame, text="●", bg=BG2, fg=BG4,
                           font=("Helvetica", 12))
            dot.pack(side="left", padx=1)
            self._curr_dots.append(dot)
        self._curr_label = tk.Label(self._curr_frame, text="", bg=BG2, fg=DIM,
                                    font=("Helvetica", 9))
        self._curr_label.pack(side="left", padx=(8, 0))

        # Progress bar
        prog_row = tk.Frame(parent, bg=BG2)
        prog_row.pack(fill="x", padx=12, pady=(2, 10))
        prog_row.columnconfigure(1, weight=1)

        self._prog_label = tk.Label(prog_row, text="0%", bg=BG2, fg=DIM,
                                    font=("Helvetica", 9), width=5, anchor="e")
        self._prog_label.grid(row=0, column=0, padx=(0, 8))
        self._progress = ttk.Progressbar(prog_row, style="Accent.Horizontal.TProgressbar",
                                         mode="determinate", maximum=100)
        self._progress.grid(row=0, column=1, sticky="ew")
        self._prog_ts_label = tk.Label(prog_row, text="", bg=BG2, fg=DIM,
                                       font=("Helvetica", 9), anchor="w")
        self._prog_ts_label.grid(row=0, column=2, padx=(8, 0))

        # Indeterminate bar (shown during builds)
        self._progress_indet = ttk.Progressbar(prog_row,
                                                style="Accent.Horizontal.TProgressbar",
                                                mode="indeterminate")
        self._progress_indet.grid(row=0, column=1, sticky="ew")
        self._progress_indet.grid_remove()

    def _show_metrics(self, mode: str) -> None:
        """Show the metrics panel; mode='train' or 'build' or 'eval'."""
        self._current_mode = mode
        self._metrics_outer.grid()
        if mode == "build":
            self._progress.grid_remove()
            self._progress_indet.grid()
            self._progress_indet.start(12)
            self._curr_frame.pack_forget()
            self._prog_label.config(text="")
            self._prog_ts_label.config(text="Build läuft…")
            for card in (self._card_ts, self._card_reward,
                         self._card_eplen, self._card_eval, self._card_fps):
                card.pack_forget()
        elif mode == "train":
            self._progress_indet.stop()
            self._progress_indet.grid_remove()
            self._progress.grid()
            self._progress["value"] = 0
            self._curr_frame.pack(fill="x", padx=12, pady=(0, 4))
            for c in (self._card_ts, self._card_reward, self._card_eplen,
                      self._card_eval, self._card_fps):
                c.pack(side="left", padx=4)
            self._best_badge.config(text="")
            for dot in self._curr_dots:
                dot.config(fg=BG4)
        elif mode == "eval":
            self._progress_indet.stop()
            self._progress_indet.grid_remove()
            self._progress.grid()
            self._progress["value"] = 0
            self._curr_frame.pack_forget()
            self._prog_ts_label.config(text="Seed 0 / 50")
            for card in (self._card_ts, self._card_reward,
                         self._card_eplen, self._card_fps):
                card.pack_forget()
            self._card_eval.pack(side="left", padx=4)

    def _hide_metrics(self) -> None:
        self._progress_indet.stop()
        self._metrics_outer.grid_remove()

    # ------------------------------------------------------------------
    # Log
    # ------------------------------------------------------------------
    def _build_log(self, parent: ttk.Frame) -> None:
        ttk.Separator(parent, orient="horizontal").pack(fill="x")

        header = tk.Frame(parent, bg=BG2)
        header.pack(fill="x", padx=10, pady=(6, 2))

        self._spinner_lbl = tk.Label(header, text=" ", bg=BG2, fg=ACCENT,
                                     font=("Helvetica", 11))
        self._spinner_lbl.pack(side="left")

        tk.Label(header, text="Output", bg=BG2, fg=ACCENT,
                 font=("Helvetica", 10, "bold")).pack(side="left", padx=4)

        self._status_var = tk.StringVar(value="Bereit")
        tk.Label(header, textvariable=self._status_var, bg=BG2, fg=DIM,
                 font=("Helvetica", 9)).pack(side="left", padx=8)

        ttk.Button(header, text="Clear",    style="Ghost.TButton",
                   command=self._clear_log).pack(side="right", padx=2)
        self._stop_btn = ttk.Button(header, text="■ Stop", style="Stop.TButton",
                                    command=self._stop_proc, state="disabled")
        self._stop_btn.pack(side="right", padx=4)

        # Toggle log visibility
        self._log_visible = True
        self._toggle_btn = ttk.Button(header, text="▾ Log",
                                      style="Ghost.TButton",
                                      command=self._toggle_log)
        self._toggle_btn.pack(side="right", padx=2)

        self._log_box = scrolledtext.ScrolledText(
            parent, height=8, bg=BG2, fg=TEXT,
            font=("Menlo", 10), insertbackground=TEXT,
            relief="flat", bd=0, state="disabled",
            selectbackground=BG3, selectforeground=TEXT,
        )
        self._log_box.pack(fill="both", expand=True, padx=10, pady=(0, 6))
        self._log_box.tag_config("cmd",  foreground=ACCENT2)
        self._log_box.tag_config("ok",   foreground=GREEN)
        self._log_box.tag_config("err",  foreground=RED)
        self._log_box.tag_config("warn", foreground=ORANGE)
        self._log_box.tag_config("seed_ok",   foreground=GREEN)
        self._log_box.tag_config("seed_fail", foreground=RED)

    def _toggle_log(self) -> None:
        if self._log_visible:
            self._log_box.pack_forget()
            self._toggle_btn.config(text="▸ Log")
        else:
            self._log_box.pack(fill="both", expand=True, padx=10, pady=(0, 6))
            self._toggle_btn.config(text="▾ Log")
        self._log_visible = not self._log_visible

    # ------------------------------------------------------------------
    # Section builders
    # ------------------------------------------------------------------
    def _build_section_build(self, f: ttk.Frame) -> None:
        self._section_header(f, "Build",
                             "Python-Bindings und Spiel-Client kompilieren.")

        card = self._card(f)
        for label, cmd_fn in [
            ("Python Bindings (stoneforge_sim)",    self._do_build_bindings),
            ("Spiel-Client  (stoneforge_client)",   self._do_build_client),
            ("Alles  (Bindings + Client)",          self._do_build_all),
        ]:
            row = tk.Frame(card, bg=BG3)
            row.pack(fill="x", pady=2)
            tk.Label(row, text=label, bg=BG3, fg=TEXT,
                     font=("Helvetica", 10)).pack(side="left", padx=14, pady=10)
            ttk.Button(row, text="Build", style="Secondary.TButton",
                       command=cmd_fn).pack(side="right", padx=12)

        self._dim(f, "Release-Modus · BUILD_PYTHON_BINDINGS=ON · -j4")

    def _build_section_training(self, f: ttk.Frame) -> None:
        self._section_header(f, "Training",
                             "DQN oder PPO trainieren. Curriculum steigert Schwierigkeit automatisch.")

        card = self._card(f)

        # Algorithm
        self._sec_label(card, "Algorithmus")
        row = tk.Frame(card, bg=BG3)
        row.pack(fill="x", padx=14, pady=(0, 8))
        self._algo_var = tk.StringVar(value="dqn")
        for text, val in [("DQN  (empfohlen)", "dqn"), ("PPO", "ppo")]:
            ttk.Radiobutton(row, text=text, variable=self._algo_var,
                            value=val).pack(side="left", padx=(0, 24))

        self._sep(card)

        # Timesteps
        self._sec_label(card, "Timesteps")
        ts_f = tk.Frame(card, bg=BG3)
        ts_f.pack(fill="x", padx=14, pady=(0, 8))
        ts_f.columnconfigure(0, weight=1)

        self._ts_var = tk.IntVar(value=1_000_000)
        self._ts_disp = tk.Label(ts_f, text="1,000,000", bg=BG3, fg=ACCENT,
                                 font=("Helvetica", 13, "bold"))
        self._ts_disp.grid(row=0, column=1, padx=(12, 0))
        ttk.Scale(ts_f, from_=100_000, to=5_000_000, variable=self._ts_var,
                  orient="horizontal", command=self._on_ts_slide
                  ).grid(row=0, column=0, sticky="ew")
        preset_row = tk.Frame(ts_f, bg=BG3)
        preset_row.grid(row=1, column=0, columnspan=2, pady=(6, 0), sticky="w")
        for lbl, v in [("100K", 100_000), ("500K", 500_000),
                       ("1M", 1_000_000), ("2M", 2_000_000), ("5M", 5_000_000)]:
            ttk.Button(preset_row, text=lbl, style="Secondary.TButton",
                       command=lambda x=v: self._set_ts(x)).pack(side="left", padx=2)

        self._sep(card)

        # Options
        self._sec_label(card, "Optionen")
        self._curriculum_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(card,
                        text="Curriculum Learning  (5–12 → 35–45 Tiles in 4 Stufen)",
                        variable=self._curriculum_var).pack(anchor="w", padx=14, pady=(0, 10))

        self._sep(card)
        ttk.Button(card, text="▶  Training starten", style="Action.TButton",
                   command=self._do_train).pack(padx=14, pady=(4, 14))

    def _build_section_play(self, f: ttk.Frame) -> None:
        self._section_header(f, "Abspielen",
                             "KI-Agent grafisch ausführen. Dual-Modus vergleicht zwei Modelle.")

        card = self._card(f)
        self._sec_label(card, "Modell auswählen")
        self._play_picker1 = ModelPicker(card)
        self._play_picker1.pack(fill="x", padx=14, pady=(0, 8))

        self._sep(card)

        opt = tk.Frame(card, bg=BG3)
        opt.pack(fill="x", padx=14, pady=(0, 8))
        tk.Label(opt, text="Seed:", bg=BG3, fg=TEXT,
                 font=("Helvetica", 10)).grid(row=0, column=0, sticky="w", padx=(0, 8))
        self._play_seed = tk.IntVar(value=42)
        ttk.Entry(opt, textvariable=self._play_seed, width=8).grid(row=0, column=1, sticky="w")
        tk.Label(opt, text="  Geschwindigkeit:", bg=BG3, fg=TEXT,
                 font=("Helvetica", 10)).grid(row=0, column=2, padx=(18, 8))
        self._play_speed = tk.DoubleVar(value=1.0)
        tk.Spinbox(opt, textvariable=self._play_speed, from_=0.1, to=10.0,
                   increment=0.5, width=5, bg=BG3, fg=TEXT, relief="flat",
                   buttonbackground=BG4, insertbackground=TEXT,
                   ).grid(row=0, column=3, sticky="w")

        self._sep(card)
        self._dual_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(card, text="Dual-Modus  (zwei Modelle vergleichen)",
                        variable=self._dual_var,
                        command=self._toggle_dual).pack(anchor="w", padx=14, pady=(4, 0))

        self._dual_inner = tk.Frame(card, bg=BG3)
        tk.Label(self._dual_inner, text="Modell 2:", bg=BG3, fg=DIM,
                 font=("Helvetica", 9)).pack(anchor="w", padx=14, pady=(6, 2))
        self._play_picker2 = ModelPicker(self._dual_inner)
        self._play_picker2.pack(fill="x", padx=14, pady=(0, 8))

        self._sep(card)
        ttk.Button(card, text="▶  Abspielen", style="Action.TButton",
                   command=self._do_play).pack(padx=14, pady=(4, 14))

    def _build_section_eval(self, f: ttk.Frame) -> None:
        self._section_header(f, "Evaluation",
                             "50-Seed Test (7000–7049) — misst echte Generalisierung.")

        card = self._card(f)
        self._sec_label(card, "Modell auswählen")
        self._eval_picker = ModelPicker(card)
        self._eval_picker.pack(fill="x", padx=14, pady=(0, 8))

        self._sep(card)
        ttk.Button(card, text="📊  50-Seed Evaluation starten",
                   style="Action.TButton",
                   command=self._do_eval).pack(padx=14, pady=(4, 14))

        self._eval_result = tk.Label(f, text="", bg=BG, fg=GREEN,
                                     font=("Helvetica", 12, "bold"))
        self._eval_result.pack(anchor="w", padx=30, pady=(12, 0))
        self._dim(f, "Ziel: ≥ 70% auf Testset A  |  ≥ 60% auf Holdout-Set B")

    def _build_section_game(self, f: ttk.Frame) -> None:
        self._section_header(f, "Spiel starten",
                             "Stoneforge-Client direkt ohne KI starten.")

        card = self._card(f)
        opt = tk.Frame(card, bg=BG3)
        opt.pack(fill="x", padx=14, pady=(12, 8))
        tk.Label(opt, text="Seed:", bg=BG3, fg=TEXT,
                 font=("Helvetica", 10)).pack(side="left", padx=(0, 8))
        self._game_seed = tk.IntVar(value=42)
        ttk.Entry(opt, textvariable=self._game_seed, width=10).pack(side="left")

        self._sep(card)
        btn_row = tk.Frame(card, bg=BG3)
        btn_row.pack(padx=14, pady=(4, 14), anchor="w")
        ttk.Button(btn_row, text="🎮  Spiel starten", style="Action.TButton",
                   command=self._do_game).pack(side="left", padx=(0, 8))
        ttk.Button(btn_row, text="Build Client zuerst", style="Secondary.TButton",
                   command=self._do_build_client).pack(side="left")

        self._game_status_lbl = tk.Label(f, text="", bg=BG,
                                         font=("Helvetica", 10))
        self._game_status_lbl.pack(anchor="w", padx=30, pady=(12, 0))
        self._refresh_game_status()

    # ------------------------------------------------------------------
    # UI helpers
    # ------------------------------------------------------------------
    def _section_header(self, f: ttk.Frame, title: str, subtitle: str) -> None:
        f.columnconfigure(0, weight=1)
        tk.Frame(f, bg=BG, height=22).pack()
        tk.Label(f, text=title, bg=BG, fg=TEXT,
                 font=("Helvetica", 20, "bold")).pack(anchor="w", padx=28)
        tk.Label(f, text=subtitle, bg=BG, fg=DIM,
                 font=("Helvetica", 10), wraplength=560).pack(anchor="w", padx=28, pady=(2, 14))

    def _card(self, parent: ttk.Frame) -> tk.Frame:
        c = tk.Frame(parent, bg=BG3)
        c.pack(fill="x", padx=24, pady=4)
        return c

    def _sep(self, parent: tk.Frame) -> None:
        tk.Frame(parent, bg=BG4, height=1).pack(fill="x", padx=8, pady=4)

    def _sec_label(self, parent: tk.Frame, text: str) -> None:
        tk.Label(parent, text=text, bg=BG3, fg=ACCENT,
                 font=("Helvetica", 9, "bold")).pack(anchor="w", padx=14, pady=(10, 4))

    def _dim(self, parent: ttk.Frame, text: str) -> None:
        tk.Label(parent, text=text, bg=BG, fg=DIM,
                 font=("Helvetica", 9), justify="left").pack(anchor="w", padx=30, pady=(10, 0))

    def _toggle_dual(self) -> None:
        if self._dual_var.get():
            self._dual_inner.pack(fill="x", pady=(0, 4))
        else:
            self._dual_inner.pack_forget()

    def _refresh_model_pickers(self) -> None:
        """Refresh all model dropdowns from the current filesystem state."""
        for picker in (getattr(self, "_play_picker1", None),
                       getattr(self, "_play_picker2", None),
                       getattr(self, "_eval_picker", None)):
            if picker is not None:
                picker.refresh()

    def _refresh_game_status(self) -> None:
        if os.path.exists(GAME_BINARY):
            self._game_status_lbl.config(
                text=f"✓ Binary gefunden: {GAME_BINARY}", fg=GREEN)
        else:
            self._game_status_lbl.config(
                text="✗ Binary fehlt — 'Build Client' ausführen.", fg=RED)

    # ------------------------------------------------------------------
    # Navigation
    # ------------------------------------------------------------------
    def _show_section(self, key: str) -> None:
        for k, btn in self._nav_btns.items():
            btn.configure(style="NavActive.TButton" if k == key else "Nav.TButton")
        for k, f in self._sections.items():
            (f.tkraise if k == key else lambda: None)()
        self._sections[key].tkraise()

        if key in ("play", "eval"):
            self._refresh_model_pickers()

    # ------------------------------------------------------------------
    # Timestep slider
    # ------------------------------------------------------------------
    def _on_ts_slide(self, _: str) -> None:
        v = max(100_000, round(self._ts_var.get() / 50_000) * 50_000)
        self._ts_var.set(v)
        self._ts_disp.config(text=f"{v:,}")

    def _set_ts(self, v: int) -> None:
        self._ts_var.set(v)
        self._ts_disp.config(text=f"{v:,}")

    # ------------------------------------------------------------------
    # Command runners
    # ------------------------------------------------------------------
    def _run(self, cmd: str, mode: str = "") -> None:
        if self._running:
            self._log("⚠  Prozess läuft bereits.\n", "warn")
            return
        self._set_running(True, mode)
        self._log(f"$ {cmd}\n", "cmd")

        def worker() -> None:
            try:
                proc = subprocess.Popen(
                    cmd,
                    shell=True,
                    cwd=ROOT,
                    env=_make_env(),
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1,
                    encoding='utf-8',
                    errors='replace',  # Handle Windows compiler encoding issues
                )
                self._proc = proc
                assert proc.stdout
                for line in proc.stdout:
                    self._out_queue.put(("line", line))
                rc = proc.wait()
                self._out_queue.put(("done", rc))
            except Exception as exc:
                self._out_queue.put(("err", str(exc)))

        threading.Thread(target=worker, daemon=True).start()

    def _do_build_bindings(self) -> None:
        # Build Python bindings
        build_cmd = (
            f"cmake -S {_quote(ROOT)} -B {_quote(BUILD_DIR)}"
            " -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON"
            f" && cmake --build {_quote(BUILD_DIR)} --target stoneforge_sim -j4"
        )
        self._run(build_cmd, mode="build")
        # After build, copy module to python/ folder
        self.after(1000, self._copy_sim_module)

    def _copy_sim_module(self) -> None:
        """Copy the compiled stoneforge_sim module (.pyd on Windows, .so on Unix) to python/ folder."""
        built = _find_so()
        if built:
            dest_dir = os.path.join(ROOT, "python")
            dest = os.path.join(dest_dir, os.path.basename(built))
            try:
                if os.path.abspath(built) != os.path.abspath(dest):
                    shutil.copy2(built, dest)
                    ext = ".pyd" if _IS_WIN else ".so"
                    self._log(f"✓ {os.path.basename(built)} → python/{ext}\n", "ok")
            except Exception as e:
                self._log(f"⚠ Kopieren fehlgeschlagen: {e}\n", "warn")

    def _do_build_client(self) -> None:
        self._run(
            f"cmake -S {_quote(ROOT)} -B {_quote(BUILD_DIR)}"
            " -DCMAKE_BUILD_TYPE=Release"
            f" && cmake --build {_quote(BUILD_DIR)} --target stoneforge_client -j4",
            mode="build",
        )
        self.after(2500, self._refresh_game_status)

    def _do_build_all(self) -> None:
        # Build everything (Python bindings + client)
        build_cmd = (
            f"cmake -S {_quote(ROOT)} -B {_quote(BUILD_DIR)}"
            " -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON"
            f" && cmake --build {_quote(BUILD_DIR)} -j4"
        )
        self._run(build_cmd, mode="build")
        # After build, copy module and refresh game status
        self.after(3000, self._copy_sim_module)
        self.after(3000, self._refresh_game_status)

    def _do_train(self) -> None:
        if not _ensure_requirements():
            self._log("✗ Anforderungen konnten nicht installiert werden.\n", "err")
            return
        algo = self._algo_var.get()
        ts = self._ts_var.get()
        self._total_ts = ts
        cur = self._curriculum_var.get()
        parts = [_quote(PY), "python/train.py",
                 "--algo", algo, "--timesteps", str(ts)]
        if not cur:
            parts.append("--no-curriculum")
        self._run(" ".join(parts), mode="train")

    def _do_play(self) -> None:
        if not _ensure_requirements():
            self._log("✗ Anforderungen konnten nicht installiert werden.\n", "err")
            return
        model = self._play_picker1.get_path()
        if not model:
            self._log("✗ Kein Modell ausgewählt.\n", "err")
            return
        parts = [_quote(PY), "python/ai_play.py",
                 "--model", _quote(model),
                 "--seed", str(self._play_seed.get()),
                 "--speed", str(self._play_speed.get())]
        if self._dual_var.get():
            m2 = self._play_picker2.get_path()
            if m2:
                parts += ["--model2", _quote(m2)]
        self._run(" ".join(parts))

    def _do_eval(self) -> None:
        if not _ensure_requirements():
            self._log("✗ Anforderungen konnten nicht installiert werden.\n", "err")
            return
        model = self._eval_picker.get_path()
        if not model:
            self._log("✗ Kein Modell ausgewählt.\n", "err")
            return
        script = (
            "import numpy as np\n"
            "from stable_baselines3 import PPO, DQN\n"
            "from stoneforge_env import ExitPotentialFieldWrapper, StoneforgeWorldEnv\n"
            "seeds=list(range(7000,7050))\n"
            f"path={repr(model)}\n"
            "make_env=lambda: ExitPotentialFieldWrapper(StoneforgeWorldEnv())\n"
            "try:\n"
            "    model=PPO.load(path); name='PPO'\n"
            "except Exception:\n"
            "    model=DQN.load(path); name='DQN'\n"
            "env=make_env(); succ,lens,rets=0,[],[]\n"
            "for i,seed in enumerate(seeds):\n"
            "    obs,_=env.reset(seed=seed); done=False; ep=0.0; n=0; ok=False\n"
            "    while not done and n<4000:\n"
            "        a,_=model.predict(obs,deterministic=True)\n"
            "        obs,r,t,tr,info=env.step(int(a)); ep+=r; n+=1\n"
            "        if info.get('reached_exit'): ok=True\n"
            "        done=t or tr\n"
            "    succ+=int(ok); lens.append(n); rets.append(ep)\n"
            "    status='OK' if ok else 'FAIL'\n"
            "    print(f'SEED_RESULT {i+1} {status} {n} {ep:.2f}', flush=True)\n"
            "print(f'EVAL_FINAL {name} {succ} {np.mean(lens):.0f} {np.mean(rets):.2f}', flush=True)\n"
        )
        self._eval_seeds_done = 0
        with tempfile.NamedTemporaryFile("w", suffix="_stoneforge_eval.py", delete=False,
                                         encoding="utf-8", newline="\n") as tmp:
            tmp.write(script)
            tmp_path = tmp.name

        self._eval_tmp_script = tmp_path
        self._run(f"{_quote(PY)} {_quote(tmp_path)}", mode="eval")

    def _do_game(self) -> None:
        if not os.path.exists(GAME_BINARY):
            self._log("✗ Binary fehlt — Build Client ausführen.\n", "err")
            return
        seed = self._game_seed.get()
        cmd = f"{_quote(GAME_BINARY)} --seed {seed}"
        self._log(f"$ {cmd}\n", "cmd")
        subprocess.Popen(
            cmd,
            shell=True,
            cwd=ROOT,
            encoding='utf-8',
            errors='replace',
        )

    def _stop_proc(self) -> None:
        if self._proc and self._proc.poll() is None:
            self._proc.terminate()
            self._log("■ Prozess gestoppt.\n", "warn")
        self._set_running(False)

    # ------------------------------------------------------------------
    # Output parsing & metric update
    # ------------------------------------------------------------------
    def _parse_and_update(self, line: str) -> None:
        """Extract metrics from SB3 / custom output and update cards."""
        if self._current_mode == "train":
            # SB3 table rows
            for m in _RE_SB3_KV.finditer(line):
                key, val = m.group(1), m.group(2).strip()
                if "total_timesteps" in key:
                    try:
                        ts = int(float(val))
                        self._card_ts.set(f"{ts:,}")
                        pct = min(100, ts * 100 // self._total_ts)
                        self._progress["value"] = pct
                        self._prog_label.config(text=f"{pct}%")
                        self._prog_ts_label.config(
                            text=f"{ts:,} / {self._total_ts:,}")
                    except ValueError:
                        pass
                elif "ep_rew_mean" in key:
                    self._card_reward.set(val)
                elif "ep_len_mean" in key:
                    self._card_eplen.set(val)
                elif "/fps" in key:
                    self._card_fps.set(val)

            # Eval callback line
            m = _RE_EVAL.search(line)
            if m:
                self._card_eval.set(m.group(1))

            # Curriculum stage
            m = _RE_CURR.search(line)
            if m:
                stage = int(m.group(1))
                total = int(m.group(2))
                d_min, d_max = m.group(3), m.group(4)
                for i, dot in enumerate(self._curr_dots):
                    dot.config(fg=ACCENT if i < stage else BG4)
                self._curr_label.config(
                    text=f"Stufe {stage}/{total} — {d_min}–{d_max} Tiles")

            # New best model
            if _RE_BEST.search(line):
                self._best_badge.config(text="⭐ Neues bestes Modell!")
                self.after(4000, lambda: self._best_badge.config(text=""))

        elif self._current_mode == "eval":
            # Per-seed result line: SEED_RESULT <i> OK|FAIL <steps> <reward>
            if line.startswith("SEED_RESULT"):
                parts = line.split()
                if len(parts) >= 5:
                    idx = int(parts[1])
                    status = parts[2]
                    pct = min(100, idx * 2)   # 50 seeds → 2% each
                    self._progress["value"] = pct
                    self._prog_label.config(text=f"{pct}%")
                    self._prog_ts_label.config(text=f"Seed {idx} / 50")
                    tag = "seed_ok" if status == "OK" else "seed_fail"
                    self._log(f"  Seed {6999+idx:5d}  [{status:4s}]  {parts[3]} steps\n", tag)
                    return   # skip default log output

            # Final result: EVAL_FINAL <name> <succ> <mean_len> <mean_ret>
            m = _RE_50SEED.search(line)
            if m or line.startswith("EVAL_FINAL"):
                if line.startswith("EVAL_FINAL"):
                    parts = line.split()
                    if len(parts) >= 5:
                        name, succ = parts[1], int(parts[2])
                        pct_str = f"{succ/50:.1%}"
                        result = f"{name}  {succ}/50  ({pct_str})"
                        self._eval_result.config(text=f"✓ {result}")
                        self._card_eval.set(pct_str)
                        self._progress["value"] = 100
                self._log(line, "ok")
                return

    # ------------------------------------------------------------------
    # State / animation
    # ------------------------------------------------------------------
    def _set_running(self, running: bool, mode: str = "") -> None:
        self._running = running
        self._stop_btn.config(state="normal" if running else "disabled")
        self._status_var.set("Läuft…" if running else "Bereit")
        if running:
            self._show_metrics(mode)
        else:
            self._hide_metrics()
            self._current_mode = ""

    def _animate_spinner(self) -> None:
        if self._running:
            self._spinner_lbl.config(
                text=SPINNER_FRAMES[self._spinner_idx % len(SPINNER_FRAMES)],
                fg=ACCENT)
            self._spinner_idx += 1
        else:
            self._spinner_lbl.config(text="●", fg=BG3)
        self.after(80, self._animate_spinner)

    # ------------------------------------------------------------------
    # Log helpers
    # ------------------------------------------------------------------
    def _log(self, text: str, tag: str = "") -> None:
        self._log_box.config(state="normal")
        if not tag:
            low = text.lower()
            if any(k in low for k in ("error", "traceback", "failed", "fehler")):
                tag = "err"
            elif any(k in low for k in ("warning", "warn")):
                tag = "warn"
            elif any(k in low for k in ("✓", "success", "best mean", "===")):
                tag = "ok"
        self._log_box.insert("end", text, tag)
        self._log_box.see("end")
        self._log_box.config(state="disabled")

    def _clear_log(self) -> None:
        self._log_box.config(state="normal")
        self._log_box.delete("1.0", "end")
        self._log_box.config(state="disabled")

    # ------------------------------------------------------------------
    # Poll queue (runs on main thread via after())
    # ------------------------------------------------------------------
    def _poll(self) -> None:
        try:
            while True:
                kind, payload = self._out_queue.get_nowait()
                if kind == "line":
                    line: str = payload  # type: ignore[assignment]
                    self._parse_and_update(line)
                    # Skip internal marker lines from eval
                    if not (self._current_mode == "eval"
                            and line.startswith(("SEED_RESULT", "EVAL_FINAL"))):
                        self._log(line)
                elif kind == "done":
                    rc: int = payload  # type: ignore[assignment]
                    msg = "\n✓ Fertig  (exit 0)\n" if rc == 0 else f"\n✗ Fehler (exit {rc})\n"
                    self._log(msg, "ok" if rc == 0 else "err")
                    if self._current_mode == "eval" and self._eval_tmp_script:
                        try:
                            os.unlink(self._eval_tmp_script)
                        except OSError:
                            pass
                        self._eval_tmp_script = None
                    self._set_running(False)
                elif kind == "err":
                    self._log(f"\n✗ {payload}\n", "err")
                    if self._current_mode == "eval" and self._eval_tmp_script:
                        try:
                            os.unlink(self._eval_tmp_script)
                        except OSError:
                            pass
                        self._eval_tmp_script = None
                    self._set_running(False)
        except queue.Empty:
            pass
        self.after(40, self._poll)


# ---------------------------------------------------------------------------

def main() -> None:
    app = App()
    app.mainloop()


if __name__ == "__main__":
    main()
