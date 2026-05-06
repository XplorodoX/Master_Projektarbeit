from __future__ import annotations

import argparse
import os
import subprocess
import sys
import threading
import time
from dataclasses import dataclass, field

from stable_baselines3 import DQN, PPO

from stoneforge_env import StoneforgeWorldEnv

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME_BINARY  = os.path.join(PROJECT_ROOT, "build", "stoneforge_client")

WIN_W   = 640
WIN_H   = 480
WIN_GAP = 8

STATS_INTERVAL = 5.0  # Sekunden zwischen Konsolen-Updates


@dataclass
class AgentStats:
    label:     str
    ep_reward: float = 0.0
    ep_steps:  int   = 0
    wins:      int   = 0
    deaths:    int   = 0
    total_eps: int   = 0
    lock:      threading.Lock = field(default_factory=threading.Lock)


def load_model(path: str) -> PPO | DQN:
    try:
        return PPO.load(path)
    except Exception:
        return DQN.load(path)


def run_agent(
    model: PPO | DQN,
    stats: AgentStats,
    seed: int,
    speed: float,
    win_x: int,
    win_y: int,
    stop_event: threading.Event,
) -> None:
    env = StoneforgeWorldEnv()
    obs, _ = env.reset(seed=seed)

    cmd = [
        GAME_BINARY,
        "--ai",
        "--seed",        str(seed),
        "--title",       stats.label,
        "--window-pos",  str(win_x), str(win_y),
        "--window-size", str(WIN_W), str(WIN_H),
    ]
    game = subprocess.Popen(cmd, stdin=subprocess.PIPE, text=True,
                            bufsize=1, cwd=PROJECT_ROOT)
    time.sleep(1.0)

    step_delay = 0.1 / speed

    try:
        while not stop_event.is_set() and game.poll() is None:
            action, _ = model.predict(obs, deterministic=True)
            game.stdin.write(f"{int(action)}\n")
            game.stdin.flush()

            obs, reward, terminated, truncated, info = env.step(int(action))

            with stats.lock:
                stats.ep_reward += float(reward)
                stats.ep_steps  += 1

            if terminated or truncated:
                with stats.lock:
                    stats.total_eps += 1
                    if info.get("reached_exit", False):
                        stats.wins   += 1
                    else:
                        stats.deaths += 1
                    stats.ep_reward = 0.0
                    stats.ep_steps  = 0
                obs, _ = env.reset(seed=seed)

            time.sleep(step_delay)
    except (BrokenPipeError, KeyboardInterrupt):
        pass
    finally:
        if game.poll() is None:
            game.terminate()


def print_stats(agents: list[AgentStats], stop_event: threading.Event) -> None:
    while not stop_event.is_set():
        time.sleep(STATS_INTERVAL)
        if stop_event.is_set():
            break

        lines = ["\n=== Stoneforge KI — Live-Stats ==="]
        for s in agents:
            with s.lock:
                win_rate = (s.wins / s.total_eps * 100) if s.total_eps > 0 else 0.0
                lines.append(
                    f"  [{s.label}]  Eps: {s.total_eps}  "
                    f"Wins: {s.wins}  Deaths: {s.deaths}  "
                    f"Win-Rate: {win_rate:.1f}%  "
                    f"Reward (lfd.): {s.ep_reward:+.1f}  Steps: {s.ep_steps}"
                )
        print("\n".join(lines))


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Vergleicht mehrere RL-Agenten nebeneinander in Raylib-Fenstern."
    )
    parser.add_argument(
        "--checkpoints", nargs="+",
        default=[
            "best_models_ppo/best_model.zip",
            "best_models_dqn/best_model.zip",
        ],
        help="Pfade zu Modell-Dateien (Leerzeichen-getrennt)",
    )
    parser.add_argument("--labels", nargs="+", default=[],
                        help="Fenstertitel für jeden Agenten (optional)")
    parser.add_argument("--seed",    type=int,   default=42)
    parser.add_argument("--speed",   type=float, default=1.0,
                        help="Geschwindigkeitsfaktor (1.0 = normal, 2.0 = doppelt)")
    parser.add_argument("--start-x", type=int,   default=40)
    parser.add_argument("--start-y", type=int,   default=100)
    args = parser.parse_args()

    if not os.path.exists(GAME_BINARY):
        print(f"Fehler: Binary nicht gefunden: {GAME_BINARY}", file=sys.stderr)
        print("Bitte zuerst bauen: cmake --build build --target stoneforge_client",
              file=sys.stderr)
        sys.exit(1)

    entries: list[tuple[PPO | DQN, AgentStats]] = []
    for idx, path in enumerate(args.checkpoints):
        if not os.path.exists(path):
            print(f"Überspringe (nicht gefunden): {path}", file=sys.stderr)
            continue
        label = args.labels[idx] if idx < len(args.labels) else os.path.basename(path)
        try:
            model = load_model(path)
            entries.append((model, AgentStats(label=label)))
            print(f"  ✓ {label}  ({path})")
        except Exception as exc:
            print(f"  ✗ {path}: {exc}", file=sys.stderr)

    if not entries:
        print("Keine Modelle geladen.", file=sys.stderr)
        sys.exit(1)

    print(f"\n{len(entries)} Fenster werden geöffnet.")
    print(f"Live-Stats alle {STATS_INTERVAL:.0f}s in der Konsole. Strg+C zum Beenden.\n")

    stop_event = threading.Event()
    threads: list[threading.Thread] = []

    stats_thread = threading.Thread(
        target=print_stats,
        args=([s for _, s in entries], stop_event),
        daemon=True,
    )
    threads.append(stats_thread)

    for i, (model, stats) in enumerate(entries):
        wx = args.start_x + i * (WIN_W + WIN_GAP)
        wy = args.start_y
        t = threading.Thread(
            target=run_agent,
            args=(model, stats, args.seed, args.speed, wx, wy, stop_event),
            daemon=True,
        )
        threads.append(t)

    for t in threads:
        t.start()
        time.sleep(0.3)

    try:
        for t in threads:
            t.join()
    except KeyboardInterrupt:
        print("\nBeende alle Fenster...")
        stop_event.set()
        for t in threads:
            t.join(timeout=3.0)


if __name__ == "__main__":
    main()
