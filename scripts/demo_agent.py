"""Stoneforge RL — Live-Demo: Der trainierte Agent spielt echte Runden im Spiel.

Zeigt die *reine* Policy (kein BFS-Orakel) im grafischen C++-Client. Standardmäßig
werden kuratierte, reproduzierbare Runden gespielt, in denen der Agent den Ausgang
zuverlässig findet — ideal zum Vorführen.

WICHTIG: Der C++-Client generiert seine Welt mit Exit-Distanz 35–45 (prozess-global).
Die Env hier MUSS dieselbe Distanz nutzen, sonst laufen Client-Welt und Agent
auseinander. Darum ist --exit 35/45 fest verdrahtet; bitte nicht ändern.

Vorführen (ein Befehl):
    source scripts/setup_env.sh
    python scripts/demo_agent.py

Optionen:
    python scripts/demo_agent.py --speed 0.05          # schneller
    python scripts/demo_agent.py --free 7009 7010       # eigene Seeds, echtes Zufalls-Sampling
    python scripts/demo_agent.py --deterministic        # sture Spielweise (schwächer, oft Timeout)
"""
from __future__ import annotations

import argparse
import subprocess
import time

import numpy as np
import torch
from sb3_contrib import RecurrentPPO

from stoneforge_env import StoneforgeWorldEnv, env_kwargs_for_model

DEFAULT_MODEL = "models/ppo_lstm_curriculum_v12_s3/best_model.zip"

# Kuratierte, reproduzierbare Erfolgs-Runden: (welt_seed, sampling_seed) → garantierter Exit.
# Ermittelt auf Testset-A-Welten (Distanz 35–45), kürzeste zuerst.
REPRO_RUNS = [
    (7025, 52),   # ~144 Schritte
    (7037, 55),   # ~188 Schritte
    (7000, 4),    # ~213 Schritte
    (7028, 60),   # ~280 Schritte
]


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Live-Demo: RL-Agent spielt Stoneforge")
    p.add_argument("--model", default=DEFAULT_MODEL)
    p.add_argument("--speed", type=float, default=0.08,
                   help="Sekunden pro Schritt (0.05=schnell, 0.15=langsam zum Erklären)")
    p.add_argument("--free", type=int, nargs="+", default=None,
                   help="Eigene Welt-Seeds mit echtem Zufalls-Sampling (Ausgang nicht garantiert)")
    p.add_argument("--deterministic", action="store_true",
                   help="Sture Spielweise (argmax) — schwächer, oft Timeout")
    p.add_argument("--client", default="build/stoneforge_client")
    p.add_argument("--window-size", type=int, nargs=2, default=[1100, 760])
    p.add_argument("--window-pos", type=int, nargs=2, default=[120, 80])
    return p.parse_args()


def start_client(args, seed: int) -> subprocess.Popen:
    cmd = [
        args.client, "--ai", "--ai-seed", str(seed), "--no-monsters",
        "--window-pos", str(args.window_pos[0]), str(args.window_pos[1]),
        "--window-size", str(args.window_size[0]), str(args.window_size[1]),
        "--title", f"Stoneforge RL — Demo (Seed {seed})",
    ]
    return subprocess.Popen(cmd, stdin=subprocess.PIPE, text=True, bufsize=1)


def play_round(args, model, env, world_seed: int, sampling_seed, idx: int, total: int) -> bool:
    if sampling_seed is not None:
        torch.manual_seed(sampling_seed)
        np.random.seed(sampling_seed)
    obs, _ = env.reset(seed=world_seed)
    start_bfs = env.core.bfs_distance_at_offset(0, 0)
    client = start_client(args, world_seed)
    time.sleep(3.5)

    print(f"\n── Runde {idx}/{total} · Welt {world_seed} · "
          f"Startdistanz {start_bfs} Felder ──")

    states = None
    ep_start = np.ones((1,), dtype=bool)
    steps = 0
    reached = False
    done = False
    try:
        while not done and steps < 4000:
            action_arr, states = model.predict(
                obs.reshape(1, -1), state=states,
                episode_start=ep_start, deterministic=args.deterministic,
            )
            ep_start = np.zeros((1,), dtype=bool)
            action = int(action_arr[0])
            client.stdin.write(f"{action}\n")
            client.stdin.flush()

            obs, _, term, trunc, info = env.step(action)
            steps += 1
            if info.get("reached_exit", False):
                reached = True
            done = term or trunc

            if steps % 25 == 0:
                bfs = env.core.current_bfs_distance_to_exit()
                print(f"   … {steps} Schritte, noch {bfs} Felder zum Ausgang")
            if reached:
                print(f"   ✓ AUSGANG GEFUNDEN nach {steps} Schritten!")
                time.sleep(1.6)
                break
            time.sleep(args.speed)
            if client.poll() is not None:
                print("   (Fenster geschlossen)")
                break
        if not reached and steps >= 4000:
            print("   ✗ Zeitlimit erreicht — diese Welt nicht gelöst")
    finally:
        if client.poll() is None:
            client.stdin.close()
            try:
                client.wait(timeout=8)
            except Exception:
                client.kill()
    time.sleep(0.7)
    return reached


def main() -> None:
    args = parse_args()
    if args.free:
        runs = [(s, None) for s in args.free]  # echtes Sampling
        mode = "eigene Seeds, Zufalls-Sampling"
    else:
        runs = REPRO_RUNS
        mode = "kuratierte Erfolgs-Runden"
    if args.deterministic:
        mode += " · sture Spielweise"

    print("=" * 64)
    print("  STONEFORGE RL — LIVE-DEMO")
    print(f"  Modell: {args.model}")
    print(f"  {mode}   ({len(runs)} Runden)")
    print("  Der Agent sieht nur 15×15 Felder + Zielrichtung — kein Wegorakel.")
    print("=" * 64)

    model = RecurrentPPO.load(args.model, device="cpu")
    kw = env_kwargs_for_model(model)
    env = StoneforgeWorldEnv(exit_min=35, exit_max=45, **kw)  # = Client-Distanz

    solved = 0
    try:
        for i, (ws, ss) in enumerate(runs, 1):
            solved += int(play_round(args, model, env, ws, ss, i, len(runs)))
    except KeyboardInterrupt:
        print("\nDemo abgebrochen.")
    finally:
        print("\n" + "=" * 64)
        print(f"  Ergebnis: {solved}/{len(runs)} Runden gelöst")
        print("=" * 64)


if __name__ == "__main__":
    main()
