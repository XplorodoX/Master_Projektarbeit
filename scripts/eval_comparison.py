"""Vergleichs-Evaluation: Ablation A → B → C → D

Fragestellung:
  A: MLP + BFS in Obs       → Baseline (historisch, ppo_phase4)
  B: MLP, kein BFS          → Was kostet das Entfernen von BFS?  (ppo_no_bfs)
  C: LSTM, kein BFS         → Hilft Gedächtnis ohne BFS?         (ppo_lstm_curriculum)
  D: LSTM+CNN, kein BFS     → Hilft CNN + Visited Mask?          (ppo_lstm_cnn)

B vs. C vs. D ist der saubere Vergleich (gleiche Env, gleiche Seeds, nur Architektur).
A ist historische Baseline (236-Feature-Env, forceGuaranteedPath=true — andere Bedingungen,
daher nur als Referenz-Obergrenze zu interpretieren).

Verwendung:
    python scripts/eval_comparison.py
    python scripts/eval_comparison.py --seeds-b 8000 8050
    python scripts/eval_comparison.py --stochastic
"""
from __future__ import annotations

import argparse
import os
import sys

import numpy as np
from stable_baselines3 import PPO
from sb3_contrib import RecurrentPPO

_PYTHON_DIR = os.path.join(os.path.dirname(__file__), "..", "python")
if _PYTHON_DIR not in sys.path:
    sys.path.insert(0, _PYTHON_DIR)

from stoneforge_env import StoneforgeWorldEnv
from cnn_extractor import StoneforgeGridCNN  # noqa: F401 — wird beim Laden von Condition D benötigt
from doc_logger import save_eval_results

# ──────────────────────────────────────────────────────────────────────────────
EXPERIMENTS = {
    "A_ppo_phase4": {
        "path":  "models/ppo_phase4/best_model.zip",
        "algo":  "ppo",
        "label": "A — MLP + BFS (Baseline)",
        "note":  "236-Feature-Env, forceGuaranteedPath=true — andere Bedingungen!",
        "skip":  True,   # inkompatibel mit aktueller 230-Feature-Env
    },
    "B_ppo_no_bfs": {
        "path":  "models/ppo_no_bfs/best_model.zip",
        "algo":  "ppo",
        "label": "B — MLP, kein BFS",
        "note":  "230-Feature-Env, forceGuaranteedPath=false",
        "skip":  False,
        "use_step_frac": False,
    },
    "C_ppo_lstm": {
        "path":  "models/ppo_lstm_curriculum/best_model.zip",
        "algo":  "rppo",
        "label": "C — LSTM, kein BFS",
        "note":  "231-Feature-Env, forceGuaranteedPath=false, Curriculum",
        "skip":  False,
        "visited_mask": False,
    },
    "C_v5_ppo_lstm": {
        "path":  "models/ppo_lstm_curriculum_v5_s0/best_model.zip",
        "algo":  "rppo",
        "label": "C v5 — LSTM, last action/reward, batch=256",
        "note":  "236-Feature-Env, forceGuaranteedPath=false, Curriculum",
        "skip":  False,
        "visited_mask": False,
        "use_last_action_reward": True,
    },
    "C_v10_ppo_lstm": {
        "path":  "models/ppo_lstm_curriculum_v10_reproduction/best_model.zip",
        "algo":  "rppo",
        "label": "C v10 — LSTM, reproduction (v2 params)",
        "note":  "231-Feature-Env, forceGuaranteedPath=false, Curriculum",
        "skip":  False,
        "visited_mask": False,
    },
    "D_ppo_lstm_cnn": {
        "path":  "models/ppo_lstm_cnn/best_model",
        "algo":  "rppo",
        "label": "D — LSTM+CNN, kein BFS",
        "note":  "461-Feature-Env (2×15×15 CNN + Visited Mask + Action Buffer), PLR",
        "skip":  False,
        "visited_mask": True,
        "use_last_action_reward": True,
    },
}

MAX_STEPS = 4000


def eval_ppo(model, seeds: list[int], exit_min: int, exit_max: int,
             deterministic: bool = True,
             env_kwargs: dict | None = None) -> dict:
    env = StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max,
                              **(env_kwargs or {}))
    successes, lengths, returns = 0, [], []
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        done, ep_ret, steps = False, 0.0, 0
        while not done and steps < MAX_STEPS:
            action, _ = model.predict(obs, deterministic=deterministic)
            obs, r, term, trunc, info = env.step(int(action))
            ep_ret += float(r); steps += 1
            done = term or trunc
            if info.get("reached_exit"):
                successes += 1
                break
        lengths.append(steps); returns.append(ep_ret)
    return {"successes": successes, "n": len(seeds),
            "mean_len": np.mean(lengths), "mean_ret": np.mean(returns)}


def eval_rppo(model, seeds: list[int], exit_min: int, exit_max: int,
              deterministic: bool = True,
              env_kwargs: dict | None = None) -> dict:
    env = StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max,
                              **(env_kwargs or {}))
    successes, lengths, returns = 0, [], []
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        done, ep_ret, steps = False, 0.0, 0
        lstm_states = None
        ep_start = np.ones((1,), dtype=bool)
        while not done and steps < MAX_STEPS:
            action, lstm_states = model.predict(
                obs.reshape(1, -1),
                state=lstm_states,
                episode_start=ep_start,
                deterministic=deterministic,
            )
            obs, r, term, trunc, info = env.step(int(action[0]))
            ep_start = np.zeros((1,), dtype=bool)
            ep_ret += float(r); steps += 1
            done = term or trunc
            if info.get("reached_exit"):
                successes += 1
                break
        lengths.append(steps); returns.append(ep_ret)
    return {"successes": successes, "n": len(seeds),
            "mean_len": np.mean(lengths), "mean_ret": np.mean(returns)}


def run_eval(key: str, cfg: dict, seeds: list[int], exit_min: int, exit_max: int,
             deterministic: bool = True) -> dict | None:
    if cfg["skip"]:
        return None
    try:
        # Env-Format automatisch aus der Obs-Dimension des Modells ableiten
        # (alte 231/236/…-dim Modelle vs. neue 229-dim, siehe stoneforge_env._OBS_DIM_KWARGS)
        from stoneforge_env import env_kwargs_for_model
        if cfg["algo"] == "rppo":
            model = RecurrentPPO.load(cfg["path"])
            res = eval_rppo(model, seeds, exit_min, exit_max,
                            deterministic=deterministic,
                            env_kwargs=env_kwargs_for_model(model))
        else:
            model = PPO.load(cfg["path"])
            res = eval_ppo(model, seeds, exit_min, exit_max,
                            deterministic=deterministic,
                            env_kwargs=env_kwargs_for_model(model))
        return res
    except Exception as e:
        print(f"  ⚠️  Fehler bei {key}: {e}", file=sys.stderr)
        return None


def print_table(results: dict, seeds_label: str, deterministic: bool = True) -> None:
    det_label = "deterministisch" if deterministic else "stochastisch"
    print(f"\n{'─'*72}")
    print(f"  Ablation-Vergleich  |  Seeds: {seeds_label}  |  {det_label}")
    print(f"{'─'*72}")
    print(f"  {'Bedingung':<30} {'SR':>6}  {'Erfolge':>8}  {'Ø Len':>7}  {'Ø Return':>9}")
    print(f"{'─'*72}")

    # Historisches Ergebnis für A (hartcodiert, andere Env-Bedingungen)
    print(f"  {'A — MLP + BFS (Baseline)':<30} {'100.0%':>6}  {'50/50':>8}  {'49.0':>7}  {'+100.5':>9}")
    print(f"    ↳ historisch, andere Env-Bedingungen (236 Features, guaranteed path)")

    for key, cfg in EXPERIMENTS.items():
        if cfg["skip"]:
            continue
        res = results.get(key)
        if res is None:
            print(f"  {cfg['label']:<30} {'—':>6}  {'—':>8}  {'—':>7}  {'—':>9}  (Fehler/fehlt)")
            continue
        sr = res["successes"] / res["n"]
        frac = f"{res['successes']}/{res['n']}"
        print(f"  {cfg['label']:<30} {sr:>6.1%}  {frac:>8}  {res['mean_len']:>7.1f}  {res['mean_ret']:>+9.2f}")
        print(f"    ↳ {cfg['note']}")

    print(f"{'─'*72}")
    print()


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser()
    p.add_argument("--seeds-a", nargs=2, type=int, default=[7000, 7050],
                   metavar=("START", "END"), help="Testset A: Seeds START bis END-1")
    p.add_argument("--seeds-b", nargs=2, type=int, default=None,
                   metavar=("START", "END"), help="Optionales Testset B (Holdout)")
    p.add_argument("--exit-min", type=int, default=35)
    p.add_argument("--exit-max", type=int, default=45)
    p.add_argument("--stochastic", action="store_true",
                   help="Stochastische Eval (deterministic=False) — entspricht Training-Callback")
    return p.parse_args()


def _results_to_conditions(results: dict, deterministic: bool) -> list[dict]:
    """Wandelt run_eval-Ergebnisse in eine speicherbare Liste um."""
    from datetime import datetime
    conditions = []
    # Historische Baseline A (hartcodiert)
    conditions.append({
        "label":     "A — MLP + BFS (Baseline)",
        "model_path":"models/ppo_phase4/best_model.zip",
        "sr":        1.0,
        "successes": 50,
        "n":         50,
        "mean_len":  49.0,
        "mean_ret":  100.5,
        "note":      "historisch, andere Env-Bedingungen (236 Features, guaranteed path)",
        "deterministic": deterministic,
    })
    for key, cfg in EXPERIMENTS.items():
        if cfg["skip"]:
            continue
        res = results.get(key)
        if res is None:
            continue
        sr = res["successes"] / res["n"]
        conditions.append({
            "label":      cfg["label"],
            "model_path": cfg["path"],
            "sr":         round(sr, 4),
            "successes":  res["successes"],
            "n":          res["n"],
            "mean_len":   round(float(res["mean_len"]), 1),
            "mean_ret":   round(float(res["mean_ret"]), 2),
            "note":       cfg.get("note", ""),
            "deterministic": deterministic,
        })
    return conditions


def main() -> None:
    args = parse_args()
    seeds_a = list(range(args.seeds_a[0], args.seeds_a[1]))
    exit_min, exit_max = args.exit_min, args.exit_max
    deterministic = not args.stochastic

    from datetime import datetime
    det_label = "deterministisch" if deterministic else "stochastisch"
    det_slug  = "det" if deterministic else "stoch"
    date_str  = datetime.now().strftime("%Y%m%d")

    print(f"\nEvaluiere auf Seeds {seeds_a[0]}–{seeds_a[-1]}, exit={exit_min}–{exit_max}, {det_label} ...")
    results_a: dict = {}
    for key, cfg in EXPERIMENTS.items():
        if cfg["skip"]:
            continue
        print(f"  → {cfg['label']} ...")
        res = run_eval(key, cfg, seeds_a, exit_min, exit_max, deterministic=deterministic)
        results_a[key] = res

    print_table(results_a, f"{seeds_a[0]}–{seeds_a[-1]}", deterministic=deterministic)

    # Testset A automatisch speichern
    save_eval_results(
        name=f"ablation_{date_str}_{det_slug}_testsetA",
        metadata={
            "seeds_label": f"{seeds_a[0]}–{seeds_a[-1]}",
            "exit_range":  f"{exit_min}–{exit_max}",
            "deterministic": deterministic,
            "date":        datetime.now().strftime("%d.%m.%Y"),
        },
        conditions=_results_to_conditions(results_a, deterministic),
    )

    if args.seeds_b:
        seeds_b = list(range(args.seeds_b[0], args.seeds_b[1]))
        print(f"Evaluiere Holdout Seeds {seeds_b[0]}–{seeds_b[-1]}, {det_label} ...")
        results_b: dict = {}
        for key, cfg in EXPERIMENTS.items():
            if cfg["skip"]:
                continue
            print(f"  → {cfg['label']} ...")
            res = run_eval(key, cfg, seeds_b, exit_min, exit_max, deterministic=deterministic)
            results_b[key] = res
        print_table(results_b, f"{seeds_b[0]}–{seeds_b[-1]} (Holdout)", deterministic=deterministic)

        # Holdout B automatisch speichern
        save_eval_results(
            name=f"ablation_{date_str}_{det_slug}_holdoutB",
            metadata={
                "seeds_label": f"{seeds_b[0]}–{seeds_b[-1]} (Holdout)",
                "exit_range":  f"{exit_min}–{exit_max}",
                "deterministic": deterministic,
                "date":        datetime.now().strftime("%d.%m.%Y"),
            },
            conditions=_results_to_conditions(results_b, deterministic),
        )


if __name__ == "__main__":
    main()
