"""Stoneforge RL — Trainings-Einstiegspunkt (PPO / DQN).

Verwendung:
    python python/train.py --algo ppo --timesteps 1000000
    python python/train.py --algo dqn --timesteps 1000000

Sanity-Run-Kriterium (Phase 0):
    PPO sollte nach 1M Steps klar über 0% Success Rate kommen.
    Bei korrektem PBRS-Signal (PBRS_BETA=5.0 in simulation.cpp)
    ist >30% nach 1M Steps ein realistisches Ziel.

Wichtig: Nach Änderung an simulation.cpp erst neu bauen:
    cmake --build build -j && export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
"""
from __future__ import annotations

import argparse
import os
import sys

import numpy as np
from stable_baselines3 import DQN, PPO
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.env_util import make_vec_env

# Stoneforge env muss im PYTHONPATH liegen (build/ + python/).
from stoneforge_env import StoneforgeWorldEnv

# ──────────────────────────────────────────────────────────────────────────────
# Konstanten
# ──────────────────────────────────────────────────────────────────────────────

EVAL_SEEDS_A = list(range(7000, 7050))   # Testset A (50 Seeds, Projektarbeit-Standard)
EVAL_SEEDS_B = list(range(8000, 8050))   # Testset B (Holdout — erst am Ende messen!)
MAX_EVAL_STEPS = 4000


# ──────────────────────────────────────────────────────────────────────────────
# Eval-Callback
# ──────────────────────────────────────────────────────────────────────────────

class SeedEvalCallback(BaseCallback):
    """Evaliert das aktuelle Modell alle eval_freq Steps auf EVAL_SEEDS_A.

    Speichert das beste Modell (nach Success Rate) in best_model_path/.
    Loggt success_rate in TensorBoard.
    """

    def __init__(
        self,
        eval_freq: int = 25_000,
        best_model_path: str = "best_models_ppo",
        n_eval_episodes: int = 50,
        eval_exit_min: int = 5,
        eval_exit_max: int = 12,
        verbose: int = 1,
    ) -> None:
        super().__init__(verbose)
        self.eval_freq = eval_freq
        self.best_model_path = best_model_path
        self.n_eval_episodes = n_eval_episodes
        self.eval_exit_min = eval_exit_min
        self.eval_exit_max = eval_exit_max
        self._best_rate = -1.0
        os.makedirs(best_model_path, exist_ok=True)

    def _on_step(self) -> bool:
        if self.eval_freq > 0 and self.n_calls % self.eval_freq == 0:
            self._run_eval()
        return True

    def _run_eval(self) -> None:
        seeds = EVAL_SEEDS_A[: self.n_eval_episodes]
        env = StoneforgeWorldEnv(exit_min=self.eval_exit_min, exit_max=self.eval_exit_max)
        successes = 0

        for seed in seeds:
            obs, _ = env.reset(seed=seed)
            done = False
            steps = 0
            while not done and steps < MAX_EVAL_STEPS:
                action, _ = self.model.predict(obs, deterministic=True)
                obs, _, term, trunc, info = env.step(int(action))
                done = term or trunc
                steps += 1
                if info.get("reached_exit"):
                    successes += 1
                    break

        rate = successes / len(seeds)

        if self.verbose:
            print(
                f"  [Eval @ {self.num_timesteps:,}] "
                f"success={successes}/{len(seeds)} ({rate:.1%})",
                flush=True,
            )

        # TensorBoard
        self.logger.record("eval/success_rate", rate)
        self.logger.record("eval/successes", successes)
        self.logger.dump(self.num_timesteps)

        if rate > self._best_rate:
            self._best_rate = rate
            save_path = os.path.join(self.best_model_path, "best_model")
            self.model.save(save_path)
            if self.verbose:
                print(f"  → Neues bestes Modell gespeichert ({rate:.1%}): {save_path}.zip")


# ──────────────────────────────────────────────────────────────────────────────
# Hyperparameter
# ──────────────────────────────────────────────────────────────────────────────

# gamma=0.999 ist Pflicht: PBRS_GAMMA in simulation.cpp ist ebenfalls 0.999.
# Bei Mismatch ist PBRS nicht mehr policy-invariant.
# Mit maxSteps=4000 und gamma=0.99 wäre 0.99^4000 ≈ 0 — Exit-Bonus unsichtbar.

PPO_KWARGS = dict(
    policy="MlpPolicy",
    n_steps=2048,
    batch_size=256,
    n_epochs=10,
    learning_rate=3e-4,
    gamma=0.999,
    gae_lambda=0.95,
    clip_range=0.2,
    ent_coef=0.01,        # kleiner Entropie-Bonus für Exploration
    vf_coef=0.5,
    verbose=1,
    tensorboard_log="tensorboard_logs/",
)

DQN_KWARGS = dict(
    policy="MlpPolicy",
    learning_rate=1e-4,
    buffer_size=200_000,
    learning_starts=10_000,
    batch_size=256,
    gamma=0.999,
    exploration_fraction=0.5,
    exploration_final_eps=0.05,
    train_freq=4,
    target_update_interval=1000,
    verbose=1,
    tensorboard_log="tensorboard_logs/",
)


# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Stoneforge RL Training")
    p.add_argument("--algo", choices=["ppo", "dqn"], default="ppo")
    p.add_argument("--timesteps", type=int, default=500_000)
    p.add_argument("--n-envs", type=int, default=8)
    p.add_argument("--eval-freq", type=int, default=25_000)
    p.add_argument("--save-dir", type=str, default=None)
    p.add_argument("--exit-min", type=int, default=5,
                   help="Minimale Exit-Distanz (Curriculum Phase 1=5, Phase 2=15, Phase 3=35)")
    p.add_argument("--exit-max", type=int, default=12,
                   help="Maximale Exit-Distanz")
    p.add_argument("--load-model", type=str, default=None,
                   help="Pfad zu einem bestehenden Modell (.zip) zum Weitertrainieren")
    return p.parse_args()


def main() -> None:
    args = parse_args()
    algo = args.algo.lower()
    save_dir = args.save_dir or f"best_models_{algo}"

    print(f"Starte Training: algo={algo.upper()}, timesteps={args.timesteps:,}, "
          f"n_envs={args.n_envs}, exit={args.exit_min}-{args.exit_max}, save_dir={save_dir}")

    def make_env():
        return StoneforgeWorldEnv(exit_min=args.exit_min, exit_max=args.exit_max)

    env = make_vec_env(make_env, n_envs=args.n_envs)

    # Eval läuft immer auf dem Ziel-Testset (Phase-3-Distanz), damit Fortschritt vergleichbar ist
    eval_cb = SeedEvalCallback(
        eval_freq=max(1, args.eval_freq // args.n_envs),
        best_model_path=save_dir,
        n_eval_episodes=50,
        eval_exit_min=args.exit_min,
        eval_exit_max=args.exit_max,
        verbose=1,
    )

    if algo == "ppo":
        tb_name = f"ppo_exit{args.exit_min}-{args.exit_max}"
        if args.load_model:
            model = PPO.load(args.load_model, env=env, **{k: v for k, v in PPO_KWARGS.items() if k != "verbose"})
            print(f"  Lade Modell: {args.load_model}")
        else:
            model = PPO(env=env, **PPO_KWARGS)
    else:
        tb_name = f"dqn_exit{args.exit_min}-{args.exit_max}"
        if args.load_model:
            model = DQN.load(args.load_model, env=env)
            print(f"  Lade Modell: {args.load_model}")
        else:
            model = DQN(env=env, **DQN_KWARGS)

    model.learn(
        total_timesteps=args.timesteps,
        callback=eval_cb,
        tb_log_name=tb_name,
        reset_num_timesteps=True,
    )

    final_path = os.path.join(save_dir, "final_model")
    model.save(final_path)
    print(f"\nTraining abgeschlossen. Finales Modell: {final_path}.zip")
    print(f"Bestes Modell: {save_dir}/best_model.zip "
          f"(Success Rate: {eval_cb._best_rate:.1%})")


if __name__ == "__main__":
    main()
