"""Nicht-gelernte Referenzpolitiken (Baselines) auf dem Standard-Eval-Protokoll.

Motivation: Ohne eine ungelernte Untergrenze ist eine Success Rate nicht einzuordnen.
Gemessen werden zwei Politiken, die *dieselbe* Observation sehen wie der Agent:

  * ``random``  — uniform zufaellige Bewegung (absolute Untergrenze).
  * ``compass`` — mit Wahrscheinlichkeit eps uniform zufaellig, sonst ein Schritt in
                  die dominante Luftlinien-Kompassrichtung (exitDx/exitDy). Nutzt
                  ausschliesslich 2 der 229 Features; das lokale 15x15-Grid wird
                  ignoriert. Kein Training, kein Gedaechtnis.

Protokoll identisch zu ``scripts/eval_final.py`` / docs/wiki/components/eval-protokoll.md:
Testset A 7000-7049, Holdout B 8000-8049, exit 35-45, Cap 4000.

Zusaetzlich zur Success Rate wird die **Pfadeffizienz** berichtet
(BFS-Optimum / tatsaechliche Schritte, gemittelt ueber die *erfolgreichen* Episoden).
Sie diskriminiert dort, wo die SR bei grosszuegigem Episodenbudget saettigt.

Aufruf:
    python scripts/eval_baselines.py                 # Baselines
    python scripts/eval_baselines.py --models        # zusaetzlich die v12-Modelle
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np

from stoneforge_env import StoneforgeWorldEnv

CAP = 4000
EXIT_MIN, EXIT_MAX = 35, 45
TESTSETS = {"A": list(range(7000, 7050)), "B": list(range(8000, 8050))}

# Obs-Layout Env v11 (229): [grid 0..224 | hp 225 | exitDx 226 | exitDy 227 | step_frac 228]
IDX_DX, IDX_DY = 226, 227
# Aktionen: 0=hoch, 1=runter, 2=links, 3=rechts
UP, DOWN, LEFT, RIGHT = 0, 1, 2, 3


# --------------------------------------------------------------------------- Politiken


class RandomPolicy:
    """Uniform zufaellige Bewegung."""

    name = "Random (uniform)"

    def __init__(self, seed: int = 0) -> None:
        self.rng = np.random.default_rng(seed)

    def reset(self) -> None:
        return None

    def __call__(self, obs, ctx):
        return int(self.rng.integers(4)), None


class CompassPolicy:
    """eps-verrauschter Greedy-Lauf entlang des Luftlinien-Kompasses."""

    def __init__(self, eps: float, seed: int = 0) -> None:
        self.eps = eps
        self.rng = np.random.default_rng(seed)
        self.name = f"Kompass-Zufallslauf (eps={eps:g})"

    def reset(self) -> None:
        return None

    def __call__(self, obs, ctx):
        if self.rng.random() < self.eps:
            return int(self.rng.integers(4)), None
        dx, dy = float(obs[IDX_DX]), float(obs[IDX_DY])
        if abs(dx) > abs(dy):
            return (RIGHT if dx > 0 else LEFT), None
        return (DOWN if dy > 0 else UP), None


class ModelPolicy:
    """RecurrentPPO mit korrekt gefuehrtem LSTM-Zustand."""

    def __init__(self, model, deterministic: bool = False, name: str = "model") -> None:
        self.model = model
        self.deterministic = deterministic
        self.name = name

    def reset(self):
        return (None, np.ones((1,), dtype=bool))

    def __call__(self, obs, ctx):
        state, episode_start = ctx
        action, state = self.model.predict(
            obs, state=state, episode_start=episode_start,
            deterministic=self.deterministic,
        )
        return int(action), (state, np.zeros((1,), dtype=bool))


# --------------------------------------------------------------------------- Messung


def evaluate(policy, env, seeds):
    """Gibt (SR, mittlere Schritte bei Erfolg, mittlere Pfadeffizienz) zurueck."""
    successes, steps_ok, efficiency = 0, [], []
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        optimal = env.core.current_bfs_distance_to_exit()
        ctx = policy.reset()
        reached, used = False, CAP
        for t in range(CAP):
            action, ctx = policy(obs, ctx)
            obs, _r, terminated, truncated, info = env.step(action)
            if info.get("reached_exit", False):
                reached, used = True, t + 1
                break
            if terminated or truncated:
                break
        successes += int(reached)
        if reached:
            steps_ok.append(used)
            if optimal > 0:
                efficiency.append(optimal / used)
    return (
        successes / len(seeds),
        float(np.mean(steps_ok)) if steps_ok else float("nan"),
        float(np.mean(efficiency)) if efficiency else float("nan"),
    )


def run(policy_factory, env_kwargs=None, repeats=5, rng_offset=100):
    """Mittelt ueber `repeats` unabhaengige Politik-RNG-Seeds (Weltseeds bleiben fix)."""
    rows = {}
    for name, seeds in TESTSETS.items():
        srs, lens, effs = [], [], []
        for k in range(repeats):
            env = StoneforgeWorldEnv(exit_min=EXIT_MIN, exit_max=EXIT_MAX, **(env_kwargs or {}))
            sr, ln, ef = evaluate(policy_factory(k + rng_offset), env, seeds)
            env.close()
            srs.append(sr * 100)
            lens.append(ln)
            effs.append(ef)
        rows[name] = {
            "sr_mean": float(np.mean(srs)),
            "sr_std": float(np.std(srs, ddof=1)) if repeats > 1 else 0.0,
            "steps": float(np.nanmean(lens)),
            "efficiency": float(np.nanmean(effs)),
        }
    return rows


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--models", action="store_true", help="v12-Modelle mitmessen")
    # Doku-Tabelle tab:baselines: --repeats 5 --rng-offset 100
    ap.add_argument("--repeats", type=int, default=5)
    ap.add_argument("--rng-offset", type=int, default=100,
                    help="Startwert der Politik-RNG-Seeds (Weltseeds bleiben fix)")
    ap.add_argument("--out", default="logs/eval_results/baselines.json")
    args = ap.parse_args()

    results = {}

    print("BFS-Optimum der Testsets (mittlere kuerzeste Weglaenge):")
    for name, seeds in TESTSETS.items():
        env = StoneforgeWorldEnv(exit_min=EXIT_MIN, exit_max=EXIT_MAX)
        d = []
        for s in seeds:
            env.reset(seed=s)
            d.append(env.core.current_bfs_distance_to_exit())
        env.close()
        print(f"  Testset {name}: min {min(d)} / Ø {np.mean(d):.1f} / max {max(d)}")
        results[f"bfs_optimum_{name}"] = {"min": min(d), "mean": float(np.mean(d)), "max": max(d)}

    specs = [("random", lambda k: RandomPolicy(seed=k))]
    specs += [(f"compass_eps{e:g}", (lambda e_: lambda k: CompassPolicy(e_, seed=k))(e))
              for e in (0.3, 0.4, 0.5, 0.6, 0.8, 0.9)]

    print(f"\n{'Politik':34s} {'A: SR':>13s} {'Schritte':>9s} {'Effiz.':>7s}"
          f" {'B: SR':>13s} {'Schritte':>9s} {'Effiz.':>7s}")
    print("-" * 100)
    for key, factory in specs:
        rows = run(factory, repeats=args.repeats, rng_offset=args.rng_offset)
        results[key] = rows
        a, b = rows["A"], rows["B"]
        label = factory(0).name
        print(f"{label:34s} {a['sr_mean']:6.1f} ± {a['sr_std']:4.1f} {a['steps']:9.0f}"
              f" {a['efficiency']:7.3f} {b['sr_mean']:6.1f} ± {b['sr_std']:4.1f}"
              f" {b['steps']:9.0f} {b['efficiency']:7.3f}")

    if args.models:
        from sb3_contrib import RecurrentPPO
        from stoneforge_env import env_kwargs_for_model
        print("-" * 100)
        for s in range(1, 8):
            path = Path(f"models/ppo_lstm_curriculum_v12_s{s}/best_model.zip")
            if not path.exists():
                continue
            model = RecurrentPPO.load(str(path), device="cpu")
            kw = env_kwargs_for_model(model)
            rows = run(lambda k, m=model: ModelPolicy(m, False, f"v12_s{s}"),
                       env_kwargs=kw, repeats=args.repeats, rng_offset=args.rng_offset)
            results[f"v12_s{s}_stoch"] = rows
            a, b = rows["A"], rows["B"]
            print(f"{'RecurrentPPO v12_s'+str(s)+' (stoch)':34s} {a['sr_mean']:6.1f} ± {a['sr_std']:4.1f}"
                  f" {a['steps']:9.0f} {a['efficiency']:7.3f} {b['sr_mean']:6.1f} ± {b['sr_std']:4.1f}"
                  f" {b['steps']:9.0f} {b['efficiency']:7.3f}")

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(results, indent=2))
    print(f"\nGespeichert: {out}")


if __name__ == "__main__":
    main()
