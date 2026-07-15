"""Stoneforge RL — Trainings-Einstiegspunkt (PPO / RecurrentPPO / DQN / A2C).

Verwendung:
    python scripts/train.py --algo rppo --timesteps 2000000
    python scripts/train.py --algo ppo  --timesteps 1000000

RecurrentPPO (rppo) gibt dem Agenten ein LSTM-Gedächtnis, damit er Navigation
in partiell beobachtbaren Welten ohne globale BFS-Information lernen kann.
"""
from __future__ import annotations

import argparse
import os
import time

import numpy as np
import torch
from stable_baselines3 import A2C, DQN, PPO
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.utils import set_random_seed
from sb3_contrib import RecurrentPPO

from stoneforge_env import StoneforgeWorldEnv
from doc_logger import append_eval_history, save_run_config, save_run_results, generate_results_md

# ──────────────────────────────────────────────────────────────────────────────
EVAL_SEEDS_A = list(range(7000, 7050))
MAX_EVAL_STEPS = 4000


# ──────────────────────────────────────────────────────────────────────────────
# Eval-Callback
# ──────────────────────────────────────────────────────────────────────────────

class SeedEvalCallback(BaseCallback):
    """Evaliert das Modell alle eval_freq Steps auf EVAL_SEEDS_A (exit=35-45).

    Speichert das beste Modell nach Success Rate. Unterstützt RecurrentPPO (LSTM).
    """

    def __init__(
        self,
        eval_freq: int = 25_000,
        best_model_path: str = "models/rppo",
        n_eval_episodes: int = 50,
        eval_exit_min: int = 35,
        eval_exit_max: int = 45,
        verbose: int = 1,
    ) -> None:
        super().__init__(verbose)
        self.eval_freq = eval_freq
        self.best_model_path = best_model_path
        self.n_eval_episodes = n_eval_episodes
        self.eval_exit_min = eval_exit_min
        self.eval_exit_max = eval_exit_max
        self._best_rate = -1.0
        self._eval_label = f"eval_{eval_exit_min}-{eval_exit_max}"
        os.makedirs(best_model_path, exist_ok=True)

    def _on_step(self) -> bool:
        if self.eval_freq > 0 and self.n_calls % self.eval_freq == 0:
            self._run_eval()
        return True

    def _run_eval(self) -> None:
        seeds = EVAL_SEEDS_A[: self.n_eval_episodes]
        env = StoneforgeWorldEnv(exit_min=self.eval_exit_min, exit_max=self.eval_exit_max)
        is_recurrent = isinstance(self.model, RecurrentPPO)
        successes = 0

        for seed in seeds:
            obs, _ = env.reset(seed=seed)
            done = False
            steps = 0
            lstm_states = None
            episode_start = np.ones((1,), dtype=bool)

            while not done and steps < MAX_EVAL_STEPS:
                if is_recurrent:
                    action, lstm_states = self.model.predict(
                        obs.reshape(1, -1),
                        state=lstm_states,
                        episode_start=episode_start,
                        deterministic=False,
                    )
                    action = int(action[0])
                    episode_start = np.zeros((1,), dtype=bool)
                else:
                    action, _ = self.model.predict(obs, deterministic=False)
                    action = int(action)

                obs, _, term, trunc, info = env.step(action)
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

        self.logger.record("eval/success_rate", rate)
        self.logger.record("eval/successes", successes)
        self.logger.dump(self.num_timesteps)

        # Eval-Eintrag dauerhaft in eval_history.json speichern
        append_eval_history(
            self.best_model_path, self.num_timesteps, rate,
            successes, len(seeds), label=self._eval_label,
        )

        if rate > self._best_rate:
            self._best_rate = rate
            save_path = os.path.join(self.best_model_path, "best_model")
            self.model.save(save_path)
            if self.verbose:
                print(f"  → Neues bestes Modell gespeichert ({rate:.1%}): {save_path}.zip")


# ──────────────────────────────────────────────────────────────────────────────
# Hyperparameter
# ──────────────────────────────────────────────────────────────────────────────

# gamma=0.999: muss mit PBRS_GAMMA in simulation.cpp übereinstimmen.
RPPO_KWARGS = dict(
    policy="MlpLstmPolicy",
    n_steps=256,           # Sequenzlänge pro Env pro Update (kurz = LSTM lernt schneller)
    batch_size=64,         # v11: validiert 06.07.2026 — lernt wie batch=8, aber 2.1x schneller.
                           # Synchron mit train_curriculum.py halten! CPU nutzen (MPS langsamer).
    n_epochs=10,
    learning_rate=3e-4,
    gamma=0.999,
    gae_lambda=0.95,
    clip_range=0.2,
    ent_coef=0.05,         # Erhöht (war 0.01): verhindert frühen Policy-Kollaps
    vf_coef=0.5,
    policy_kwargs=dict(
        n_lstm_layers=1,
        lstm_hidden_size=256,
        shared_lstm=False,
        enable_critic_lstm=True,   # synchron mit train_curriculum.py
    ),
    verbose=1,
    tensorboard_log="logs/tensorboard/",
)

PPO_KWARGS = dict(
    policy="MlpPolicy",
    n_steps=2048,
    batch_size=256,
    n_epochs=10,
    learning_rate=3e-4,
    gamma=0.999,
    gae_lambda=0.95,
    clip_range=0.2,
    ent_coef=0.05,
    vf_coef=0.5,
    verbose=1,
    tensorboard_log="logs/tensorboard/",
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
    tensorboard_log="logs/tensorboard/",
)

A2C_KWARGS = dict(
    policy="MlpPolicy",
    n_steps=2048,
    learning_rate=7e-4,
    gamma=0.999,
    gae_lambda=1.0,
    ent_coef=0.05,
    vf_coef=0.5,
    verbose=1,
    tensorboard_log="logs/tensorboard/",
)


# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def auto_device() -> str:
    if torch.cuda.is_available():
        return "cuda"
    if torch.backends.mps.is_available():
        return "mps"
    return "cpu"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Stoneforge RL Training")
    p.add_argument("--algo", choices=["rppo", "ppo", "dqn", "a2c"], default="rppo")
    p.add_argument("--timesteps", type=int, default=2_000_000)
    p.add_argument("--n-envs", type=int, default=8)
    p.add_argument("--eval-freq", type=int, default=25_000)
    p.add_argument("--save-dir", type=str, default=None)
    p.add_argument("--exit-min", type=int, default=5)
    p.add_argument("--exit-max", type=int, default=12)
    p.add_argument("--load-model", type=str, default=None)
    p.add_argument("--seed", type=int, default=None)
    p.add_argument("--device", type=str, default="auto",
                   help="Trainingsgerät: auto | cpu | cuda | mps")
    return p.parse_args()


def main() -> None:
    args = parse_args()
    algo = args.algo.lower()
    save_dir = args.save_dir or f"models/{algo}"

    if args.seed is not None:
        set_random_seed(args.seed)

    device = auto_device() if args.device == "auto" else args.device
    print(f"Starte Training: algo={algo.upper()}, timesteps={args.timesteps:,}, "
          f"n_envs={args.n_envs}, exit={args.exit_min}-{args.exit_max}, "
          f"seed={args.seed}, save_dir={save_dir}, device={device}")

    # Konfiguration dauerhaft speichern
    from datetime import datetime
    _kwargs_map = {"rppo": RPPO_KWARGS, "ppo": PPO_KWARGS, "dqn": DQN_KWARGS, "a2c": A2C_KWARGS}
    save_run_config(save_dir, {
        "algo":        algo,
        "timesteps":   args.timesteps,
        "n_envs":      args.n_envs,
        "exit_range":  f"{args.exit_min}-{args.exit_max}",
        "seed":        args.seed,
        "device":      device,
        "load_model":  args.load_model,
        "date":        datetime.now().strftime("%d.%m.%Y"),
        "hyperparams": {k: v for k, v in _kwargs_map.get(algo, {}).items()
                        if k not in ("verbose", "tensorboard_log")},
    })
    _train_start = time.time()

    def make_env():
        return StoneforgeWorldEnv(exit_min=args.exit_min, exit_max=args.exit_max)

    env = make_vec_env(make_env, n_envs=args.n_envs, seed=args.seed)

    eval_cb = SeedEvalCallback(
        eval_freq=max(1, args.eval_freq // args.n_envs),
        best_model_path=save_dir,
        n_eval_episodes=50,
        eval_exit_min=35,
        eval_exit_max=45,
        verbose=1,
    )

    if algo == "rppo":
        tb_name = f"rppo_exit{args.exit_min}-{args.exit_max}"
        if args.load_model:
            model = RecurrentPPO.load(args.load_model, env=env, device=device,
                                      **{k: v for k, v in RPPO_KWARGS.items()
                                         if k not in ("verbose", "policy", "policy_kwargs")})
            print(f"  Lade Modell: {args.load_model}")
        else:
            model = RecurrentPPO(env=env, device=device, **RPPO_KWARGS)

    elif algo == "ppo":
        tb_name = f"ppo_exit{args.exit_min}-{args.exit_max}"
        if args.load_model:
            model = PPO.load(args.load_model, env=env, device=device,
                             **{k: v for k, v in PPO_KWARGS.items() if k != "verbose"})
            print(f"  Lade Modell: {args.load_model}")
        else:
            model = PPO(env=env, device=device, **PPO_KWARGS)

    elif algo == "a2c":
        tb_name = f"a2c_exit{args.exit_min}-{args.exit_max}"
        if args.load_model:
            model = A2C.load(args.load_model, env=env, device=device,
                             **{k: v for k, v in A2C_KWARGS.items() if k != "verbose"})
        else:
            model = A2C(env=env, device=device, **A2C_KWARGS)

    else:  # dqn
        tb_name = f"dqn_exit{args.exit_min}-{args.exit_max}"
        if args.load_model:
            model = DQN.load(args.load_model, env=env, device=device)
        else:
            model = DQN(env=env, device=device, **DQN_KWARGS)

    model.learn(
        total_timesteps=args.timesteps,
        callback=eval_cb,
        tb_log_name=tb_name,
        reset_num_timesteps=True,
    )

    final_path = os.path.join(save_dir, "final_model")
    model.save(final_path)
    _duration_s = int(time.time() - _train_start)
    _h, _r = divmod(_duration_s, 3600)
    _m, _s = divmod(_r, 60)
    _duration_str = f"{_h}h {_m}m {_s}s"

    print(f"\nTraining abgeschlossen. Finales Modell: {final_path}.zip")
    print(f"Bestes Modell: {save_dir}/best_model.zip "
          f"(Success Rate: {eval_cb._best_rate:.1%})")
    print(f"Dauer: {_duration_str}")

    # Ergebnisse dauerhaft speichern
    save_run_results(save_dir, {
        "algo":              algo,
        "best_sr_testset_a": round(eval_cb._best_rate, 4),
        "training_duration": _duration_str,
        "training_steps":    args.timesteps,
        "device":            device,
        "exit_range":        f"{args.exit_min}-{args.exit_max}",
    })
    generate_results_md()


if __name__ == "__main__":
    main()
