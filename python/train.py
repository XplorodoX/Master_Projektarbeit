from __future__ import annotations

import argparse
import os
import subprocess
import sys
from collections import deque

import gymnasium as gym
import numpy as np
from stable_baselines3 import DQN, PPO
from stable_baselines3.common.callbacks import BaseCallback, CallbackList, EvalCallback
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.monitor import Monitor
from sb3_contrib import RecurrentPPO

from stoneforge_env import ExitPotentialFieldWrapper, StoneforgeConfig, StoneforgeWorldEnv, SymmetryAugmentationWrapper, OneHotGridWrapper
from rnd_wrapper import RNDWrapper

_PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def _rebuild_sim() -> bool:
    """Baut stoneforge_sim (Python-Binding) neu bevor Training startet.
    Stellt sicher dass Python-Env und C++ Client immer denselben Stand haben.
    Gibt True zurueck wenn Build erfolgreich.
    """
    build_dir = os.path.join(_PROJECT_ROOT, "build")
    if not os.path.isdir(build_dir):
        print("[Build] Kein build/-Ordner gefunden — Build uebersprungen.", file=sys.stderr)
        return False

    print("[Build] Baue stoneforge_sim und stoneforge_client neu...")
    result = subprocess.run(
        ["cmake", "--build", build_dir, "-j", "--target", "stoneforge_sim", "stoneforge_client"],
        cwd=_PROJECT_ROOT,
    )
    if result.returncode != 0:
        print("[Build] FEHLER: Build fehlgeschlagen! Training wird trotzdem gestartet.", file=sys.stderr)
        return False
    print("[Build] Build erfolgreich.")
    return True


class ReducedActionEnv(gym.ActionWrapper):
    """Restrict training to movement-only actions."""

    _ACTION_MAP = [0, 1, 2, 3]

    def __init__(self, env: gym.Env):
        super().__init__(env)
        self.action_space = gym.spaces.Discrete(len(self._ACTION_MAP))

    def action(self, action: int) -> int:
        return self._ACTION_MAP[int(action)]


def make_env(disable_mobs: bool = True, eval_mode: bool = False, use_rnd: bool = False) -> gym.Env:
    if eval_mode:
        # Eval immer bei voller Schwierigkeit (35-45 Tiles) — misst echten Fortschritt.
        cfg = StoneforgeConfig(disable_mobs=disable_mobs)
        base = ReducedActionEnv(ExitPotentialFieldWrapper(StoneforgeWorldEnv(cfg)))
        return SymmetryAugmentationWrapper(base, augment=False)
    else:
        # Training startet bei Stage-0-Distanz; CurriculumCallback schaltet weiter.
        cfg = StoneforgeConfig(
            disable_mobs=disable_mobs,
            exit_min_distance=_CURRICULUM_STAGES[0][0],
            exit_max_distance=_CURRICULUM_STAGES[0][1],
        )
        base = ReducedActionEnv(ExitPotentialFieldWrapper(StoneforgeWorldEnv(cfg)))
        if use_rnd:
            base = RNDWrapper(base, intrinsic_scale=0.5, update_proportion=0.25)
        sym = SymmetryAugmentationWrapper(base, augment=True)
        # One-hot encode grid for better spatial representation before feeding the policy
        return OneHotGridWrapper(sym)


# ---------------------------------------------------------------------------
# Curriculum Learning — leistungsbasiert
# ---------------------------------------------------------------------------
# Problem mit zeitbasiertem Curriculum:
#   Der Agent wird auf die nächste Stufe gezwungen egal ob er bereit ist.
#   → Reward kollabiert, Replay Buffer füllt sich mit Misserfolgen.
#
# Fix: Stufe wechselt erst wenn ∅ Reward der letzten N Episoden den
#      Schwellwert erreicht. Als Sicherheitsnetz gibt es trotzdem eine
#      maximale Zeitgrenze (forced_fraction) damit das Training nicht ewig
#      auf einer Stufe steckt.

_CURRICULUM_STAGES = [
    # exit_min, exit_max, reward_threshold, forced_fraction
    # reward_threshold: ∅ Reward über letzte 50 Episoden muss diesen Wert
    #                   überschreiten bevor zur nächsten Stufe gewechselt wird.
    # forced_fraction:  Advance ZU DIESER Stage spätestens bei diesem Anteil.
    #
    # v1.3 Curriculum-Redesign (3 Stages statt 4):
    # ─ Training startet bei Stage-0-Distanz (5-15 Tiles) statt Default (35-45)
    # ─ Stage 3 (volle Schwierigkeit) wird bei 40% erzwungen → 60% des Trainings!
    # ─ BFS-Thresholds: mit +0.10/Schritt braucht man ~70% Erfolg für Threshold 50
    #
    # Zeitplan bei 1M Steps:
    #   0 – 20%: 5–15 Tiles   (200K Steps — Agent sieht Exit direkt, lernt Zielkennung)
    #  20 – 40%: 15–30 Tiles  (200K Steps — Transition zu indirekter Navigation)
    #  40 –100%: 35–45 Tiles  (600K Steps — volle Schwierigkeit, 60% des Trainings!)
    # forced_fraction bedeutet: "Advance ZU DIESER Stage spätestens bei X%".
    # Stage 3 (35-45) muss forced=0.40 haben damit der Wechsel zu voller Schwierigkeit
    # bei 40% passiert und 60% des Trainings dort stattfinden.
    ( 5, 15,  50.0, 0.20),   # Stage 1: advance TO 5-15  by 20%
    (15, 30,  15.0, 0.40),   # Stage 2: advance TO 15-30 by 40%
    (35, 45,  None, 0.40),   # Stage 3: advance TO 35-45 by 40% → 60% Training hier!
]

_REWARD_WINDOW = 50   # Anzahl Episoden für den gleitenden Mittelwert


class CurriculumCallback(BaseCallback):
    """Leistungsbasiertes Curriculum: Stufe wechselt nur wenn der Agent bereit ist."""

    def __init__(self, total_timesteps: int, verbose: int = 1) -> None:
        super().__init__(verbose)
        self._total_ts = total_timesteps
        self._stage = -1                              # aktuell aktive Stufe (Index)
        # Track success-rate (reached_exit) instead of raw reward to avoid
        # being fooled by intrinsic bonuses (e.g. RND).
        self._success_buf: deque[int] = deque(maxlen=_REWARD_WINDOW)

    def _on_step(self) -> bool:
        # Episode-Rewards aus dem Info-Dict sammeln (Monitor schreibt sie rein).
        for info in self.locals.get("infos", []):
            if "episode" in info:
                # Prefer explicit success flag set by the env (info['reached_exit']).
                succ = 1 if info.get("reached_exit", False) else 0
                self._success_buf.append(succ)

        fraction = self.num_timesteps / self._total_ts
        next_stage = self._stage + 1

        if next_stage >= len(_CURRICULUM_STAGES):
            return True   # letzte Stufe bereits aktiv

        emin, emax, threshold, forced = _CURRICULUM_STAGES[next_stage]
        # recent_mean now expresses success rate in percent over the window
        recent_mean = (float(np.mean(self._success_buf)) * 100.0) if len(self._success_buf) >= 20 else -9999.0

        # Weiterschalten wenn Leistung gut genug ODER Zeitlimit erreicht.
        ready = (threshold is not None and recent_mean >= threshold
                 and len(self._reward_buf) >= 20)
        forced_now = fraction >= forced

        if ready or forced_now:
            self._stage = next_stage
            self._reward_buf.clear()   # frisches Fenster für neue Stufe
            self.training_env.env_method(
                "set_curriculum_stage",
                exit_min_distance=emin,
                exit_max_distance=emax,
                force_guaranteed_path=True,
            )
            if self.verbose:
                reason = "Leistung OK" if ready else "Zeitlimit"
                print(
                    f"\n[Curriculum] Stufe {next_stage + 1}/{len(_CURRICULUM_STAGES)}: "
                    f"Exit {emin}–{emax} Tiles  "
                    f"({fraction:.0%} | Grund: {reason} | avgReward={recent_mean:.1f})"
                )

        return True


# ---------------------------------------------------------------------------
# Haupttraining
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(description="Trainiere RL Agenten für Stoneforge")
    parser.add_argument("--algo", type=str, default="dqn", choices=["ppo", "dqn", "rppo"])
    parser.add_argument("--timesteps", type=int, default=1_000_000)
    parser.add_argument("--no-curriculum", action="store_true",
                        help="Curriculum Learning deaktivieren (direkt volle Distanz)")
    parser.add_argument("--monsters", action="store_true",
                        help="Monster aktivieren (Standard: deaktiviert fuer sauberes Navigationstraining)")
    parser.add_argument("--rnd", action="store_true",
                        help="RND (Random Network Distillation) intrinsische Motivation aktivieren")
    args = parser.parse_args()

    _rebuild_sim()

    disable_mobs = not args.monsters
    use_rnd = args.rnd
    print(f"Starte Training mit {args.algo.upper()} für {args.timesteps:,} Steps...")
    print(f"Monster: {'AN' if args.monsters else 'AUS (--monsters zum Aktivieren)'}")
    print(f"RND: {'AN (intrinsic_scale=0.5)' if use_rnd else 'AUS (--rnd zum Aktivieren)'}")
    if not args.no_curriculum:
        print("Curriculum Learning: leistungsbasiert (4 Stufen)")

    n_envs = 4 if args.algo == "rppo" else (8 if args.algo == "ppo" else 1)
    env = make_vec_env(lambda: make_env(disable_mobs=disable_mobs, use_rnd=use_rnd), n_envs=n_envs)

    best_model_path = f"./best_models_{args.algo}/"

    # Eval-Env immer auf voller Distanz (35-45) → misst echten Fortschritt.
    eval_env = Monitor(make_env(disable_mobs=disable_mobs, eval_mode=True))

    eval_callback = EvalCallback(
        eval_env,
        best_model_save_path=best_model_path,
        log_path="./logs/",
        eval_freq=max(1, 20_000 // n_envs),
        n_eval_episodes=20,
        deterministic=True,
        render=False,
        verbose=1,
        warn=False,
    )

    callbacks = [eval_callback]
    if not args.no_curriculum:
        callbacks.append(CurriculumCallback(args.timesteps, verbose=1))

    tensorboard_folder = "./tensorboard_logs/"

    # 239 Input-Features (225 Grid + 5 Skalare + 9 Potential-Field).
    # [256,256] hat genuegend Kapazitaet fuer raeumliche Muster + Richtungsfeatures.
    net_arch = [256, 256]

    if args.algo == "rppo":
        # RecurrentPPO (LSTM-PPO) — analog zu DreamerV3's Kernvorteil: Gedächtnis.
        # Der Agent erinnert sich an die letzten Schritte (verhindert ↑↓-Pendeln strukturell).
        # lstm_hidden_size=256: genug Kapazität um Trajektorie der letzten ~20 Schritte zu merken.
        # 8 Envs liefern diverse Episoden → On-Policy-Update stabilisiert sich schnell.
        model = RecurrentPPO(
            "MlpLstmPolicy",
            env,
            verbose=1,
            n_steps=512,
            batch_size=256,
            n_epochs=4,
            learning_rate=3e-4,
            gamma=0.995,
            ent_coef=0.01,
            gae_lambda=0.95,
            policy_kwargs=dict(
                net_arch=net_arch,
                lstm_hidden_size=256,
                n_lstm_layers=1,
                shared_lstm=False,
                enable_critic_lstm=True,
            ),
            tensorboard_log=tensorboard_folder,
        )
    elif args.algo == "ppo":
        model = PPO(
            "MlpPolicy",
            env,
            verbose=1,
            n_steps=2048,
            batch_size=256,
            learning_rate=3e-4,
            gamma=0.995,
            ent_coef=0.01,
            policy_kwargs=dict(net_arch=net_arch),
            tensorboard_log=tensorboard_folder,
        )
    else:
        model = DQN(
            "MlpPolicy",
            env,
            verbose=1,
            learning_rate=3e-4,
            buffer_size=500_000,
            learning_starts=10_000,
            batch_size=256,
            train_freq=4,
            target_update_interval=500,
            exploration_fraction=0.70,
            exploration_final_eps=0.05,
            gamma=0.995,
            policy_kwargs=dict(net_arch=net_arch),
            tensorboard_log=tensorboard_folder,
        )

    model.learn(
        total_timesteps=args.timesteps,
        tb_log_name=f"{args.algo}_run",
        callback=CallbackList(callbacks),
    )
    print(f"Training beendet. Bestes Modell: '{best_model_path}best_model.zip'")


if __name__ == "__main__":
    main()
