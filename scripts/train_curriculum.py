"""Stoneforge RL — Curriculum-Training für RecurrentPPO (LSTM, kein BFS).

Drei Phasen, identisch zu ppo_phase4 — aber mit LSTM statt MLP und ohne BFS in der Obs:
  Phase 1: exit=5–12,   max 500k Steps,  Ziel >40% SR auf Trainingsverteilung
  Phase 2: exit=12–25,  max 500k Steps,  Ziel >30% SR auf Trainingsverteilung
  Phase 3: exit=25–45,  max 1M  Steps,   Ziel >70% SR auf Projektarbeit-Kriterium (35–45)

Verwendung:
    python scripts/train_curriculum.py
    python scripts/train_curriculum.py --save-dir models/ppo_lstm_curriculum
"""
from __future__ import annotations

import argparse
import os

import numpy as np
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.utils import set_random_seed
from sb3_contrib import RecurrentPPO

from stoneforge_env import StoneforgeWorldEnv

# ──────────────────────────────────────────────────────────────────────────────
EVAL_SEEDS = list(range(7000, 7050))
MAX_EVAL_STEPS = 4000

PHASES = [
    {"exit_min":  5, "exit_max": 12, "max_steps":   500_000, "target_sr": 0.40,
     "eval_min":  5, "eval_max": 12,  "label": "Phase 1 (exit=5–12)"},
    {"exit_min": 12, "exit_max": 25, "max_steps":   500_000, "target_sr": 0.30,
     "eval_min": 12, "eval_max": 25,  "label": "Phase 2 (exit=12–25)"},
    {"exit_min": 25, "exit_max": 45, "max_steps": 1_000_000, "target_sr": 0.70,
     "eval_min": 35, "eval_max": 45,  "label": "Phase 3 (exit=25–45, eval 35–45)"},
]

RPPO_KWARGS = dict(
    policy="MlpLstmPolicy",
    n_steps=256,
    batch_size=8,
    n_epochs=10,
    learning_rate=3e-4,
    gamma=0.999,
    gae_lambda=0.95,
    clip_range=0.2,
    ent_coef=0.05,
    vf_coef=0.5,
    policy_kwargs=dict(n_lstm_layers=1, lstm_hidden_size=256, shared_lstm=False),
    verbose=1,
    tensorboard_log="logs/tensorboard/",
)


# ──────────────────────────────────────────────────────────────────────────────
# Callbacks
# ──────────────────────────────────────────────────────────────────────────────

class CurriculumEvalCallback(BaseCallback):
    """Evaluiert alle eval_freq Steps auf der Trainingsverteilung.

    Stoppt das Training wenn target_sr erreicht wird (gibt False zurück).
    Speichert das beste Modell nach SR in best_<save_path>.
    """

    def __init__(self, eval_freq, save_path, target_sr,
                 eval_exit_min, eval_exit_max, phase_label, verbose=1):
        super().__init__(verbose)
        self.eval_freq = eval_freq
        self.save_path = save_path
        # Best-Checkpoint in separatem Pfad speichern, wird nicht überschrieben
        self.best_save_path = save_path.replace("_model", "_best_model")
        self.target_sr = target_sr
        self.eval_exit_min = eval_exit_min
        self.eval_exit_max = eval_exit_max
        self.phase_label = phase_label
        self._best_sr = -1.0
        os.makedirs(os.path.dirname(save_path), exist_ok=True)

    def _on_step(self) -> bool:
        if self.n_calls % self.eval_freq == 0:
            sr = self._run_eval()
            if sr >= self.target_sr:
                print(f"\n  ✓ Ziel-SR {self.target_sr:.0%} erreicht ({sr:.1%}) "
                      f"— {self.phase_label} abgeschlossen.")
                return False  # Training stoppen, nächste Phase starten
        return True

    def _run_eval(self) -> float:
        env = StoneforgeWorldEnv(exit_min=self.eval_exit_min, exit_max=self.eval_exit_max)
        successes = 0

        for seed in EVAL_SEEDS:
            obs, _ = env.reset(seed=seed)
            done, steps = False, 0
            lstm_states = None
            ep_start = np.ones((1,), dtype=bool)

            while not done and steps < MAX_EVAL_STEPS:
                action, lstm_states = self.model.predict(
                    obs.reshape(1, -1), state=lstm_states,
                    episode_start=ep_start, deterministic=True,  # deterministisch für konsistente Messung
                )
                obs, _, term, trunc, info = env.step(int(action[0]))
                ep_start = np.zeros((1,), dtype=bool)
                done = term or trunc; steps += 1
                if info.get("reached_exit"):
                    successes += 1; break

        sr = successes / len(EVAL_SEEDS)

        if self.verbose:
            print(f"  [Eval @ {self.num_timesteps:,}] {self.phase_label}: "
                  f"SR={successes}/{len(EVAL_SEEDS)} ({sr:.1%})", flush=True)

        self.logger.record("eval/success_rate", sr)
        self.logger.record("eval/successes", successes)
        self.logger.dump(self.num_timesteps)

        if sr > self._best_sr:
            self._best_sr = sr
            self.model.save(self.best_save_path)
            if self.verbose:
                print(f"  → Bestes Modell gespeichert ({sr:.1%}): {self.best_save_path}.zip")

        return sr


# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser()
    p.add_argument("--save-dir", default="models/ppo_lstm_curriculum")
    p.add_argument("--n-envs", type=int, default=8)
    p.add_argument("--eval-freq", type=int, default=25_000)
    p.add_argument("--seed", type=int, default=None)
    return p.parse_args()


def main() -> None:
    args = parse_args()
    os.makedirs(args.save_dir, exist_ok=True)

    if args.seed is not None:
        set_random_seed(args.seed)

    print("\n" + "═"*60)
    print("  Curriculum-Training: RecurrentPPO (LSTM), kein BFS")
    print("  Obs: 231 Features (Grid + Kompass + Step-Counter)")
    print("  Phasen: 5-12 → 12-25 → 25-45")
    print("═"*60 + "\n")

    model = None

    for i, phase in enumerate(PHASES, 1):
        print(f"\n{'─'*60}")
        print(f"  Starte {phase['label']}")
        print(f"  Ziel-SR: {phase['target_sr']:.0%}, max {phase['max_steps']:,} Steps")
        print(f"{'─'*60}")

        def make_env(exit_min=phase["exit_min"], exit_max=phase["exit_max"]):
            return StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max)

        env = make_vec_env(make_env, n_envs=args.n_envs, seed=args.seed)

        phase_model_path = os.path.join(args.save_dir, f"phase{i}_model")
        tb_name = f"rppo_curriculum_p{i}_exit{phase['exit_min']}-{phase['exit_max']}"

        callback = CurriculumEvalCallback(
            eval_freq=max(1, args.eval_freq // args.n_envs),
            save_path=phase_model_path,
            target_sr=phase["target_sr"],
            eval_exit_min=phase["eval_min"],
            eval_exit_max=phase["eval_max"],
            phase_label=phase["label"],
            verbose=1,
        )

        if model is None:
            model = RecurrentPPO(env=env, **RPPO_KWARGS)
        else:
            # Bestes Modell aus vorheriger Phase laden (nicht letzter Stand)
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

        # Letzten Trainingsstand sichern (separat vom besten Checkpoint)
        model.save(phase_model_path)
        print(f"  Phase {i} abgeschlossen. Bestes SR (det): {callback._best_sr:.1%}")

    # Finales Modell = bestes Phase-3-Checkpoint (nicht letzter Stand) → als best_model.zip kopieren
    import shutil
    src = os.path.join(args.save_dir, "phase3_best_model.zip")
    dst = os.path.join(args.save_dir, "best_model.zip")
    if os.path.exists(src):
        shutil.copy2(src, dst)
    else:
        # Fallback: letzter Phase-3-Stand
        src_fallback = os.path.join(args.save_dir, "phase3_model.zip")
        if os.path.exists(src_fallback):
            shutil.copy2(src_fallback, dst)

    print(f"\n{'═'*60}")
    print(f"  Curriculum abgeschlossen!")
    print(f"  Bestes Modell: {dst}")
    print(f"  Vergleich: python scripts/eval_comparison.py")
    print(f"{'═'*60}\n")


if __name__ == "__main__":
    main()
