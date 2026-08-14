"""Rendert den tatsaechlich gelaufenen Pfad des LSTM-PPO-Agenten neben dem
BFS-optimalen kuerzesten Weg auf einer echten Testset-A-Weltinstanz.

Macht den in Kapitel 5.2.2 berichteten Pfadeffizienz-Wert visuell fassbar,
statt ihn nur als Zahl (0,049) stehen zu lassen.

Passierbarkeit (TileType Empty/Exit) repliziert exakt object.cpp: nur diese
zwei Typen liefern isPassable() == true, alle anderen (Wall, Resource, Tree,
WoodWall, WoodLog, Structure*) sind default-impassable (siehe object.hpp).

Aufruf:
    python scripts/plot_agent_path_vs_bfs.py [--seed 7000] [--model models/ppo_lstm_curriculum_v12_s1/best_model.zip]
"""
from __future__ import annotations

import argparse
import os
import sys
from collections import deque

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Patch

_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, _ROOT)
sys.path.insert(0, os.path.join(_ROOT, "build"))
sys.path.insert(0, os.path.join(_ROOT, "python"))

from sb3_contrib import RecurrentPPO
from stoneforge_env import StoneforgeWorldEnv, env_kwargs_for_model

PASSABLE_TILES = {0, 3}  # TileType.Empty, TileType.Exit

C_WALL = "#4A4A4A"
C_FLOOR = "#EDEEF0"
C_AGENT = "#EB6834"   # identisch zu C_LSTM in plot_eval_results.py
C_BFS = "#1BAF7A"     # identisch zu C_DET in plot_v12_learning_curves.py
C_SPAWN = "#222222"
C_EXIT = "#222222"


def rollout(model, env, seed: int, max_steps: int = 4000):
    obs, _ = env.reset(seed=seed)
    positions = [env.core.player_pos()]
    lstm_state = None
    episode_start = np.ones((1,), dtype=bool)
    done = False
    reached = False
    step = 0
    while not done and step < max_steps:
        action, lstm_state = model.predict(
            obs, state=lstm_state, episode_start=episode_start, deterministic=False,
        )
        episode_start = np.zeros((1,), dtype=bool)
        obs, _, term, trunc, info = env.step(int(action))
        positions.append(env.core.player_pos())
        done = term or trunc
        reached = reached or bool(info.get("reached_exit"))
        step += 1
    return positions, reached


def bfs_shortest_path(passable: np.ndarray, start, goal):
    h, w = passable.shape
    prev = {start: None}
    q = deque([start])
    while q:
        cur = q.popleft()
        if cur == goal:
            break
        cx, cy = cur
        for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)):
            nx, ny = cx + dx, cy + dy
            if 0 <= nx < w and 0 <= ny < h and passable[ny, nx] and (nx, ny) not in prev:
                prev[(nx, ny)] = cur
                q.append((nx, ny))
    if goal not in prev:
        return None
    path = [goal]
    while path[-1] != start:
        path.append(prev[path[-1]])
    return list(reversed(path))


def build_grid(env, positions, exit_pos, margin=6):
    xs = [p[0] for p in positions] + [exit_pos[0]]
    ys = [p[1] for p in positions] + [exit_pos[1]]
    x0, x1 = min(xs) - margin, max(xs) + margin
    y0, y1 = min(ys) - margin, max(ys) + margin
    w, h = x1 - x0 + 1, y1 - y0 + 1
    passable = np.zeros((h, w), dtype=bool)
    for gy in range(h):
        for gx in range(w):
            wx, wy = x0 + gx, y0 + gy
            passable[gy, gx] = env.core.tile_at(wx, wy) in PASSABLE_TILES
    return passable, x0, y0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", default="models/ppo_lstm_curriculum_v12_s1/best_model.zip")
    ap.add_argument("--seeds", default="7000,7001,7002,7003,7004,7005,7006,7007,7008,7009")
    ap.add_argument("-o", "--out", default="docs/Doku/Bilder/agent_pfad_vs_bfs.png")
    args = ap.parse_args()

    model_path = os.path.join(_ROOT, args.model) if not os.path.isabs(args.model) else args.model
    model = RecurrentPPO.load(model_path, device="cpu")
    env_kwargs = env_kwargs_for_model(model)
    env = StoneforgeWorldEnv(exit_min=35, exit_max=45, **env_kwargs)

    chosen = None
    for seed in (int(s) for s in args.seeds.split(",")):
        positions, reached = rollout(model, env, seed)
        if reached and len(positions) < 3000:
            chosen = (seed, positions)
            break
        print(f"seed {seed}: reached={reached}, steps={len(positions)-1} -- skip")

    if chosen is None:
        raise SystemExit("Keine der Test-Seeds fuehrte zu einem kompakt darstellbaren Erfolg.")

    seed, positions = chosen
    exit_pos = tuple(env.core.exit_pos())
    spawn_pos = positions[0]
    passable, x0, y0 = build_grid(env, positions, exit_pos)
    bfs_path = bfs_shortest_path(passable, (spawn_pos[0] - x0, spawn_pos[1] - y0),
                                  (exit_pos[0] - x0, exit_pos[1] - y0))
    if bfs_path is None:
        raise SystemExit("BFS-Pfad in nachgebautem Ausschnitt nicht gefunden (Rand zu knapp?).")

    agent_steps = len(positions) - 1
    bfs_steps = len(bfs_path) - 1
    efficiency = bfs_steps / agent_steps

    fig, ax = plt.subplots(figsize=(11, 9.5))
    ax.imshow(~passable, cmap="Greys", vmin=0, vmax=1.4, origin="upper",
              extent=(x0 - 0.5, x0 + passable.shape[1] - 0.5, y0 + passable.shape[0] - 0.5, y0 - 0.5))

    ax_x = [p[0] for p in positions]
    ax_y = [p[1] for p in positions]
    ax.plot(ax_x, ax_y, color=C_AGENT, linewidth=1.6, alpha=0.85, zorder=3,
            label=f"Agent (LSTM-PPO, {agent_steps} Schritte)")

    bx = [x0 + p[0] for p in bfs_path]
    by = [y0 + p[1] for p in bfs_path]
    ax.plot(bx, by, color=C_BFS, linewidth=2.4, zorder=4,
            label=f"BFS-Optimum ({bfs_steps} Schritte)")

    ax.scatter([spawn_pos[0]], [spawn_pos[1]], color=C_SPAWN, s=90, zorder=5, marker="o")
    ax.annotate("Start", (spawn_pos[0], spawn_pos[1]), textcoords="offset points",
                xytext=(8, 8), fontsize=12, fontweight="bold")
    ax.scatter([exit_pos[0]], [exit_pos[1]], color=C_EXIT, s=140, zorder=5, marker="*")
    ax.annotate("Ausgang", (exit_pos[0], exit_pos[1]), textcoords="offset points",
                xytext=(8, 8), fontsize=12, fontweight="bold")

    ax.set_title(f"Gelaufener Pfad gegen BFS-Optimum (Testset A, Seed {seed}, "
                 f"Pfadeffizienz {efficiency:.3f})", fontsize=14, fontweight="bold")
    ax.set_xticks([]); ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_visible(False)
    ax.legend(loc="upper left", fontsize=12, frameon=True)

    fig.tight_layout()
    out = os.path.join(_ROOT, args.out) if not os.path.isabs(args.out) else args.out
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"gespeichert: {out}  (seed={seed}, agent={agent_steps}, bfs={bfs_steps}, eff={efficiency:.4f})")


if __name__ == "__main__":
    main()
