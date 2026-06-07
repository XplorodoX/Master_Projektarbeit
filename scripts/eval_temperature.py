#!/usr/bin/env python3
"""Stoneforge RL — Temperatur-Sampling-Benchmark.

Vergleicht für dasselbe PPO-Modell auf derselben Seed-Liste:
  - deterministisch (Argmax)
  - stochastisch (model.predict(..., deterministic=False))
  - Temperatur-Sampling für mehrere Tau-Werte

Damit lässt sich schnell prüfen, ob leichtes Sampling die Loops im offenen
Standard-Setup reduziert, ohne auf rein zufälliges Verhalten zurückzufallen.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import numpy as np
import torch
from stable_baselines3 import PPO

PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "build"))
sys.path.insert(0, str(PROJECT_ROOT / "python"))

from stoneforge_env import StoneforgeWorldEnv


def _parse_int_list(text: str) -> list[int]:
    values = []
    for part in text.split(","):
        part = part.strip()
        if not part:
            continue
        values.append(int(part))
    return values


def _sample_temperature_action(model: PPO, obs: np.ndarray, temperature: float) -> int:
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


def _run_seeds(
    model: PPO,
    env: StoneforgeWorldEnv,
    seeds: list[int],
    mode: str,
    temperature: float | None = None,
) -> dict:
    successes, lengths, returns = 0, [], []

    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        done = False
        steps = 0
        ep_ret = 0.0
        reached = False

        while not done and steps < 4000:
            if mode == "deterministic":
                action, _ = model.predict(obs, deterministic=True)
                action = int(action)
            elif mode == "stochastic":
                action, _ = model.predict(obs, deterministic=False)
                action = int(action)
            elif mode == "temperature":
                if temperature is None:
                    raise ValueError("temperature mode requires a temperature value")
                action = _sample_temperature_action(model, obs, temperature)
            else:
                raise ValueError(f"unknown mode: {mode}")

            obs, reward, term, trunc, info = env.step(action)
            ep_ret += float(reward)
            steps += 1
            if info.get("reached_exit", False):
                reached = True
            done = term or trunc

        successes += int(reached)
        lengths.append(steps)
        returns.append(ep_ret)

    n = len(seeds)
    return {
        "success": successes,
        "n": n,
        "rate": successes / n,
        "mean_len": float(np.mean(lengths)) if lengths else 0.0,
        "mean_return": float(np.mean(returns)) if returns else 0.0,
    }


def _print_row(label: str, mode: str, r: dict) -> None:
    print(
        f"| {label} | {mode} | {r['success']} / {r['n']} | {r['rate']:.1%} | "
        f"{r['mean_len']:.1f} | {r['mean_return']:.2f} |"
    )


def main() -> None:
    parser = argparse.ArgumentParser(description="Stoneforge temperature sampling benchmark")
    parser.add_argument(
        "--model",
        default="models/ppo_delta_v1/best_model.zip",
        help="Pfad zum Modell relativ zum Projektroot",
    )
    parser.add_argument("--exit-min", type=int, default=35)
    parser.add_argument("--exit-max", type=int, default=45)
    parser.add_argument(
        "--seeds",
        default=",".join(str(s) for s in range(7000, 7050)),
        help="Kommagetrennte Seed-Liste",
    )
    parser.add_argument(
        "--temperatures",
        default="0.1,0.2,0.3",
        help="Kommagetrennte Temperatur-Liste für Sampling",
    )
    args = parser.parse_args()

    model_path = PROJECT_ROOT / args.model
    if not model_path.exists():
        print(f"FEHLER: Modell nicht gefunden: {model_path}", file=sys.stderr)
        sys.exit(1)

    seeds = _parse_int_list(args.seeds)
    temperatures = [float(t) for t in args.temperatures.split(",") if t.strip()]

    print("=== Stoneforge Temperature Benchmark ===")
    print(f"Modell        : {model_path.relative_to(PROJECT_ROOT)}")
    print(f"Eval-Welt     : exit={args.exit_min}-{args.exit_max}")
    print(f"Seeds         : {len(seeds)} ({seeds[0]}-{seeds[-1]})")

    model = PPO.load(str(model_path))
    env = StoneforgeWorldEnv(exit_min=args.exit_min, exit_max=args.exit_max)

    print("\n| Modus | Ergebnis | Success | Rate | Mean Len | Mean Return |")
    print("| --- | --- | ---: | ---: | ---: | ---: |")

    det = _run_seeds(model, env, seeds, mode="deterministic")
    _print_row("baseline", "deterministic", det)

    stoch = _run_seeds(model, env, seeds, mode="stochastic")
    _print_row("baseline", "stochastic", stoch)

    for temperature in temperatures:
        result = _run_seeds(
            model,
            env,
            seeds,
            mode="temperature",
            temperature=temperature,
        )
        _print_row("temperature", f"tau={temperature:g}", result)


if __name__ == "__main__":
    main()