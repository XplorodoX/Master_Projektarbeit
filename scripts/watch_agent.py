"""Stoneforge RL — Agent im Spiel beobachten.

Startet den grafischen C++-Client (build/stoneforge_client --ai) und
steuert ihn per stdin mit dem trainierten PPO-Modell.

Verwendung:
    python scripts/watch_agent.py
    python scripts/watch_agent.py --model models/ppo_phase4/best_model.zip
    python scripts/watch_agent.py --seeds 7000 7001 7002 --speed 0.15
    python scripts/watch_agent.py --episodes 5 --seed 7042
"""
from __future__ import annotations

import argparse
import subprocess
import sys
import time

import numpy as np
import torch
from stable_baselines3 import A2C, DQN, PPO
from sb3_contrib import RecurrentPPO

from stoneforge_env import StoneforgeWorldEnv


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Watch trained agent in game")
    p.add_argument("--model", default="models/ppo_phase4/best_model.zip",
                   help="Pfad zum Modell (.zip) — PPO, A2C oder DQN (wird automatisch erkannt)")
    p.add_argument("--seed", type=int, default=7000,
                   help="Startseed (wird für alle Episoden inkrementiert)")
    p.add_argument("--episodes", type=int, default=10,
                   help="Anzahl Episoden die gezeigt werden")
    p.add_argument("--exit-min", type=int, default=35)
    p.add_argument("--exit-max", type=int, default=45)
    p.add_argument("--speed", type=float, default=0.12,
                   help="Sekunden pro Schritt (0.05=schnell, 0.2=langsam)")
    p.add_argument("--temperature", type=float, default=0.2,
                   help="Temperatur fürs Sampling, wenn --deterministic nicht gesetzt ist")
    p.add_argument("--deterministic", action="store_true",
                   help="Deterministisch (argmax) statt stochastisch — kann Loops erzeugen")
    p.add_argument("--monsters", action="store_true",
                   help="Monster aktivieren (Standard: aus)")
    p.add_argument("--client", default="build/stoneforge_client",
                   help="Pfad zum stoneforge_client Binary")
    p.add_argument("--no-live-map", action="store_true",
                   help="Live Map Server NICHT starten (default: immer an)")
    p.add_argument("--live-map-port", type=int, default=8642,
                   help="Port des Live Map Servers (default: 8642)")
    return p.parse_args()


ACT_NAMES = {0: '↑', 1: '↓', 2: '←', 3: '→'}


def bfs_best_action(env: StoneforgeWorldEnv) -> int:
    """Gibt die BFS-optimale Aktion zurück (niedrigste Nachbar-Distanz)."""
    nbrs = [env.core.bfs_distance_at_offset(dx, dy)
            for dx, dy in [(0, -1), (0, 1), (-1, 0), (1, 0)]]
    return int(np.argmin(nbrs))


def sample_temperature_action(model, obs: np.ndarray, temperature: float) -> int:
    if temperature <= 0:
        action, _ = model.predict(obs, deterministic=True)
        return int(action)

    obs_tensor, _ = model.policy.obs_to_tensor(obs)
    with torch.no_grad():
        distribution = model.policy.get_distribution(obs_tensor)
        logits = distribution.distribution.logits
        scaled_logits = logits / float(temperature)
        probs = torch.softmax(scaled_logits, dim=-1)
        action = torch.multinomial(probs, num_samples=1)
    return int(action.squeeze(0).item())


def main() -> None:
    args = parse_args()

    # Live Map Server starten (default: immer, außer --no-live-map)
    _send_update = None
    if not args.no_live_map:
        import sys, os
        sys.path.insert(0, os.path.dirname(__file__))
        from live_map_server import start_server, send_update as _su
        start_server(args.live_map_port)
        _send_update = lambda data: _su(data, args.live_map_port)
        print(f"  Live Map: http://localhost:{args.live_map_port}")

    print(f"Lade Modell: {args.model}")
    model = None
    model_name = ""
    is_recurrent = False
    for Cls, name in [(RecurrentPPO, "RecurrentPPO (LSTM)"), (PPO, "PPO"), (A2C, "A2C"), (DQN, "DQN")]:
        try:
            model = Cls.load(args.model)
            model_name = name
            is_recurrent = (Cls is RecurrentPPO)
            break
        except Exception:
            pass
    if model is None:
        print(f"Fehler: Konnte Modell nicht laden: {args.model}", file=sys.stderr)
        sys.exit(1)
    print(f"Algorithmus: {model_name}")

    env = StoneforgeWorldEnv(exit_min=args.exit_min, exit_max=args.exit_max)

    client_cmd = [
        args.client,
        "--ai",
        "--ai-seed", str(args.seed),
        "--title", "Stoneforge RL — Agent Watch",
    ]
    if not args.monsters:
        client_cmd.append("--no-monsters")
    print(f"Starte Client: {' '.join(client_cmd)}")
    client = subprocess.Popen(
        client_cmd,
        stdin=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

    successes = 0
    try:
        lstm_states = None
        ep_start = np.ones((1,), dtype=bool)

        for ep in range(args.episodes):
            seed = args.seed + ep
            obs, _ = env.reset(seed=seed)
            done = False
            steps = 0
            ep_ret = 0.0
            reached = False
            last_pos = None
            stuck_steps = 0
            lstm_states = None
            ep_start = np.ones((1,), dtype=bool)

            if _send_update:
                _send_update({
                    "episode": ep + 1, "seed": seed, "step": 0,
                    "x": 0, "y": 0, "bfs": env.core.current_bfs_distance_to_exit(),
                    "success": None, "successes": successes,
                    "total_eps": ep, "sr": successes / max(ep, 1),
                    "exit_dx": float(obs[228]), "exit_dy": float(obs[229]),
                    "running": True,
                })

            print(f"\n--- Episode {ep+1} (seed={seed}) ---")
            while not done and steps < 4000:
                if is_recurrent:
                    action_arr, lstm_states = model.predict(
                        obs.reshape(1, -1), state=lstm_states,
                        episode_start=ep_start, deterministic=args.deterministic,
                    )
                    action = int(action_arr[0])
                    ep_start = np.zeros((1,), dtype=bool)
                elif args.deterministic:
                    action, _ = model.predict(obs, deterministic=True)
                    action = int(action)
                else:
                    action = sample_temperature_action(model, obs, args.temperature)

                # BFS-Fallback: wenn Agent >= 4 Schritte an derselben Position feststeckt,
                # erzwinge BFS-optimale Richtung statt Policy
                pos = env.core.player_pos()
                if pos == last_pos:
                    stuck_steps += 1
                else:
                    stuck_steps = 0
                last_pos = pos

                if stuck_steps >= 4:
                    action = bfs_best_action(env)

                # Aktion an C++-Client senden
                client.stdin.write(f"{action}\n")
                client.stdin.flush()

                obs, reward, term, trunc, info = env.step(action)
                bfs = env.core.current_bfs_distance_to_exit()
                ep_ret += float(reward)
                steps += 1
                if info.get("reached_exit", False):
                    reached = True

                done = term or trunc

                if _send_update:
                    cur = env.core.player_pos()
                    _send_update({
                        "episode": ep + 1, "seed": seed, "step": steps,
                        "x": cur[0], "y": cur[1], "bfs": bfs,
                        "success": True if reached else None,
                        "successes": successes + int(reached),
                        "total_eps": ep + 1,
                        "sr": (successes + int(reached)) / (ep + 1),
                        "exit_dx": float(obs[228]),
                        "exit_dy": float(obs[229]),
                        "running": True,
                    })

                fallback = " [BFS-Fallback]" if stuck_steps >= 4 else ""
                temp_tag = "det" if args.deterministic else f"tau={args.temperature:g}"
                print(
                    f"  Step {steps:3d} | pos={str(pos):>10} | BFS={bfs:3d} | "
                    f"act={ACT_NAMES[action]}{fallback} | {temp_tag}"
                )

                if reached:
                    successes += 1
                    print(f"  => EXIT GEFUNDEN ✓ | steps={steps} | return={ep_ret:.1f}")
                    time.sleep(1.5)   # kurz warten damit man es sieht
                    client.terminate()
                    break

                time.sleep(args.speed)

                if client.poll() is not None:
                    print("Client wurde geschlossen.")
                    return

            if not reached:
                status = "Timeout ✗"
                bfs = env.core.current_bfs_distance_to_exit()
                print(f"  => {status} | steps={steps} | bfs_remaining={bfs}")
                if _send_update:
                    cur = env.core.player_pos()
                    _send_update({
                        "success": False, "successes": successes,
                        "total_eps": ep + 1, "sr": successes / (ep + 1),
                        "x": cur[0], "y": cur[1], "running": True,
                    })
                client.terminate()

    except BrokenPipeError:
        print("Client geschlossen.")
    except KeyboardInterrupt:
        print("\nAbgebrochen.")
    finally:
        if client.poll() is None:
            client.stdin.close()
            try:
                client.wait(timeout=10)
            except Exception:
                client.kill()

    print(f"\nGesamt: {successes}/{args.episodes} Exits gefunden ({successes/args.episodes:.1%})")


if __name__ == "__main__":
    main()
