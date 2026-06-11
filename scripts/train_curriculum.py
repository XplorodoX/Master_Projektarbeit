"""Stoneforge RL — Curriculum-Training für RecurrentPPO (LSTM, kein BFS).

Drei Phasen, identisch zu ppo_phase4 — aber mit LSTM statt MLP und ohne BFS in der Obs:
  Phase 1: exit=5–12,   max 500k Steps,  Ziel >85% SR auf Trainingsverteilung
  Phase 2: exit=12–25,  max 500k Steps,  Ziel >70% SR auf Trainingsverteilung
  Phase 3: exit=25–45,  max 1M  Steps,   Ziel >70% SR auf Projektarbeit-Kriterium (35–45)

Verwendung:
    python scripts/train_curriculum.py
    python scripts/train_curriculum.py --save-dir models/ppo_lstm_curriculum
"""
from __future__ import annotations

import argparse
import os
import sys

import numpy as np

from stable_baselines3.common.callbacks import BaseCallback, CallbackList
from stable_baselines3.common.vec_env import DummyVecEnv
from stable_baselines3.common.utils import set_random_seed
from sb3_contrib import RecurrentPPO

_PYTHON_DIR  = os.path.join(os.path.dirname(__file__), "..", "python")
_SCRIPTS_DIR = os.path.dirname(__file__)
for _d in (_PYTHON_DIR, _SCRIPTS_DIR):
    if _d not in sys.path:
        sys.path.insert(0, _d)

from stoneforge_env import StoneforgeWorldEnv, SwarmSeedPool
from stream_wrapper import StreamWrapper

# ──────────────────────────────────────────────────────────────────────────────
VAL_SEEDS = list(range(6000, 6050))
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
    policy_kwargs=dict(n_lstm_layers=1, lstm_hidden_size=512, shared_lstm=False),
    verbose=1,
    tensorboard_log="logs/tensorboard/",
)


# ──────────────────────────────────────────────────────────────────────────────
# Callbacks
# ──────────────────────────────────────────────────────────────────────────────

class CurriculumEvalCallback(BaseCallback):
    """Evaluiert alle eval_freq Steps auf der Trainingsverteilung.

    Stoppt das Training wenn target_sr in 2 aufeinanderfolgenden Evals erreicht wird (gibt False zurück).
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
                    return False  # Training stoppen, nächste Phase starten
            else:
                self.consecutive_successes = 0
        return True

    def _run_eval(self) -> float:
        env = StoneforgeWorldEnv(exit_min=self.eval_exit_min, exit_max=self.eval_exit_max, use_last_action_reward=True)
        successes = 0

        for seed in VAL_SEEDS:
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
                print(f"  → Bestes Modell gespeichert ({sr:.1%}): {self.best_save_path}.zip")
            if hasattr(self, '_meta_cb'):
                self._meta_cb._best_sr = sr

        return sr


# ──────────────────────────────────────────────────────────────────────────────
# Meta-Update-Callback (schickt Phase/Timesteps an ws_map_server)
# ──────────────────────────────────────────────────────────────────────────────

class MetaCallback(BaseCallback):
    """Aktualisiert globale Trainingsmetadaten im ws_map_server (alle 200 Steps)."""

    def __init__(self, phase_label: str, swarm_pool: SwarmSeedPool | None,
                 update_freq: int = 200, verbose: int = 0):
        super().__init__(verbose)
        self.phase_label = phase_label
        self.swarm_pool  = swarm_pool
        self.update_freq = update_freq
        self._best_sr    = 0.0

    def _on_step(self) -> bool:
        if self.n_calls % self.update_freq == 0:
            try:
                import ws_map_server as _ws
                pool_size = self.swarm_pool.stats()["pool_size"] if self.swarm_pool else 0
                _ws.update_meta({
                    "phase":      self.phase_label,
                    "timesteps":  int(self.num_timesteps),
                    "best_sr":    self._best_sr,
                    "swarm_pool": pool_size,
                })
            except Exception:
                pass
        return True


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
    p.add_argument("--save-dir", default="models/ppo_lstm_curriculum")
    p.add_argument("--n-envs", type=int, default=16)
    p.add_argument("--eval-freq", type=int, default=25_000)
    p.add_argument("--seed", type=int, default=None)
    p.add_argument("--start-phase", type=int, default=1, choices=[1, 2, 3],
                   help="Startet ab einer bestimmten Phase (setzt vorherige phase_best_model.zip voraus)")
    p.add_argument("--no-swarm", action="store_true",
                   help="Swarm-Training deaktivieren (default: immer an)")
    p.add_argument("--swarm-prob", type=float, default=0.3,
                   help="Wahrscheinlichkeit, bei Reset einen Swarm-Seed zu verwenden (default: 0.3)")
    p.add_argument("--no-heatmap",   action="store_true",
                   help="Heatmap nach dem Training NICHT generieren")
    p.add_argument("--no-live-map",  action="store_true",
                   help="WS Live Map NICHT starten (default: immer an)")
    p.add_argument("--live-map-port", type=int, default=8766,
                   help="Port für Live Map HTTP+WS (default: 8766)")
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

    # WebSocket Live Map Server starten (wie transdimensional.xyz bei PokéRL)
    live_map_active = False
    if not args.no_live_map:
        if args.subproc:
            print("  ⚠️  Live Map funktioniert in Subprozessen nicht in-process — Live Map wird deaktiviert.")
        else:
            import ws_map_server
            live_map_active = ws_map_server.start_server(port=args.live_map_port)

    print("\n" + "═"*60)
    print("  Curriculum-Training: RecurrentPPO (LSTM), kein BFS")
    print("  Obs: 236 Features (Grid + Kompass + Step-Counter + Last Action/Reward)")
    print("  Phasen: 5-12 → 12-25 → 25-45")
    if swarm_pool:
        if swarm_pool.plr_mode:
            print(f"  Swarm: AN (PLR-Modus, prob={args.swarm_prob:.0%})")
        else:
            print(f"  Swarm: AN (Standard-Erfolgs-Modus, prob={args.swarm_prob:.0%})")
    else:
        print(f"  Swarm: AUS")
    if live_map_active:
        print(f"  Live Map: http://localhost:{args.live_map_port}")
    if args.subproc:
        print("  VecEnv: SubprocVecEnv (Parallel)")
    else:
        print("  VecEnv: DummyVecEnv (Sequenziell)")
    print("═"*60 + "\n")

    model = None

    for i, phase in enumerate(PHASES, 1):
        if i < args.start_phase:
            print(f"  Überspringe {phase['label']} (--start-phase={args.start_phase})")
            continue

        print(f"\n{'─'*60}")
        print(f"  Starte {phase['label']}")
        print(f"  Ziel-SR: {phase['target_sr']:.0%}, max {phase['max_steps']:,} Steps")
        print(f"{'─'*60}")

        # StreamWrapper um jede Env — sendet pro Step Position an WS-Server
        # (analog zu PokéRL's StreamWrapper, aber in-process statt externer WS)
        def make_env_i(agent_id, exit_min=phase["exit_min"],
                       exit_max=phase["exit_max"], _pool=swarm_pool):
            base = StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max, swarm_pool=_pool, use_last_action_reward=True)
            return StreamWrapper(base, agent_id=agent_id)

        env_fns = [
            (lambda aid=j: make_env_i(aid))
            for j in range(args.n_envs)
        ]
        if args.subproc:
            from stable_baselines3.common.vec_env import SubprocVecEnv
            env = SubprocVecEnv(env_fns)
        else:
            env = DummyVecEnv(env_fns)
        if args.seed is not None:
            env.seed(args.seed)

        phase_model_path = os.path.join(args.save_dir, f"phase{i}_model")
        tb_name = f"rppo_curriculum_p{i}_exit{phase['exit_min']}-{phase['exit_max']}"

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
        if live_map_active:
            meta_cb = MetaCallback(
                phase_label=phase["label"],
                swarm_pool=swarm_pool,
                update_freq=200,
            )
            eval_cb._meta_cb = meta_cb   # SR-Weitergabe
            callbacks.append(meta_cb)
        if i == 3:
            callbacks.append(EntropyAnnealingCallback(duration_steps=500_000, start_ent=0.01, end_ent=0.001))
        callback = CallbackList(callbacks)

        if model is None and i == 1:
            model = RecurrentPPO(env=env, **RPPO_KWARGS)
        elif model is None or i > 1:
            # Bestes Modell aus vorheriger Phase laden (auch bei --start-phase > 1)
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
        print(f"  Phase {i} abgeschlossen. Bestes SR (det): {eval_cb._best_sr:.1%}")
        if swarm_pool:
            stats = swarm_pool.stats()
            print(f"  Swarm-Pool: {stats['pool_size']} Seeds, "
                  f"{stats['total_added']} hinzugefügt, "
                  f"{stats['total_sampled']} gesampled")

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

    # Automatische Heatmap nach Abschluss
    if not args.no_heatmap and os.path.exists(dst):
        print("  Generiere Heatmap (deterministisch) ...")
        try:
            import subprocess
            subprocess.run(
                ["python", "scripts/heatmap_eval.py",
                 "--model", dst, "--out-dir", "logs"],
                check=True,
            )
            print("  Heatmap fertig. Gespeichert in logs/\n")
        except Exception as exc:
            print(f"  Heatmap fehlgeschlagen: {exc}\n")


if __name__ == "__main__":
    main()
