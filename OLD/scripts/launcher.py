#!/usr/bin/env python3
"""Terminal launcher for Stoneforge RL training and playback."""
from __future__ import annotations

import glob
import os
import platform
import shlex
import shutil
import subprocess
import sys
from typing import Optional

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Windows uses Scripts/python.exe, Unix uses bin/python3
_IS_WIN = platform.system() == "Windows"
if _IS_WIN:
    VENV_PY = os.path.join(ROOT, ".venv", "Scripts", "python.exe")
else:
    VENV_PY = os.path.join(ROOT, ".venv", "bin", "python3")

PY = VENV_PY if os.path.exists(VENV_PY) else shutil.which("python3") or "python"
BUILD_DIR = os.path.join(ROOT, "build")
REQ_FILE = os.path.join(ROOT, "python", "requirements.txt")
VENV_REQ_MARKER = os.path.join(ROOT, ".venv", ".requirements_installed")

DEFAULT_MODEL_DQN = "best_models_dqn/best_model.zip"
DEFAULT_MODEL_PPO = "best_models_ppo/best_model.zip"


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
    """Build environment dict with correct PYTHONPATH so stoneforge_sim is importable."""
    env = os.environ.copy()
    extra = os.pathsep.join([BUILD_DIR, os.path.join(ROOT, "python")])
    current = env.get("PYTHONPATH", "")
    env["PYTHONPATH"] = f"{extra}{os.pathsep}{current}" if current else extra
    return env


def run(cmd: str, cwd: Optional[str] = None, check: bool = True) -> int:
    """Run a shell command, streaming output live. Returns exit code."""
    print(f"$ {cmd}")
    # Use shell=True for cross-platform compatibility
    # On Windows, shell=True is necessary for batch commands and environment variable expansion
    # Use encoding='utf-8' with errors='replace' to handle compiler output on Windows
    proc = subprocess.run(
        cmd,
        shell=True,
        cwd=cwd or ROOT,
        env=_make_env(),
        encoding='utf-8',
        errors='replace',  # Replace undecodable chars instead of crashing
    )
    if check and proc.returncode != 0:
        print(f"[launcher] Command failed (exit {proc.returncode})", file=sys.stderr)
    return proc.returncode


def _find_so() -> Optional[str]:
    """Find the built stoneforge_sim module (.pyd on Windows, .so on Unix)."""
    if _IS_WIN:
        patterns = [
            os.path.join(BUILD_DIR, "Release", "stoneforge_sim*.pyd"),
            os.path.join(BUILD_DIR, "stoneforge_sim*.pyd"),
            os.path.join(ROOT, "python", "stoneforge_sim*.pyd"),
        ]
    else:
        patterns = [
            os.path.join(BUILD_DIR, "stoneforge_sim*.so"),
            os.path.join(ROOT, "python", "stoneforge_sim*.so"),
        ]
    for pattern in patterns:
        matches = glob.glob(pattern)
        if matches:
            return matches[0]
    return None


def build_bindings(force: bool = False) -> bool:
    """Build Python bindings if not already present. Returns True on success."""
    # Install requirements into venv once.
    if os.path.exists(REQ_FILE) and os.path.exists(os.path.join(ROOT, ".venv")):
        if not os.path.exists(VENV_REQ_MARKER):
            print("[launcher] Installing Python requirements into venv...")
            if run(f"{_quote(PY)} -m pip install -r {_quote(REQ_FILE)}") == 0:
                with open(VENV_REQ_MARKER, "w"):
                    pass
        else:
            print("[launcher] Requirements already installed.")

    if not force and _find_so() is not None:
        print(f"[launcher] Python bindings present: {_find_so()}")
        return True

    print("[launcher] Building Python bindings (Release)...")
    # Configure with Release build — important for training speed.
    rc = run(
        f"cmake -S {_quote(ROOT)} -B {_quote(BUILD_DIR)}"
        " -DCMAKE_BUILD_TYPE=Release"
        " -DBUILD_PYTHON_BINDINGS=ON"
    )
    if rc != 0:
        return False

    rc = run(f"cmake --build {_quote(BUILD_DIR)} --target stoneforge_sim -j 4")
    if rc != 0:
        return False

    built = _find_so()
    if built is None:
        ext = ".pyd" if _IS_WIN else ".so"
        print(f"[launcher] Build done but {ext} not found.", file=sys.stderr)
        return False

    # Copy into python/ so imports work without PYTHONPATH pointing at build/.
    dest_dir = os.path.join(ROOT, "python")
    dest = os.path.join(dest_dir, os.path.basename(built))
    if os.path.abspath(built) != os.path.abspath(dest):
        shutil.copy2(built, dest)
        print(f"[launcher] Copied {os.path.basename(built)} to python/")
    return True


def train(algo: str = "dqn", timesteps: int = 1_000_000) -> int:
    if not build_bindings():
        return 1
    parts = [
        _quote(PY),
        "python/train.py",
        "--algo", _quote(algo),
        "--timesteps", str(int(timesteps)),
        "--exit-min", "5",
        "--exit-max", "45",
    ]
    return run(" ".join(parts), cwd=ROOT)


def play(model: str, seed: int = 42, speed: float = 1.0) -> int:
    if not build_bindings():
        return 1
    parts = [
        _quote(PY),
        "python/watch_agent.py",
        "--model", _quote(model),
        "--seed", str(int(seed)),
        "--speed", str(float(speed)),
    ]
    return run(" ".join(parts), cwd=ROOT)


def eval_50_seeds(model_path: str) -> int:
    """Run the standard 50-seed evaluation (seeds 7000–7049) and print results."""
    if not build_bindings():
        return 1

    script = f"""
import numpy as np
from stable_baselines3 import PPO, DQN
from stoneforge_env import StoneforgeWorldEnv

seeds = list(range(7000, 7050))
path = {repr(model_path)}

def evaluate(path):
    try:
        model = PPO.load(path)
        name = "PPO"
    except Exception:
        model = DQN.load(path)
        name = "DQN"
    env = StoneforgeWorldEnv(exit_min=35, exit_max=45)
    succ, lens, rets = 0, [], []
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        done = False
        ep_ret = 0.0
        steps = 0
        reached = False
        while not done and steps < 4000:
            action, _ = model.predict(obs, deterministic=False)
            obs, r, term, trunc, info = env.step(int(action))
            ep_ret += float(r)
            steps += 1
            if info.get("reached_exit", False):
                reached = True
            done = term or trunc
        succ += int(reached)
        lens.append(steps)
        rets.append(ep_ret)
    print(f"{{name}} | success={{succ}}/50 ({{succ/50:.1%}}) | mean_len={{np.mean(lens):.1f}} | mean_return={{np.mean(rets):.2f}}")

evaluate(path)
"""
    cmd = f"{_quote(PY)} -c {_quote(script)}"
    return run(cmd, cwd=ROOT)


# ---------------------------------------------------------------------------
# Prompts
# ---------------------------------------------------------------------------

def prompt_int(prompt: str, default: int) -> int:
    try:
        val = input(f"{prompt} [{default:,}]: ").strip()
        return int(val) if val else default
    except ValueError:
        print(f"Ungültige Eingabe, verwende {default:,}")
        return default


def prompt_float(prompt: str, default: float) -> float:
    try:
        val = input(f"{prompt} [{default}]: ").strip()
        return float(val) if val else default
    except ValueError:
        print(f"Ungültige Eingabe, verwende {default}")
        return default


def prompt_str(prompt: str, default: str) -> str:
    val = input(f"{prompt} [{default}]: ").strip()
    return val if val else default


def prompt_bool(prompt: str, default: bool) -> bool:
    hint = "J/n" if default else "j/N"
    val = input(f"{prompt} [{hint}]: ").strip().lower()
    if not val:
        return default
    return val in ("j", "ja", "y", "yes", "1")


# ---------------------------------------------------------------------------
# Menu
# ---------------------------------------------------------------------------

def _default_model() -> str:
    for path in (DEFAULT_MODEL_DQN, DEFAULT_MODEL_PPO):
        if os.path.exists(os.path.join(ROOT, path)):
            return path
    return DEFAULT_MODEL_DQN


def menu() -> None:
    print("=" * 45)
    print("  Stoneforge RL Launcher")
    print("=" * 45)
    while True:
        print("\nOptionen:")
        print("  1) DQN trainieren  (empfohlen)")
        print("  2) PPO trainieren")
        print("  3) Modell abspielen")
        print("  4) Zwei Modelle vergleichen (Dual)")
        print("  5) 50-Seed Evaluation")
        print("  6) Python Bindings bauen")
        print("  7) Beenden")
        choice = input("\nOption wählen: ").strip()

        if choice == "1":
            ts = prompt_int("Timesteps", 1_000_000)
            train("dqn", ts)

        elif choice == "2":
            ts = prompt_int("Timesteps", 1_000_000)
            train("ppo", ts)

        elif choice == "3":
            m = prompt_str("Modellpfad", _default_model())
            seed = prompt_int("Seed", 42)
            speed = prompt_float("Geschwindigkeit", 1.0)
            play(m, seed=seed, speed=speed)

        elif choice == "4":
            m = prompt_str("Modellpfad", _default_model())
            seed = prompt_int("Seed", 42)
            speed = prompt_float("Geschwindigkeit", 1.0)
            play(m, seed=seed, speed=speed)

        elif choice == "5":
            m = prompt_str("Modellpfad", _default_model())
            eval_50_seeds(m)

        elif choice == "6":
            build_bindings(force=True)

        elif choice == "7":
            print("Tschüss!")
            break

        else:
            print("Ungültige Option.")


def main() -> None:
    if len(sys.argv) > 1 and sys.argv[1] in ("-h", "--help"):
        print("Usage: python scripts/launcher.py  # interaktives Menü")
        print()
        print("Kein Argument nötig — startet das Menü direkt.")
        return
    try:
        menu()
    except KeyboardInterrupt:
        print("\nAbgebrochen.")


if __name__ == "__main__":
    main()
