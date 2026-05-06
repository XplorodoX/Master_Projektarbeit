from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time
import threading

from stable_baselines3 import DQN, PPO

from stoneforge_env import StoneforgeWorldEnv

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME_BINARY  = os.path.join(PROJECT_ROOT, "build", "stoneforge_client")

# Fenstergröße pro Panel (passt 3x nebeneinander auf Full-HD)
WIN_W = 640
WIN_H = 480
WIN_GAP = 8   # Pixel Abstand zwischen Fenstern


def load_model(path: str) -> PPO | DQN:
    try:
        return PPO.load(path)
    except Exception:
        return DQN.load(path)


def run_agent(model: PPO, label: str, seed: int, speed: float,
              win_x: int, win_y: int, stop_event: threading.Event) -> None:
    """Läuft in einem eigenen Thread: steuert ein Raylib-Fenster."""
    env = StoneforgeWorldEnv()
    obs, _ = env.reset(seed=seed)

    cmd = [
        GAME_BINARY,
        "--ai",
        "--seed",    str(seed),
        "--title",   label,
        "--window-pos", str(win_x), str(win_y),
        "--window-size", str(WIN_W), str(WIN_H),
    ]
    game = subprocess.Popen(cmd, stdin=subprocess.PIPE, text=True,
                            bufsize=1, cwd=PROJECT_ROOT)
    time.sleep(1.0)   # Fenster öffnen lassen

    step_delay = 0.1 / speed

    try:
        while not stop_event.is_set() and game.poll() is None:
            action, _ = model.predict(obs, deterministic=True)
            game.stdin.write(f"{int(action)}\n")
            game.stdin.flush()

            obs, _r, terminated, truncated, _ = env.step(int(action))
            if terminated or truncated:
                obs, _ = env.reset(seed=seed)

            time.sleep(step_delay)
    except (BrokenPipeError, KeyboardInterrupt):
        pass
    finally:
        if game.poll() is None:
            game.terminate()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Öffnet mehrere Raylib-Fenster nebeneinander — je eines pro Checkpoint."
    )
    parser.add_argument(
        "--checkpoints", nargs="+",
        default=[
            "best_models_ppo/best_model.zip",
            "best_models_dqn/best_model.zip",
        ],
        help="Liste von Modell-Pfaden (Leerzeichen-getrennt)",
    )
    parser.add_argument("--labels", nargs="+", default=[],
                        help="Fenstertitel für jedes Checkpoint (optional)")
    parser.add_argument("--seed",  type=int,   default=42)
    parser.add_argument("--speed", type=float, default=1.0,
                        help="Geschwindigkeitsfaktor (1.0 = normal, 2.0 = doppelt)")
    parser.add_argument("--start-x", type=int, default=40,
                        help="X-Position des ersten Fensters")
    parser.add_argument("--start-y", type=int, default=100,
                        help="Y-Position des ersten Fensters")
    args = parser.parse_args()

    if not os.path.exists(GAME_BINARY):
        print(f"Fehler: Binary nicht gefunden: {GAME_BINARY}", file=sys.stderr)
        sys.exit(1)

    # Checkpoints laden
    entries = []
    for idx, path in enumerate(args.checkpoints):
        if not os.path.exists(path):
            print(f"Überspringe (nicht gefunden): {path}", file=sys.stderr)
            continue
        label = args.labels[idx] if idx < len(args.labels) else os.path.basename(path)
        try:
            model = load_model(path)
            entries.append((model, label))
            print(f"  ✓ {label}")
        except Exception as e:
            print(f"  ✗ {path}: {e}", file=sys.stderr)

    if not entries:
        print("Keine Modelle geladen.", file=sys.stderr)
        sys.exit(1)

    print(f"\n{len(entries)} Fenster werden geöffnet — nebeneinander angeordnet.")
    print("Strg+C zum Beenden.\n")

    stop_event = threading.Event()
    threads = []

    for i, (model, label) in enumerate(entries):
        wx = args.start_x + i * (WIN_W + WIN_GAP)
        wy = args.start_y
        t = threading.Thread(
            target=run_agent,
            args=(model, label, args.seed, args.speed, wx, wy, stop_event),
            daemon=True,
        )
        threads.append(t)

    # Alle Threads starten
    for t in threads:
        t.start()
        time.sleep(0.4)   # Kurze Pause damit die Fenster nacheinander aufgehen

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
