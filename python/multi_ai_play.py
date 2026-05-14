from __future__ import annotations

import argparse
import os
import platform
import subprocess
import sys
import time
import threading

from stable_baselines3 import DQN, PPO

from stoneforge_env import ExitPotentialFieldWrapper, StoneforgeConfig, StoneforgeWorldEnv

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

_IS_WIN = platform.system() == "Windows"
_EXE_NAME = "stoneforge_client.exe" if _IS_WIN else "stoneforge_client"
GAME_BINARY = os.path.join(PROJECT_ROOT, "build", _EXE_NAME)

WIN_W = 640
WIN_H = 480
WIN_GAP = 8


def load_model(path: str) -> PPO | DQN:
    try:
        return PPO.load(path)
    except Exception:
        return DQN.load(path)


def sanitize_action(action: int) -> int:
    # Mining-Aktion (4) blockieren — Agent soll nur navigieren.
    return 7 if action == 4 else action


def run_agent(model: PPO | DQN, label: str, seed: int, speed: float,
              win_x: int, win_y: int, stop_event: threading.Event,
              disable_mobs: bool = True) -> None:
    """Laeuft in einem eigenen Thread: steuert ein Raylib-Fenster."""
    cfg = StoneforgeConfig(disable_mobs=disable_mobs)
    env = ExitPotentialFieldWrapper(StoneforgeWorldEnv(cfg))
    obs, _ = env.reset(seed=seed)

    # Erwartete Obs-Shape aus dem Modell lesen (fuer Kompatibilitaet mit alten Checkpoints).
    expected_shape = tuple(getattr(model, "observation_space", env.observation_space).shape)

    binary_args = [GAME_BINARY, "--ai", "--seed", str(seed),
                   "--title", label,
                   "--window-pos", str(win_x), str(win_y),
                   "--window-size", str(WIN_W), str(WIN_H)]
    if disable_mobs:
        binary_args.append("--no-monsters")

    game = subprocess.Popen(binary_args, stdin=subprocess.PIPE, text=True,
                            bufsize=1, cwd=PROJECT_ROOT)
    time.sleep(1.0)

    step_delay = 0.1 / speed

    try:
        while not stop_event.is_set() and game.poll() is None:
            # Obs-Groesse anpassen falls altes Modell geladen wurde.
            if obs.shape != expected_shape:
                import numpy as np
                if obs.size > expected_shape[0]:
                    use_obs = obs[:expected_shape[0]]
                else:
                    use_obs = np.zeros(expected_shape, dtype=obs.dtype)
                    use_obs[:obs.size] = obs
            else:
                use_obs = obs

            action, _ = model.predict(use_obs, deterministic=True)
            safe_action = sanitize_action(int(action))
            game.stdin.write(f"{safe_action}\n")
            game.stdin.flush()

            obs, _r, terminated, truncated, _ = env.step(safe_action)
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
        description="Oeffnet mehrere Raylib-Fenster nebeneinander — je eines pro Checkpoint."
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
                        help="Fenstertitel fuer jedes Checkpoint (optional)")
    parser.add_argument("--seed",  type=int,   default=42)
    parser.add_argument("--speed", type=float, default=1.0,
                        help="Geschwindigkeitsfaktor (1.0 = normal, 2.0 = doppelt)")
    parser.add_argument("--start-x", type=int, default=40)
    parser.add_argument("--start-y", type=int, default=100)
    parser.add_argument("--monsters", action="store_true",
                        help="Monster aktivieren (Standard: aus)")
    args = parser.parse_args()

    if not os.path.exists(GAME_BINARY):
        print(f"Fehler: Binary nicht gefunden: {GAME_BINARY}", file=sys.stderr)
        sys.exit(1)

    disable_mobs = not args.monsters

    entries = []
    for idx, path in enumerate(args.checkpoints):
        if not os.path.exists(path):
            print(f"Ueberspringe (nicht gefunden): {path}", file=sys.stderr)
            continue
        label = args.labels[idx] if idx < len(args.labels) else os.path.basename(path)
        try:
            model = load_model(path)
            entries.append((model, label))
            print(f"  OK  {label}")
        except Exception as e:
            print(f"  FEHLER {path}: {e}", file=sys.stderr)

    if not entries:
        print("Keine Modelle geladen.", file=sys.stderr)
        sys.exit(1)

    print(f"\n{len(entries)} Fenster werden geoeffnet — nebeneinander angeordnet.")
    print(f"Monster: {'AN' if args.monsters else 'AUS'}")
    print("Strg+C zum Beenden.\n")

    stop_event = threading.Event()
    threads = []

    for i, (model, label) in enumerate(entries):
        wx = args.start_x + i * (WIN_W + WIN_GAP)
        wy = args.start_y
        t = threading.Thread(
            target=run_agent,
            args=(model, label, args.seed, args.speed, wx, wy, stop_event, disable_mobs),
            daemon=True,
        )
        threads.append(t)

    for t in threads:
        t.start()
        time.sleep(0.4)

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
