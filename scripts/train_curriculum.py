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
import time

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
from doc_logger import append_eval_history, save_run_config, save_run_results, generate_results_md

# ──────────────────────────────────────────────────────────────────────────────
VAL_SEEDS = list(range(6000, 6050))
MAX_EVAL_STEPS = 4000

# Eval-Historie für die Live Map — modulglobal, damit sie Phasenwechsel überlebt
_LIVE_EVALS: list = []

# gate_metric: Metrik für Phasen-Stopp + best_model-Auswahl.
#   Phase 1/2: "stoch" — mit ent_coef=0.05 ist die Policy absichtlich stochastisch,
#   deterministische SR ist dort ~0% (Argmax einer fast-uniformen Verteilung) und
#   trägt kein Signal. Phase 3/4: "det" — Zielmetrik nach Entropie-Annealing.
# eval_max_steps: Episoden-Cap pro Eval. Einheitlich 4000 (= Env-maxSteps, = finaler
#   50-Seed-Eval). Die am 06.07. eingeführten Kurz-Caps (P1=600/P2=1200) haben die SR
#   massiv unterschätzt (Seed-0 phase1_best: 46% @600 vs. 86% @4000 — Steps-bis-Erfolg
#   p95≈1900, max≈3240) und Gate/Modellselektion verzerrt; gemessene Eval-Dauer bei
#   Cap 4000 ist nur ~21 s (det+stoch, 50 Seeds) → Cap-Ersparnis war unnötig.
PHASES = [
    {"exit_min":  5, "exit_max": 12, "max_steps":   500_000, "target_sr": 0.85,
     "eval_min":  5, "eval_max": 12,  "label": "Phase 1 (exit=5–12)",
     "gate_metric": "stoch", "eval_max_steps": 4000},
    {"exit_min": 12, "exit_max": 25, "max_steps":   500_000, "target_sr": 0.70,
     "eval_min": 12, "eval_max": 25,  "label": "Phase 2 (exit=12–25)",
     "gate_metric": "stoch", "eval_max_steps": 4000},
    {"exit_min": 25, "exit_max": 45, "max_steps": 1_000_000, "target_sr": 0.70,
     "eval_min": 35, "eval_max": 45,  "label": "Phase 3 (exit=25–45, eval 35–45)",
     "gate_metric": "det", "eval_max_steps": 4000},
    # Fix 2: Greedy Fine-Tune — ent_coef→0.0001, Modellselektion auf deterministischer SR
    {"exit_min": 25, "exit_max": 45, "max_steps":   200_000, "target_sr": 0.60,
     "eval_min": 35, "eval_max": 45,  "label": "Phase 4 (Greedy Fine-Tune)",
     "gate_metric": "det", "eval_max_steps": 4000,
     "ent_coef_override": 0.0001},
]

RPPO_KWARGS = dict(
    policy="MlpLstmPolicy",
    n_steps=256,          # v10: exakt wie v2 (das funktionierende Modell)
    batch_size=8,         # ZURÜCK auf 8 (07.07.2026): batch=64 destabilisiert den Critic
                          # (EV≈0.1, SR oszilliert chaotisch; 4 Läufe + A/B, CHANGELOG v2026-07-07.4).
                          # Die 64er-"Validierung" vom 06.07. war ein Einzel-Snapshot. CPU!
    n_epochs=10,          # v10: wie v2 (default)
    learning_rate=3e-4,
    gamma=0.999,
    gae_lambda=0.95,
    clip_range=0.2,
    ent_coef=0.05,        # v10: wie v2 — 5× höher für Exploration (war 0.01)
    vf_coef=0.5,
    policy_kwargs=dict(n_lstm_layers=1, lstm_hidden_size=256, shared_lstm=False,
                       enable_critic_lstm=True),
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
                 eval_exit_min, eval_exit_max, phase_label, verbose=1,
                 gate_metric="det", eval_max_steps=MAX_EVAL_STEPS):
        super().__init__(verbose)
        self.eval_freq = eval_freq
        self.save_path = save_path
        # Best-Checkpoint in separatem Pfad speichern, wird nicht überschrieben
        self.best_save_path = save_path.replace("_model", "_best_model")
        self.target_sr = target_sr
        self.eval_exit_min = eval_exit_min
        self.eval_exit_max = eval_exit_max
        self.phase_label = phase_label
        self.gate_metric = gate_metric          # "stoch" (Phase 1/2) oder "det" (Phase 3/4)
        self.eval_max_steps = eval_max_steps
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

    def _eval_once(self, env, deterministic: bool) -> int:
        """Ein Eval-Durchlauf über alle VAL_SEEDS, gibt Anzahl Erfolge zurück."""
        successes = 0
        for seed in VAL_SEEDS:
            obs, _ = env.reset(seed=seed)
            done, steps = False, 0
            lstm_states = None
            ep_start = np.ones((1,), dtype=bool)

            while not done and steps < self.eval_max_steps:
                action, lstm_states = self.model.predict(
                    obs.reshape(1, -1), state=lstm_states,
                    episode_start=ep_start, deterministic=deterministic,
                )
                obs, _, term, trunc, info = env.step(int(action[0]))
                ep_start = np.zeros((1,), dtype=bool)
                done = term or trunc; steps += 1
                if info.get("reached_exit"):
                    successes += 1; break
        return successes

    def _run_eval(self) -> float:
        env = StoneforgeWorldEnv(
            exit_min=self.eval_exit_min, exit_max=self.eval_exit_max,
            # v11: 229-dim (Energie/Inventar entfernt — tote Features)
        )
        # Immer BEIDE Metriken messen (Det/Stoch-Gap-Kurve fürs Paper);
        # Gating + best_model-Auswahl laufen auf gate_metric der Phase.
        succ_det = self._eval_once(env, deterministic=True)
        succ_stoch = self._eval_once(env, deterministic=False)
        n = len(VAL_SEEDS)
        sr_det, sr_stoch = succ_det / n, succ_stoch / n
        sr = sr_stoch if self.gate_metric == "stoch" else sr_det

        if self.verbose:
            print(f"  [Eval @ {self.num_timesteps:,}] {self.phase_label}: "
                  f"det={succ_det}/{n} ({sr_det:.1%}), stoch={succ_stoch}/{n} ({sr_stoch:.1%}) "
                  f"— Gate: {self.gate_metric}", flush=True)

        self.logger.record("eval/sr_det", sr_det)
        self.logger.record("eval/sr_stoch", sr_stoch)
        self.logger.record("eval/success_rate", sr)   # Gating-Metrik (rückwärtskompatibel)
        self.logger.dump(self.num_timesteps)

        # Live Map: vollständige det/stoch-Historie (ws_map.html zeichnet daraus
        # beide SR-Kurven + Phasen-Marker; Fallback ohne "evals" ist best_sr)
        try:
            import ws_map_server as _ws
            _LIVE_EVALS.append({"ts": int(self.num_timesteps), "det": sr_det,
                                "stoch": sr_stoch, "phase": self.phase_label})
            _ws.update_meta({"evals": _LIVE_EVALS})
        except Exception:
            pass

        if sr > self._best_sr:
            self._best_sr = sr
            self.model.save(self.best_save_path)
            if self.verbose:
                print(f"  → Bestes Modell gespeichert ({sr:.1%} {self.gate_metric}): {self.best_save_path}.zip")
            if hasattr(self, '_meta_cb'):
                self._meta_cb._best_sr = sr

        # Eval-Eintrag dauerhaft speichern (beide Metriken im Label sichtbar)
        save_dir = os.path.dirname(self.save_path)
        append_eval_history(
            save_dir, self.num_timesteps, sr,
            succ_stoch if self.gate_metric == "stoch" else succ_det,
            n, label=f"{self.phase_label} [det={sr_det:.0%}|stoch={sr_stoch:.0%}]",
        )

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
    print("  Obs: 229 Features (Grid 225 + HP + Kompass 2 + StepFrac) — v11: ohne tote Features")
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

    from datetime import datetime
    save_run_config(args.save_dir, {
        "algo":       "rppo",
        "script":     "train_curriculum.py",
        "n_envs":     args.n_envs,
        "eval_freq":  args.eval_freq,
        "seed":       args.seed,
        "swarm":      not args.no_swarm,
        "swarm_prob": args.swarm_prob,
        "plr":        args.plr,
        "start_phase":args.start_phase,
        "date":       datetime.now().strftime("%d.%m.%Y"),
        "phases":     [{
            "label":      p["label"],
            "exit_min":   p["exit_min"],
            "exit_max":   p["exit_max"],
            "max_steps":  p["max_steps"],
            "target_sr":  p["target_sr"],
        } for p in PHASES],
        "hyperparams": {k: v for k, v in RPPO_KWARGS.items()
                        if k not in ("verbose", "tensorboard_log")},
    })
    _train_start = time.time()
    _phase_results: list[dict] = []
    model = None

    for i, phase in enumerate(PHASES, 1):
        if i < args.start_phase:
            print(f"  Überspringe {phase['label']} (--start-phase={args.start_phase})")
            continue

        print(f"\n{'─'*60}")
        print(f"  Starte {phase['label']}")
        print(f"  Ziel-SR: {phase['target_sr']:.0%}, max {phase['max_steps']:,} Steps")
        print(f"{'─'*60}")

        # Pool bei Phasenstart leeren: ein "gelöster"/"gescheiterter" Seed aus der
        # vorherigen Phase ist mit neuer Exit-Distanz eine andere Aufgabe —
        # die Pool-Aussage gilt über Phasengrenzen hinweg nicht.
        if swarm_pool is not None and swarm_pool.stats()["pool_size"] > 0:
            swarm_pool.clear()
            print("  Swarm-Pool geleert (Phasenwechsel)")

        # StreamWrapper um jede Env — sendet pro Step Position an WS-Server
        # (analog zu PokéRL's StreamWrapper, aber in-process statt externer WS)
        def make_env_i(agent_id, exit_min=phase["exit_min"],
                       exit_max=phase["exit_max"], _pool=swarm_pool):
            base = StoneforgeWorldEnv(
                exit_min=exit_min, exit_max=exit_max, swarm_pool=_pool,
                # v11: 229-dim obs (Energie/Inventar entfernt — tote Features)
            )
            # StreamWrapper nur bei aktiver Live Map — er lief bisher auch bei
            # --no-live-map mit (voller Stream-Codepfad ohne Server) und steht
            # unter Verdacht, das Training zu stören (Bisect 07.07.2026).
            if not live_map_active:
                return base
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
            gate_metric=phase.get("gate_metric", "det"),
            eval_max_steps=phase.get("eval_max_steps", MAX_EVAL_STEPS),
        )

        callbacks = [eval_cb]
        if live_map_active:
            meta_cb = MetaCallback(
                phase_label=phase["label"],
                swarm_pool=swarm_pool,
                update_freq=200,
            )
            eval_cb._meta_cb = meta_cb
            callbacks.append(meta_cb)
        if i == 3:
            # Entropie in Phase 3 linear 0.05 → 0.001.
            # Start MUSS dem Trainings-ent_coef aus Phase 1/2 entsprechen (RPPO_KWARGS:
            # 0.05), sonst springt die Entropie beim Phasenwechsel abrupt 0.05 → 0.01.
            callbacks.append(EntropyAnnealingCallback(duration_steps=500_000, start_ent=RPPO_KWARGS["ent_coef"], end_ent=0.001))
        if i == 4:
            # Fix 2: Phase 4 Greedy Fine-Tune — Entropie sofort auf 0.0001 setzen
            ent_override = phase.get("ent_coef_override", 0.0001)
            if model is not None:
                model.ent_coef = ent_override
            print(f"  ent_coef → {ent_override} (Greedy Fine-Tune)")
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
        _phase_dur = int(time.time() - _train_start)
        print(f"  Phase {i} abgeschlossen. Bestes SR ({eval_cb.gate_metric}): {eval_cb._best_sr:.1%}")
        swarm_stats: dict = {}
        if swarm_pool:
            swarm_stats = swarm_pool.stats()
            print(f"  Swarm-Pool: {swarm_stats['pool_size']} Seeds, "
                  f"{swarm_stats['total_added']} hinzugefügt, "
                  f"{swarm_stats['total_sampled']} gesampled")

        _phase_result = {
            "phase":       i,
            "label":       phase["label"],
            "best_sr":     round(eval_cb._best_sr, 4),
            "target_sr":   phase["target_sr"],
            "exit_range":  f"{phase['exit_min']}-{phase['exit_max']}",
            "swarm_pool":  swarm_stats,
        }
        _phase_results.append(_phase_result)
        # Phasenergebnis sofort sichern (crash-sicher)
        save_run_results(args.save_dir, {f"phase{i}": _phase_result})

    # Finales Modell: Phase 4 > Phase 3 > Fallback letzter Stand
    import shutil
    dst = os.path.join(args.save_dir, "best_model.zip")
    for candidate in ["phase4_best_model.zip", "phase3_best_model.zip",
                      "phase4_model.zip", "phase3_model.zip"]:
        src = os.path.join(args.save_dir, candidate)
        if os.path.exists(src):
            shutil.copy2(src, dst)
            print(f"  Finales Modell aus: {candidate}")
            break

    _total_dur_s = int(time.time() - _train_start)
    _h, _r = divmod(_total_dur_s, 3600)
    _m, _s = divmod(_r, 60)
    _dur_str = f"{_h}h {_m}m {_s}s"
    best_phase_sr = max((p["best_sr"] for p in _phase_results), default=0.0)

    print(f"\n{'═'*60}")
    print(f"  Curriculum abgeschlossen! Dauer: {_dur_str}")
    print(f"  Bestes Modell: {dst}")
    print(f"  Vergleich: python scripts/eval_comparison.py")
    print(f"{'═'*60}\n")

    # Gesamt-Ergebnis dauerhaft speichern
    save_run_results(args.save_dir, {
        "algo":               "rppo",
        "training_duration":  _dur_str,
        "phases":             _phase_results,
        "best_sr_testset_a":  best_phase_sr,
        "exit_range":         "5-45 (curriculum)",
        "final_model":        dst,
    })
    generate_results_md()

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
