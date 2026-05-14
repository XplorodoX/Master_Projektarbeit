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

from stoneforge_env import ExitPotentialFieldWrapper, StoneforgeConfig, StoneforgeWorldEnv

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


def make_env(disable_mobs: bool = True) -> gym.Env:
    cfg = StoneforgeConfig(disable_mobs=disable_mobs)
    return ReducedActionEnv(ExitPotentialFieldWrapper(StoneforgeWorldEnv(cfg)))


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
    # forced_fraction:  Spätestens bei diesem Anteil des Trainings wird
    #                   weitergeschaltet (Sicherheitsnetz).
    ( 5, 12,  -4.0, 0.20),   # sehr nah — Zufallspfad reicht; Grenze: reward > -4
    (12, 22, -18.0, 0.45),   # mittelklein; Grenze: reward > -18
    (22, 35, -35.0, 0.70),   # mittel; Grenze: reward > -35
    (35, 45,  None, 1.00),   # volle Schwierigkeit — bleibt bis Trainingsende
]

_REWARD_WINDOW = 50   # Anzahl Episoden für den gleitenden Mittelwert


class CurriculumCallback(BaseCallback):
    """Leistungsbasiertes Curriculum: Stufe wechselt nur wenn der Agent bereit ist."""

    def __init__(self, total_timesteps: int, verbose: int = 1) -> None:
        super().__init__(verbose)
        self._total_ts = total_timesteps
        self._stage = -1                              # aktuell aktive Stufe (Index)
        self._reward_buf: deque[float] = deque(maxlen=_REWARD_WINDOW)

    def _on_step(self) -> bool:
        # Episode-Rewards aus dem Info-Dict sammeln (Monitor schreibt sie rein).
        for info in self.locals.get("infos", []):
            if "episode" in info:
                self._reward_buf.append(float(info["episode"]["r"]))

        fraction = self.num_timesteps / self._total_ts
        next_stage = self._stage + 1

        if next_stage >= len(_CURRICULUM_STAGES):
            return True   # letzte Stufe bereits aktiv

        emin, emax, threshold, forced = _CURRICULUM_STAGES[next_stage]
        recent_mean = float(np.mean(self._reward_buf)) if len(self._reward_buf) >= 20 else -9999.0

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
    parser.add_argument("--algo", type=str, default="dqn", choices=["ppo", "dqn"])
    parser.add_argument("--timesteps", type=int, default=1_000_000)
    parser.add_argument("--no-curriculum", action="store_true",
                        help="Curriculum Learning deaktivieren (direkt volle Distanz)")
    parser.add_argument("--monsters", action="store_true",
                        help="Monster aktivieren (Standard: deaktiviert fuer sauberes Navigationstraining)")
    args = parser.parse_args()

    _rebuild_sim()

    disable_mobs = not args.monsters
    print(f"Starte Training mit {args.algo.upper()} für {args.timesteps:,} Steps...")
    print(f"Monster: {'AN' if args.monsters else 'AUS (--monsters zum Aktivieren)'}")
    if not args.no_curriculum:
        print("Curriculum Learning: leistungsbasiert (4 Stufen)")

    n_envs = 8 if args.algo == "ppo" else 1
    env = make_vec_env(lambda: make_env(disable_mobs=disable_mobs), n_envs=n_envs)

    best_model_path = f"./best_models_{args.algo}/"

    # Eval-Env immer auf voller Distanz (35-45) → misst echten Fortschritt.
    eval_env = Monitor(make_env(disable_mobs=disable_mobs))

    eval_callback = EvalCallback(
        eval_env,
        best_model_save_path=best_model_path,
        log_path="./logs/",
        eval_freq=max(1, 20_000 // n_envs),
        n_eval_episodes=20,
        deterministic=True,
        render=False,
        verbose=1,
    )

    callbacks = [eval_callback]
    if not args.no_curriculum:
        callbacks.append(CurriculumCallback(args.timesteps, verbose=1))

    tensorboard_folder = "./tensorboard_logs/"

    # 239 Input-Features (225 Grid + 5 Skalare + 9 Potential-Field).
    # [256,256] hat genuegend Kapazitaet fuer raeumliche Muster + Richtungsfeatures.
    net_arch = [256, 256]

    if args.algo == "ppo":
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
            # 3e-4 statt 1e-4: schnelleres Lernen durch groessere Schritte.
            # Mit normalisierter Obs ist der Gradient stabiler → hoehere LR vertretbar.
            learning_rate=3e-4,
            buffer_size=500_000,
            learning_starts=10_000,
            # 256 statt 128: groessere Batches → stabilere Gradient-Schaetzung.
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
