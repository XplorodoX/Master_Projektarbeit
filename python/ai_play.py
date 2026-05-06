from __future__ import annotations

import argparse
import os
import platform
import subprocess
import sys
import time

import numpy as np
from stable_baselines3 import DQN, PPO

from stoneforge_env import ExitPotentialFieldWrapper, StoneforgeWorldEnv


def make_play_env() -> StoneforgeWorldEnv:
    return ExitPotentialFieldWrapper(StoneforgeWorldEnv())  # type: ignore[return-value]

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Windows executables have .exe extension, Unix does not
_IS_WIN = platform.system() == "Windows"
_EXE_NAME = "stoneforge_client.exe" if _IS_WIN else "stoneforge_client"
GAME_BINARY = os.path.join(PROJECT_ROOT, "build", _EXE_NAME)


def load_model(path: str):
    try:
        model = PPO.load(path)
        return model, True
    except Exception:
        model = DQN.load(path)
        return model, True  # bestes Modell → immer deterministisch


def sanitize_action(action: int) -> int:
    # Block mining during playback; fallback to wait.
    return 7 if action == 4 else action


def run_single(model_path: str, seed: int, speed: float) -> None:
    model, deterministic = load_model(model_path)
    env = make_play_env()
    obs, _ = env.reset(seed=seed)
    print(f"Starte Spiel (Seed {seed}, Modell: {model_path}, deterministisch={deterministic})...")
    # Determine expected observation shape from the model (if available)
    expected_obs_space = getattr(model, "observation_space", None)
    if expected_obs_space is not None:
        expected_shape = tuple(expected_obs_space.shape)
    else:
        expected_shape = obs.shape
    game = subprocess.Popen(
        [GAME_BINARY, "--ai", "--seed", str(seed)],
        stdin=subprocess.PIPE, text=True, bufsize=1, cwd=PROJECT_ROOT,
    )
    time.sleep(1.5)
    step_delay = 0.1 / speed

    try:
        while game.poll() is None:
            # Adapt observation to model's expected shape (trim or pad)
            if obs.shape != expected_shape:
                if obs.size > expected_shape[0]:
                    use_obs = obs[: expected_shape[0]]
                else:
                    use_obs = np.zeros(expected_shape, dtype=obs.dtype)
                    use_obs[: obs.size] = obs
            else:
                use_obs = obs

            action, _ = model.predict(use_obs, deterministic=deterministic)
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


def run_dual(model1_path: str, model2_path: str, seed: int, speed: float) -> None:
    model1, det1 = load_model(model1_path)
    model2, det2 = load_model(model2_path)

    env1 = make_play_env()
    env2 = make_play_env()
    obs1, _ = env1.reset(seed=seed)
    obs2, _ = env2.reset(seed=seed)

    label1 = os.path.splitext(os.path.basename(model1_path))[0]
    label2 = os.path.splitext(os.path.basename(model2_path))[0]
    print(f"Starte Dual-Modus: [{label1}] vs [{label2}] (Seed {seed})")

    game = subprocess.Popen(
        [GAME_BINARY, "--ai-dual", "--seed", str(seed)],
        stdin=subprocess.PIPE, text=True, bufsize=1, cwd=PROJECT_ROOT,
    )
    time.sleep(1.5)
    step_delay = 0.1 / speed

    try:
        # Prepare expected shapes for both models
        exp1 = getattr(model1, "observation_space", None)
        exp2 = getattr(model2, "observation_space", None)
        s1 = tuple(exp1.shape) if exp1 is not None else obs1.shape
        s2 = tuple(exp2.shape) if exp2 is not None else obs2.shape

        while game.poll() is None:
            # Adapt obs1
            if obs1.shape != s1:
                if obs1.size > s1[0]:
                    in1 = obs1[: s1[0]]
                else:
                    in1 = np.zeros(s1, dtype=obs1.dtype)
                    in1[: obs1.size] = obs1
            else:
                in1 = obs1

            # Adapt obs2
            if obs2.shape != s2:
                if obs2.size > s2[0]:
                    in2 = obs2[: s2[0]]
                else:
                    in2 = np.zeros(s2, dtype=obs2.dtype)
                    in2[: obs2.size] = obs2
            else:
                in2 = obs2

            a1, _ = model1.predict(in1, deterministic=det1)
            a2, _ = model2.predict(in2, deterministic=det2)
            safe_a1 = sanitize_action(int(a1))
            safe_a2 = sanitize_action(int(a2))

            game.stdin.write(f"{safe_a1} {safe_a2}\n")
            game.stdin.flush()

            obs1, _r, t1, tr1, _ = env1.step(safe_a1)
            obs2, _r, t2, tr2, _ = env2.step(safe_a2)
            if t1 or tr1:
                obs1, _ = env1.reset(seed=seed)
            if t2 or tr2:
                obs2, _ = env2.reset(seed=seed)

            time.sleep(step_delay)
    except (BrokenPipeError, KeyboardInterrupt):
        pass
    finally:
        if game.poll() is None:
            game.terminate()
    print("Beendet.")


def main() -> None:
    parser = argparse.ArgumentParser(description="KI spielt Stoneforge mit echter Grafik")
    parser.add_argument("--model", type=str, default="best_models_ppo/best_model.zip",
                        help="Modell für Einzelmodus oder linke Seite im Dual-Modus")
    parser.add_argument("--model2", type=str, default=None,
                        help="Zweites Modell für Dual-Modus (z.B. dqn_stoneforge_model.zip)")
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--speed", type=float, default=1.0,
                        help="Geschwindigkeitsfaktor (1.0 = normal, 2.0 = doppelt)")
    args = parser.parse_args()

    if not os.path.exists(GAME_BINARY):
        print(f"Fehler: Game-Binary nicht gefunden: {GAME_BINARY}", file=sys.stderr)
        print("Bitte zuerst bauen: cmake --build build --target stoneforge_client", file=sys.stderr)
        sys.exit(1)

    if args.model2:
        run_dual(args.model, args.model2, args.seed, args.speed)
    else:
        run_single(args.model, args.seed, args.speed)


if __name__ == "__main__":
    main()
