"""Standard-Eval über alle 7 v12-Seeds — Testset A (7000-7049) + Holdout B (8000-8049).

Protokoll (docs/wiki/components/eval-protokoll.md):
  exit 35-45, Cap 4000 (= Env-maxSteps), deterministisch UND stochastisch.
  LSTM-State wird korrekt über die Episode geführt und bei reset() genullt.
Ausgabe: Mittel ± Std mit ddof=1 (korrekt für n=7) UND ddof=0 (wie die 08.07.-Tabelle).
"""
import json
import sys
import time

import numpy as np
from sb3_contrib import RecurrentPPO

from stoneforge_env import StoneforgeWorldEnv, env_kwargs_for_model

SEEDS = [1, 2, 3, 4, 5, 6, 7]
TESTSETS = {"A": range(7000, 7050), "B": range(8000, 8050)}
CAP = 4000


def eval_model(model, env, seeds, deterministic):
    succ = 0
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        lstm_states = None
        episode_starts = np.ones((1,), dtype=bool)
        reached = False
        for _ in range(CAP):
            action, lstm_states = model.predict(
                obs, state=lstm_states, episode_start=episode_starts,
                deterministic=deterministic,
            )
            obs, _r, term, trunc, info = env.step(int(action))
            episode_starts = np.zeros((1,), dtype=bool)
            if info.get("reached_exit", False):
                reached = True
            if term or trunc:
                break
        succ += int(reached)
    return succ / len(seeds)


results = {}
for s in SEEDS:
    path = f"models/ppo_lstm_curriculum_v12_s{s}/best_model.zip"
    t0 = time.time()
    model = RecurrentPPO.load(path, device="cpu")
    kw = env_kwargs_for_model(model)
    env = StoneforgeWorldEnv(exit_min=35, exit_max=45, **kw)
    row = {}
    for name, seeds in TESTSETS.items():
        for det in (True, False):
            row[f"{name}_{'det' if det else 'stoch'}"] = eval_model(model, env, seeds, det)
    results[f"s{s}"] = row
    env.close()
    print(f"  s{s}: {row}  ({time.time()-t0:.0f}s)", flush=True)

with open("scratchpad_final_eval_n7.json", "w") as f:
    json.dump(results, f, indent=2)

print("\n" + "=" * 78)
print(f"{'Lauf':6s} {'A stoch':>9s} {'A det':>8s} {'B stoch':>9s} {'B det':>8s}")
print("-" * 78)
for s in SEEDS:
    r = results[f"s{s}"]
    print(f"s{s:<5d} {r['A_stoch']:>8.0%} {r['A_det']:>8.0%} {r['B_stoch']:>8.0%} {r['B_det']:>8.0%}")
print("-" * 78)
for key, label in [("A_stoch", "A stoch"), ("A_det", "A det"),
                   ("B_stoch", "B stoch"), ("B_det", "B det")]:
    v = np.array([results[f"s{s}"][key] for s in SEEDS]) * 100
    print(f"{label:8s} n=7: {v.mean():5.1f} ± {v.std(ddof=1):4.1f} (ddof=1)"
          f"  |  ± {v.std(ddof=0):4.1f} (ddof=0)")
print("=" * 78)

# n=3 (nur s1-s3) zum Vergleich mit der 08.07.-Tabelle
print("\nZum Vergleich — nur s1-s3 (die 08.07.-Läufe):")
for key, label in [("A_stoch", "A stoch"), ("B_stoch", "B stoch")]:
    v = np.array([results[f"s{s}"][key] for s in [1, 2, 3]]) * 100
    print(f"  {label:8s} n=3: {v.mean():5.1f} ± {v.std(ddof=1):4.1f} (ddof=1) | ± {v.std(ddof=0):4.1f} (ddof=0)")
