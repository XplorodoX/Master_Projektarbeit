"""Verhaltensanalyse des trainierten PPO-Agenten.

Zeigt pro Episode: Position-Trajektorie, BFS-Distanz, Reward, gewählte vs. optimale Aktion.
Erkennt Loops (↑↓-Fallen), Stagnation und Fortschritt.

Aufruf:
    python scripts/analyze_agent.py [--seeds 7000,7001,7002] [--model models/ppo_baseline/best_model.zip]
    python scripts/analyze_agent.py --exit-min 5 --exit-max 12 --model models/ppo_baseline/best_model.zip
"""
from __future__ import annotations

import argparse
import sys
import os
from collections import Counter, deque

import numpy as np

_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, _ROOT)
sys.path.insert(0, os.path.join(_ROOT, "build"))
sys.path.insert(0, os.path.join(_ROOT, "python"))

from stable_baselines3 import PPO, DQN
from stoneforge_env import StoneforgeWorldEnv

ACTION_NAMES = {0: "↑", 1: "↓", 2: "←", 3: "→"}
BFS_DIR_NAMES = ["cur", "↑", "↓", "←", "→"]


def optimal_action_from_bfs(env: StoneforgeWorldEnv) -> int:
    """Wählt die Richtung mit kleinstem BFS-Wert (optimal)."""
    bfs = [
        env.core.bfs_distance_at_offset( 0, -1),  # ↑ (action 0)
        env.core.bfs_distance_at_offset( 0,  1),  # ↓ (action 1)
        env.core.bfs_distance_at_offset(-1,  0),  # ← (action 2)
        env.core.bfs_distance_at_offset( 1,  0),  # → (action 3)
    ]
    return int(np.argmin(bfs)), bfs


def analyze_episode(model, env: StoneforgeWorldEnv, seed: int, max_steps: int = 200,
                    verbose: bool = True) -> dict:
    obs, _ = env.reset(seed=seed)

    positions = []
    bfs_history = []
    rewards = []
    actions_taken = []
    optimal_actions = []
    bfs_raw_history = []

    done = False
    step = 0
    reached = False
    recent_pos = deque(maxlen=6)

    while not done and step < max_steps:
        pos = env.core.player_pos()
        bfs_cur = env.core.current_bfs_distance_to_exit()
        opt_action, bfs_neighbors = optimal_action_from_bfs(env)

        positions.append(pos)
        bfs_history.append(bfs_cur)
        optimal_actions.append(opt_action)
        bfs_raw_history.append(bfs_neighbors)
        recent_pos.append(pos)

        if isinstance(model, BfsGreedyPolicy):
            action = opt_action
        elif isinstance(model, RandomPolicy):
            action = int(model.rng.integers(0, 4))
        else:
            action, _ = model.predict(obs, deterministic=True)
            action = int(action)
        actions_taken.append(action)

        obs, reward, term, trunc, info = env.step(action)
        rewards.append(reward)
        done = term or trunc
        if info.get("reached_exit"):
            reached = True
        step += 1

    # Auswertung
    action_counts = Counter(actions_taken)
    correct_count = sum(a == o for a, o in zip(actions_taken, optimal_actions))
    correct_pct = correct_count / max(1, len(actions_taken)) * 100

    # Loop-Erkennung: Wie oft wiederholt sich eine Position?
    pos_counts = Counter(positions)
    max_repeats = max(pos_counts.values()) if pos_counts else 0
    loop_positions = [(p, c) for p, c in pos_counts.items() if c >= 4]

    bfs_start = bfs_history[0] if bfs_history else 0
    bfs_end = bfs_history[-1] if bfs_history else 0
    bfs_best = min(bfs_history) if bfs_history else 0
    total_reward = sum(rewards)

    if verbose:
        print(f"\n{'='*60}")
        print(f"Seed {seed} | Steps: {step} | Reached: {reached}")
        print(f"BFS: {bfs_start} → {bfs_end} (best: {bfs_best})")
        print(f"Total reward: {total_reward:.2f}")
        print(f"Actions: { {ACTION_NAMES[k]: v for k, v in sorted(action_counts.items())} }")
        print(f"Optimal-Align: {correct_count}/{len(actions_taken)} ({correct_pct:.1f}%)")
        print(f"Max pos repeats: {max_repeats} | Loop positions: {len(loop_positions)}")

        # Erste 30 Schritte im Detail
        print(f"\n  Step | Pos        | BFS | Opt | Act | Reward | BFS-Nbrs [↑↓←→]")
        print(f"  -----|------------|-----|-----|-----|--------|------------------")
        n_show = min(30, len(positions))
        for i in range(n_show):
            a_sym = ACTION_NAMES[actions_taken[i]]
            o_sym = ACTION_NAMES[optimal_actions[i]]
            match = "✓" if actions_taken[i] == optimal_actions[i] else "✗"
            nbrs = bfs_raw_history[i]
            print(f"  {i+1:4d} | ({positions[i][0]:3d},{positions[i][1]:3d}) | "
                  f"{bfs_history[i]:3d} | {o_sym}  | {a_sym} {match}| "
                  f"{rewards[i]:+.3f} | {nbrs}")

        if loop_positions:
            print(f"\n  Häufig besuchte Positionen (≥4x):")
            for p, c in sorted(loop_positions, key=lambda x: -x[1])[:5]:
                print(f"    {p}: {c}x")

    return {
        "seed": seed,
        "steps": step,
        "reached": reached,
        "bfs_start": bfs_start,
        "bfs_end": bfs_end,
        "bfs_best": bfs_best,
        "total_reward": total_reward,
        "correct_pct": correct_pct,
        "max_repeats": max_repeats,
        "loop_count": len(loop_positions),
        "action_counts": dict(action_counts),
    }


class BfsGreedyPolicy:
    """Oracle: wählt immer die Richtung mit kleinstem BFS-Wert."""
    def predict(self, obs, deterministic=True):
        return None, None  # unused — handled specially in analyze_episode


class RandomPolicy:
    """Zufällige Policy (Baseline)."""
    rng = np.random.default_rng(42)
    def predict(self, obs, deterministic=True):
        return self.rng.integers(0, 4), None


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--model", default=None,
                   help="Pfad zu einem SB3-Modell (.zip). Leer → BFS-Greedy Oracle.")
    p.add_argument("--algo", choices=["ppo", "dqn"], default="ppo")
    p.add_argument("--policy", choices=["model", "bfs", "random"], default=None,
                   help="Welche Policy? (default: model wenn --model angegeben, sonst bfs)")
    p.add_argument("--seeds", default="7000,7001,7002,7003,7004")
    p.add_argument("--exit-min", type=int, default=5)
    p.add_argument("--exit-max", type=int, default=12)
    p.add_argument("--max-steps", type=int, default=200)
    args = p.parse_args()

    seeds = [int(s) for s in args.seeds.split(",")]

    # Policy auswählen
    if args.model:
        print(f"Lade Modell: {args.model}")
        ModelClass = PPO if args.algo == "ppo" else DQN
        try:
            model = ModelClass.load(args.model)
            policy_name = f"{args.algo.upper()} model"
        except Exception as e:
            print(f"  Fehler beim Laden: {e}")
            print("  Fallback: BFS-Greedy Oracle")
            model = BfsGreedyPolicy()
            policy_name = "BFS-Greedy Oracle (Fallback)"
    else:
        chosen = args.policy or "bfs"
        if chosen == "bfs":
            model = BfsGreedyPolicy()
            policy_name = "BFS-Greedy Oracle"
        else:
            model = RandomPolicy()
            policy_name = "Random Policy"
    print(f"Policy: {policy_name}")

    env = StoneforgeWorldEnv(exit_min=args.exit_min, exit_max=args.exit_max)

    results = []
    for seed in seeds:
        r = analyze_episode(model, env, seed, max_steps=args.max_steps, verbose=True)
        results.append(r)

    # Zusammenfassung
    print(f"\n{'='*60}")
    print(f"ZUSAMMENFASSUNG ({len(seeds)} Seeds, exit={args.exit_min}-{args.exit_max})")
    print(f"{'='*60}")
    success_rate = sum(r["reached"] for r in results) / len(results)
    mean_bfs_improvement = np.mean([r["bfs_start"] - r["bfs_best"] for r in results])
    mean_align = np.mean([r["correct_pct"] for r in results])
    mean_loops = np.mean([r["loop_count"] for r in results])
    mean_reward = np.mean([r["total_reward"] for r in results])

    print(f"Success Rate:       {success_rate:.1%} ({sum(r['reached'] for r in results)}/{len(results)})")
    print(f"BFS-Fortschritt:    Ø {mean_bfs_improvement:.1f} Tiles verbessert")
    print(f"Optimal-Alignment:  Ø {mean_align:.1f}%")
    print(f"Loop-Positionen:    Ø {mean_loops:.1f} Positionen mit ≥4x Besuchen")
    print(f"Mean Return:        {mean_reward:.2f}")

    # Globale Aktionsverteilung
    total_acts = Counter()
    for r in results:
        for k, v in r["action_counts"].items():
            total_acts[k] += v
    total = sum(total_acts.values())
    print(f"Aktionsverteilung:  { {ACTION_NAMES[k]: f'{v/total:.1%}' for k, v in sorted(total_acts.items())} }")


if __name__ == "__main__":
    main()
