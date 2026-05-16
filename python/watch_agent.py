"""Stoneforge RL — Agent im Spiel beobachten.

Startet den grafischen C++-Client (build/stoneforge_client --ai) und
steuert ihn per stdin mit dem trainierten PPO-Modell.

Verwendung:
    python python/watch_agent.py
    python python/watch_agent.py --model best_models_ppo_phase3/final_model.zip
    python python/watch_agent.py --seeds 7000 7001 7002 --speed 0.15
    python python/watch_agent.py --episodes 5 --seed 7042
"""
from __future__ import annotations

import argparse
import subprocess
import sys
import time

import numpy as np
from stable_baselines3 import PPO

from stoneforge_env import StoneforgeWorldEnv


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Watch trained agent in game")
    p.add_argument("--model", default="best_models_ppo_phase3/final_model.zip",
                   help="Pfad zum PPO-Modell (.zip)")
    p.add_argument("--seed", type=int, default=7000,
                   help="Startseed (wird für alle Episoden inkrementiert)")
    p.add_argument("--episodes", type=int, default=10,
                   help="Anzahl Episoden die gezeigt werden")
    p.add_argument("--exit-min", type=int, default=35)
    p.add_argument("--exit-max", type=int, default=45)
    p.add_argument("--speed", type=float, default=0.12,
                   help="Sekunden pro Schritt (0.05=schnell, 0.2=langsam)")
    p.add_argument("--deterministic", action="store_true",
                   help="Deterministisch (argmax) statt stochastisch — kann Loops erzeugen")
    p.add_argument("--client", default="build/stoneforge_client",
                   help="Pfad zum stoneforge_client Binary")
    return p.parse_args()


def main() -> None:
    args = parse_args()

    print(f"Lade Modell: {args.model}")
    model = PPO.load(args.model)

    env = StoneforgeWorldEnv(exit_min=args.exit_min, exit_max=args.exit_max)

    client_cmd = [
        args.client,
        "--ai",
        "--ai-seed", str(args.seed),
        "--no-monsters",
        "--title", "Stoneforge RL — Agent Watch",
    ]
    print(f"Starte Client: {' '.join(client_cmd)}")
    client = subprocess.Popen(
        client_cmd,
        stdin=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

    successes = 0
    try:
        for ep in range(args.episodes):
            seed = args.seed + ep
            obs, _ = env.reset(seed=seed)
            done = False
            steps = 0
            ep_ret = 0.0
            reached = False

            while not done and steps < 4000:
                action, _ = model.predict(obs, deterministic=args.deterministic)
                action = int(action)

                # Aktion an C++-Client senden
                client.stdin.write(f"{action}\n")
                client.stdin.flush()

                obs, reward, term, trunc, info = env.step(action)
                ep_ret += float(reward)
                steps += 1
                if info.get("reached_exit", False):
                    reached = True
                done = term or trunc

                time.sleep(args.speed)

                if client.poll() is not None:
                    print("Client wurde geschlossen.")
                    return

            successes += int(reached)
            status = "EXIT GEFUNDEN ✓" if reached else "Timeout ✗"
            bfs = env.core.current_bfs_distance_to_exit()
            print(
                f"  Episode {ep+1:2d} (seed={seed}): {status} | "
                f"steps={steps} | return={ep_ret:.1f} | bfs_remaining={bfs}"
            )

    except BrokenPipeError:
        print("Client geschlossen.")
    except KeyboardInterrupt:
        print("\nAbgebrochen.")
    finally:
        if client.poll() is None:
            client.stdin.close()
            client.wait(timeout=2)

    print(f"\nGesamt: {successes}/{args.episodes} Exits gefunden ({successes/args.episodes:.1%})")


if __name__ == "__main__":
    main()
