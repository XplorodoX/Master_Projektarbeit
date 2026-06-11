"""Stoneforge RL — Ablation D: RecurrentPPO mit CNN + Visited Mask.

Identisches Curriculum wie ppo_lstm_curriculum (Ablation C), aber:
  - Observation: 2-Kanal 15×15 Grid (Tile-Typen + Visited Mask) → 456 dims
  - Feature Extractor: 2D-CNN statt flacher MLP-Layer
  - LSTM bleibt: 256 Hidden Units, 1 Layer

Ablationsmatrix:
  A  ppo_phase4          MLP   + BFS  + Curriculum → 100% det  (Referenz-Obergrenze)
  B  ppo_no_bfs          MLP   - BFS  - Curriculum →   0% det  (Negativ-Ergebnis)
  C  ppo_lstm_curriculum LSTM  - BFS  + Curriculum →  86% stoch / 36% det
  D  ppo_lstm_cnn        LSTM-CNN - BFS + Curriculum + Visited Mask  ← dieser Lauf

Verwendung:
    python scripts/train_cnn.py
    python scripts/train_cnn.py --save-dir models/ppo_lstm_cnn_run2 --seed 1
    python scripts/train_cnn.py --start-phase 2 --save-dir models/ppo_lstm_cnn
"""
from __future__ import annotations

import argparse
import os
import sys

import numpy as np
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.utils import set_random_seed
from sb3_contrib import RecurrentPPO

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "python"))
from stoneforge_env import StoneforgeWorldEnv, SwarmSeedPool
from cnn_extractor import StoneforgeGridCNN

# ──────────────────────────────────────────────────────────────────────────────
VAL_SEEDS     = list(range(6000, 6050))
MAX_EVAL_STEPS = 4000

PHASES = [
    {"exit_min":  5, "exit_max": 12, "max_steps":   500_000, "target_sr": 0.85,
     "eval_min":  5, "eval_max": 12,  "label": "Phase 1 (exit=5–12)"},
    {"exit_min": 12, "exit_max": 25, "max_steps":   500_000, "target_sr": 0.70,
     "eval_min": 12, "eval_max": 25,  "label": "Phase 2 (exit=12–25)"},
    {"exit_min": 25, "exit_max": 45, "max_steps": 1_000_000, "target_sr": 0.70,
     "eval_min": 35, "eval_max": 45,  "label": "Phase 3 (exit=25–45, eval 35–45)"},
]

RPPO_KWARGS = dict(
    policy="MlpLstmPolicy",
    n_steps=512,
    batch_size=256,
    n_epochs=4,
    learning_rate=3e-4,
    gamma=0.999,
    gae_lambda=0.95,
    clip_range=0.2,
    ent_coef=0.01,
    vf_coef=0.5,
    policy_kwargs=dict(
        n_lstm_layers=1,
        lstm_hidden_size=512,
        shared_lstm=False,
        features_extractor_class=StoneforgeGridCNN,
        features_extractor_kwargs=dict(cnn_out_features=128),
    ),
    verbose=1,
    tensorboard_log="logs/tensorboard/",
)


# ──────────────────────────────────────────────────────────────────────────────
# Callback
# ──────────────────────────────────────────────────────────────────────────────

class CurriculumEvalCallback(BaseCallback):
    def __init__(self, eval_freq, save_path, target_sr,
                 eval_exit_min, eval_exit_max, phase_label, verbose=1):
        super().__init__(verbose)
        self.eval_freq      = eval_freq
        self.save_path      = save_path
        self.best_save_path = save_path.replace("_model", "_best_model")
        self.target_sr      = target_sr
        self.eval_exit_min  = eval_exit_min
        self.eval_exit_max  = eval_exit_max
        self.phase_label    = phase_label
        self._best_sr       = -1.0
        self.consecutive_successes = 0
        os.makedirs(os.path.dirname(save_path), exist_ok=True)

    def _on_step(self) -> bool:
        if self.n_calls % self.eval_freq == 0:
            sr = self._run_eval()
            if sr >= self.target_sr:
                self.consecutive_successes += 1
                if self.consecutive_successes >= 2:
                    print(f"\n  ✓ Ziel-SR {self.target_sr:.0%} in 2 aufeinanderfolgenden Evals erreicht ({sr:.1%}) "
                          f"— {self.phase_label} abgeschlossen.")
                    return False
            else:
                self.consecutive_successes = 0
        return True

    def _run_eval(self) -> float:
        # CNN-Variante: use_visited_mask=True
        env = StoneforgeWorldEnv(exit_min=self.eval_exit_min,
                                  exit_max=self.eval_exit_max,
                                  use_visited_mask=True)
        successes = 0
        for seed in VAL_SEEDS:
            obs, _ = env.reset(seed=seed)
            done = False; steps = 0
            lstm_states = None
            ep_start = np.ones((1,), dtype=bool)
            while not done and steps < MAX_EVAL_STEPS:
                action, lstm_states = self.model.predict(
                    obs.reshape(1, -1), state=lstm_states,
                    episode_start=ep_start, deterministic=True,
                )
                obs, _, term, trunc, info = env.step(int(action[0]))
                ep_start = np.zeros((1,), dtype=bool)
                done = term or trunc; steps += 1
                if info.get("reached_exit"):
                    successes += 1; break

        sr = successes / len(VAL_SEEDS)
        if self.verbose:
            print(f"  [Eval @ {self.num_timesteps:,}] {self.phase_label}: "
                  f"SR={successes}/{len(VAL_SEEDS)} ({sr:.1%})", flush=True)

        self.logger.record("eval/success_rate", sr)
        self.logger.record("eval/successes", successes)
        self.logger.dump(self.num_timesteps)

        if sr > self._best_sr:
            self._best_sr = sr
            self.model.save(self.best_save_path)
            if self.verbose:
                print(f"  → Bestes Modell ({sr:.1%}): {self.best_save_path}.zip")
        return sr


# ──────────────────────────────────────────────────────────────────────────────
# Entropy-Annealing-Callback (lässt ent_coef in Phase 3 abklingen)
# ──────────────────────────────────────────────────────────────────────────────

class EntropyAnnealingCallback(BaseCallback):
    """Linear annealing of ent_coef from 0.01 to 0.001 over 500k steps in Phase 3."""

    def __init__(self, duration_steps: int = 500_000, start_ent: float = 0.01, end_ent: float = 0.001, verbose: int = 0):
        super().__init__(verbose)
        self.duration_steps = duration_steps
        self.start_ent = start_ent
        self.end_ent = end_ent
        self.start_timesteps = None

    def _on_step(self) -> bool:
        if self.start_timesteps is None:
            self.start_timesteps = self.num_timesteps
        
        elapsed = self.num_timesteps - self.start_timesteps
        if elapsed <= self.duration_steps:
            fraction = elapsed / self.duration_steps
            ent = self.start_ent + fraction * (self.end_ent - self.start_ent)
        else:
            ent = self.end_ent
        
        self.model.ent_coef = ent
        self.logger.record("train/entropy_coefficient", ent)
        return True


# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser()
    p.add_argument("--save-dir",    default="models/ppo_lstm_cnn")
    p.add_argument("--n-envs",      type=int, default=16)
    p.add_argument("--eval-freq",   type=int, default=25_000)
    p.add_argument("--seed",        type=int, default=None)
    p.add_argument("--start-phase", type=int, default=1, choices=[1, 2, 3])
    p.add_argument("--no-swarm",    action="store_true",
                   help="Swarm-Training deaktivieren (default: immer an)")
    p.add_argument("--swarm-prob",  type=float, default=0.3)
    p.add_argument("--no-heatmap",  action="store_true",
                   help="Heatmap nach dem Training NICHT generieren")
    p.add_argument("--plr", action="store_true",
                   help="Invertiert Swarm-Pool zu PLR-Semantik (auf knapp gescheiterte Seeds fokussieren)")
    p.add_argument("--subproc", action="store_true",
                   help="Verwende SubprocVecEnv statt DummyVecEnv")
    return p.parse_args()


def main() -> None:
    args = parse_args()
    os.makedirs(args.save_dir, exist_ok=True)
    if args.seed is not None:
        set_random_seed(args.seed)

    swarm_pool = SwarmSeedPool(swarm_prob=args.swarm_prob, plr_mode=args.plr) if not args.no_swarm else None

    print("\n" + "═"*60)
    print("  Ablation D: RecurrentPPO — CNN + Visited Mask, kein BFS")
    print("  Obs: 456 dims (2×15×15 Grid + 6 Extras)")
    print("  CNN: Conv(16)→Conv(32)→Pool→Conv(64) → Linear(128)")
    print("  LSTM: 256 Hidden Units, 1 Layer")
    print("  Phasen: 5-12 → 12-25 → 25-45")
    if swarm_pool:
        if swarm_pool.plr_mode:
            print(f"  Swarm: AN (PLR-Modus, prob={args.swarm_prob:.0%})")
        else:
            print(f"  Swarm: AN (Standard-Erfolgs-Modus, prob={args.swarm_prob:.0%})")
    else:
        print(f"  Swarm: AUS  (--no-swarm)")
    if args.subproc:
        print("  VecEnv: SubprocVecEnv (Parallel)")
    else:
        print("  VecEnv: DummyVecEnv (Sequenziell)")
    print("═"*60 + "\n")

    model = None

    for i, phase in enumerate(PHASES, 1):
        if i < args.start_phase:
            print(f"  Überspringe {phase['label']}")
            continue

        print(f"\n{'─'*60}")
        print(f"  Starte {phase['label']}")
        print(f"  Ziel-SR: {phase['target_sr']:.0%}, max {phase['max_steps']:,} Steps")
        print(f"{'─'*60}")

        def make_env(exit_min=phase["exit_min"], exit_max=phase["exit_max"],
                     _pool=swarm_pool):
            return StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max,
                                      swarm_pool=_pool, use_visited_mask=True)

        if args.subproc:
            from stable_baselines3.common.vec_env import SubprocVecEnv
            env = make_vec_env(make_env, n_envs=args.n_envs, seed=args.seed, vec_env_cls=SubprocVecEnv)
        else:
            env = make_vec_env(make_env, n_envs=args.n_envs, seed=args.seed)

        phase_model_path = os.path.join(args.save_dir, f"phase{i}_model")
        tb_name = f"rppo_cnn_p{i}_exit{phase['exit_min']}-{phase['exit_max']}"

        eval_cb = CurriculumEvalCallback(
            eval_freq=max(1, args.eval_freq // args.n_envs),
            save_path=phase_model_path,
            target_sr=phase["target_sr"],
            eval_exit_min=phase["eval_min"],
            eval_exit_max=phase["eval_max"],
            phase_label=phase["label"],
            verbose=1,
        )

        callbacks = [eval_cb]
        if i == 3:
            callbacks.append(EntropyAnnealingCallback(duration_steps=500_000, start_ent=0.01, end_ent=0.001))
        from stable_baselines3.common.callbacks import CallbackList
        callback = CallbackList(callbacks)

        if model is None and i == 1:
            model = RecurrentPPO(env=env, **RPPO_KWARGS)
        elif model is None or i > 1:
            prev_best = os.path.join(args.save_dir, f"phase{i-1}_best_model.zip")
            prev_last = os.path.join(args.save_dir, f"phase{i-1}_model.zip")
            prev_path = prev_best if os.path.exists(prev_best) else prev_last
            print(f"  Lade Phase-{i-1}-Modell: {prev_path}")
            model = RecurrentPPO.load(
                prev_path, env=env,
                **{k: v for k, v in RPPO_KWARGS.items()
                   if k not in ("verbose", "policy", "policy_kwargs")},
            )

        model.learn(
            total_timesteps=phase["max_steps"],
            callback=callback,
            tb_log_name=tb_name,
            reset_num_timesteps=(i == 1),
        )

        model.save(phase_model_path)
        print(f"  Phase {i} fertig. Bestes SR (det): {eval_cb._best_sr:.1%}")
        if swarm_pool:
            s = swarm_pool.stats()
            print(f"  Swarm: pool={s['pool_size']}, "
                  f"added={s['total_added']}, sampled={s['total_sampled']}")

    import shutil
    src = os.path.join(args.save_dir, "phase3_best_model.zip")
    dst = os.path.join(args.save_dir, "best_model.zip")
    if os.path.exists(src):
        shutil.copy2(src, dst)
    elif os.path.exists(os.path.join(args.save_dir, "phase3_model.zip")):
        shutil.copy2(os.path.join(args.save_dir, "phase3_model.zip"), dst)

    print(f"\n{'═'*60}")
    print(f"  Ablation D abgeschlossen!")
    print(f"  Bestes Modell: {dst}")
    print(f"  Vergleich: python scripts/eval_comparison.py")
    print(f"{'═'*60}\n")

    if not args.no_heatmap and os.path.exists(dst):
        import subprocess
        subprocess.run(["python", "scripts/heatmap_eval.py",
                        "--model", dst, "--out-dir", "logs"], check=False)


if __name__ == "__main__":
    main()
