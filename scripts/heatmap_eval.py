#!/usr/bin/env python3
"""Stoneforge RL — Heatmap-Evaluation

Führt den Standard-Eval (Seeds 7000–7049) durch und generiert eine Heatmap
der besuchten Koordinaten — aufgeteilt in alle / erfolgreiche / gescheiterte
Episoden. Inspiriert von PufferAI/PokéRL.

Blau (wenige Besuche) → Rot (viele Besuche).

Verwendung:
    python scripts/heatmap_eval.py \\
        --model models/ppo_lstm_curriculum_v2/best_model.zip
    python scripts/heatmap_eval.py \\
        --model models/ppo_lstm_curriculum/best_model.zip --stochastic \\
        --seeds 7000 7050
    python scripts/heatmap_eval.py \\
        --model models/ppo_phase4/best_model.zip --holdout
"""
from __future__ import annotations

import argparse
import os
import sys

import numpy as np

# PYTHONPATH muss build/ und python/ enthalten
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "python"))

import matplotlib
matplotlib.use("Agg")   # kein Display nötig — speichert direkt als PNG
import matplotlib.pyplot as plt

from stable_baselines3 import A2C, DQN, PPO
from sb3_contrib import RecurrentPPO

from stoneforge_env import StoneforgeWorldEnv

MAX_STEPS = 4000


# ─── CLI ──────────────────────────────────────────────────────────────────────

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Heatmap-Evaluation")
    p.add_argument("--model", required=True, help="Pfad zum Modell (.zip)")
    p.add_argument("--stochastic", action="store_true",
                   help="Stochastische Policy (deterministic=False)")
    p.add_argument("--holdout", action="store_true",
                   help="Holdout-Set B (Seeds 8000–8049) statt Testset A")
    p.add_argument("--seeds", nargs=2, type=int, metavar=("START", "END"),
                   help="Eigener Seed-Bereich [START, END)")
    p.add_argument("--exit-min", type=int, default=35)
    p.add_argument("--exit-max", type=int, default=45)
    p.add_argument("--out-dir", default="logs", help="Ausgabeverzeichnis für PNG")
    p.add_argument("--show", action="store_true",
                   help="Heatmap nach dem Speichern im System-Viewer öffnen")
    return p.parse_args()


# ─── Eval ─────────────────────────────────────────────────────────────────────

def load_model(path: str):
    for Cls, name, recurrent in [
        (RecurrentPPO, "RecurrentPPO", True),
        (PPO,          "PPO",          False),
        (A2C,          "A2C",          False),
        (DQN,          "DQN",          False),
    ]:
        try:
            m = Cls.load(path)
            return m, name, recurrent
        except Exception:
            pass
    raise RuntimeError(f"Modell konnte nicht geladen werden: {path}")


def run_eval(model, is_recurrent: bool, seeds: list[int],
             exit_min: int, exit_max: int, deterministic: bool):
    env = StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max)

    all_pos:     list[tuple[int, int]] = []
    success_pos: list[tuple[int, int]] = []
    fail_pos:    list[tuple[int, int]] = []
    successes = 0

    for i, seed in enumerate(seeds):
        obs, _ = env.reset(seed=seed)
        done      = False
        steps     = 0
        reached   = False
        ep_pos:   list[tuple[int, int]] = []
        lstm_states = None
        ep_start    = np.ones((1,), dtype=bool)

        while not done and steps < MAX_STEPS:
            pos = env.core.player_pos()
            ep_pos.append(pos)

            if is_recurrent:
                action_arr, lstm_states = model.predict(
                    obs.reshape(1, -1), state=lstm_states,
                    episode_start=ep_start, deterministic=deterministic,
                )
                action = int(action_arr[0])
                ep_start = np.zeros((1,), dtype=bool)
            else:
                action, _ = model.predict(obs, deterministic=deterministic)
                action = int(action)

            obs, _, term, trunc, info = env.step(action)
            steps += 1
            if info.get("reached_exit"):
                reached = True
            done = term or trunc

        all_pos.extend(ep_pos)
        if reached:
            successes += 1
            success_pos.extend(ep_pos)
        else:
            fail_pos.extend(ep_pos)

        icon = "✓" if reached else "✗"
        print(f"  [{i+1:3d}/{len(seeds)}] Seed {seed}: {icon}  ({steps:4d} Steps)",
              flush=True)

    return all_pos, success_pos, fail_pos, successes


# ─── Heatmap-Plot ─────────────────────────────────────────────────────────────

def _hist(positions: list[tuple[int, int]], x_range, y_range, bins_x, bins_y):
    if not positions:
        return np.zeros((bins_x, bins_y))
    xs = [p[0] for p in positions]
    ys = [p[1] for p in positions]
    h, _, _ = np.histogram2d(xs, ys,
                              bins=[bins_x, bins_y],
                              range=[x_range, y_range])
    return h


def plot_heatmap(all_pos, success_pos, fail_pos, model_path, algo,
                 sr, n_seeds, deterministic, out_dir, show):
    if not all_pos:
        print("Keine Positionsdaten — kein Plot möglich.")
        return

    xs = [p[0] for p in all_pos]
    ys = [p[1] for p in all_pos]
    margin = 3
    x_range = (min(xs) - margin, max(xs) + margin)
    y_range = (min(ys) - margin, max(ys) + margin)
    bins_x  = x_range[1] - x_range[0] + 1
    bins_y  = y_range[1] - y_range[0] + 1

    ext = [x_range[0], x_range[1], y_range[0], y_range[1]]

    fig, axes = plt.subplots(1, 3, figsize=(20, 7))
    fig.patch.set_facecolor('#0d1117')

    mode  = "deterministisch" if deterministic else "stochastisch"
    mname = os.path.basename(os.path.dirname(model_path))
    fig.suptitle(
        f"Stoneforge Heatmap — {mname}  [{algo}]  |  SR: {sr:.1%} ({mode},  {n_seeds} Seeds)",
        color='#e6edf3', fontsize=13, fontweight='bold', y=0.98,
    )

    def _draw(ax, positions, title, cmap):
        h = _hist(positions, x_range, y_range, bins_x, bins_y)
        ax.set_facecolor('#0d1117')
        if positions:
            im = ax.imshow(
                np.log1p(h.T), origin='lower', cmap=cmap,
                extent=ext, aspect='equal',
            )
            plt.colorbar(im, ax=ax, label='log(Besuche + 1)',
                         fraction=0.046, pad=0.04).ax.yaxis.set_tick_params(color='#8b949e')
        else:
            ax.text(0.5, 0.5, 'keine Daten', ha='center', va='center',
                    transform=ax.transAxes, color='#484f58', fontsize=12)
        ax.set_title(f"{title}  ({len(positions):,} Steps)",
                     color='#c9d1d9', fontsize=11, pad=8)
        ax.set_xlabel("X (Tiles)", color='#8b949e')
        ax.set_ylabel("Y (Tiles)", color='#8b949e')
        ax.tick_params(colors='#8b949e')
        for spine in ax.spines.values():
            spine.set_edgecolor('#30363d')
        # Startpunkt markieren
        if x_range[0] <= 0 <= x_range[1] and y_range[0] <= 0 <= y_range[1]:
            ax.scatter([0], [0], c='#58a6ff', s=80, marker='*',
                       zorder=5, label='Start (0,0)')
            ax.legend(fontsize=8, facecolor='#161b22', edgecolor='#30363d',
                      labelcolor='#c9d1d9')

    _draw(axes[0], all_pos,     "Alle Episodes", 'hot')
    _draw(axes[1], success_pos, "✓ Erfolgreiche Episodes", 'YlGn')
    _draw(axes[2], fail_pos,    "✗ Gescheiterte Episodes", 'OrRd')

    plt.tight_layout(rect=[0, 0, 1, 0.96])

    os.makedirs(out_dir, exist_ok=True)
    fname = f"heatmap_{mname}_{mode}.png"
    fpath = os.path.join(out_dir, fname)
    plt.savefig(fpath, dpi=150, bbox_inches='tight', facecolor=fig.get_facecolor())
    print(f"\n  Heatmap gespeichert: {fpath}")

    if show:
        import subprocess, platform
        opener = "open" if platform.system() == "Darwin" else "xdg-open"
        subprocess.run([opener, fpath], check=False)


# ─── Main ─────────────────────────────────────────────────────────────────────

def main() -> None:
    args = parse_args()

    if args.seeds:
        seeds = list(range(args.seeds[0], args.seeds[1]))
    elif args.holdout:
        seeds = list(range(8000, 8050))
        print("Holdout-Set B (Seeds 8000–8049)")
    else:
        seeds = list(range(7000, 7050))
        print("Testset A (Seeds 7000–7049)")

    print(f"Lade Modell: {args.model}")
    model, algo, is_recurrent = load_model(args.model)
    deterministic = not args.stochastic

    print(f"Algo: {algo}  |  Modus: {'stochastisch' if args.stochastic else 'deterministisch'}")
    print(f"Seeds: {seeds[0]}–{seeds[-1]}  |  Exit: {args.exit_min}–{args.exit_max} Tiles\n")

    all_pos, success_pos, fail_pos, successes = run_eval(
        model, is_recurrent, seeds, args.exit_min, args.exit_max, deterministic
    )

    sr = successes / len(seeds)
    print(f"\nErgebnis: {successes}/{len(seeds)} ({sr:.1%})")
    print(f"Gesamte Steps: {len(all_pos):,}  |  "
          f"Erfolg: {len(success_pos):,}  |  Fehler: {len(fail_pos):,}")

    plot_heatmap(all_pos, success_pos, fail_pos,
                 args.model, algo, sr, len(seeds),
                 deterministic, args.out_dir, args.show)


if __name__ == "__main__":
    main()
