#!/usr/bin/env python3
"""
Hard-World Eval — Stoneforge RL Projektarbeit

Testet ob der Agent auf härteren prozedurell generierten Welten generalisiert.
Ohne diesen Test bleibt die 98%-Aussage wissenschaftlich angreifbar: gemessen
wurde auf einer fast leeren Welt (Wand 0.05, kein Cellular Smoothing).

Weltparameter HARD vs. STANDARD:
  coldWallThreshold  : 0.05 → 0.15   (3× dichter)
  warmWallThreshold  : 0.05 → 0.15
  mossWallThreshold  : 0.05 → 0.15
  enableCellularSmoothing: false → true  (cave-artige Strukturen)
  forceGuaranteedPath: true  bleibt! — sonst "keine Lösung" ≠ "Agent scheitert"
  exitMinDistance / exitMaxDistance: 35–45  (gleich wie Standard-Eval)

BFS-Encoding-Kompatibilität:
  Default : Delta-Encoding  (post-Phase-4-Retraining, aktuelles stoneforge_env.py)
  --legacy-bfs : Absolut-Encoding  (Phase-4-Modell und früher, _BFS_MAX=64)

Verwendung:
  # Phase-4-Modell (Absolut-BFS, 98% auf Standard-Welt):
  python scripts/eval_hard_world.py \\
      --model models/ppo_phase4/final_model.zip --legacy-bfs

  # Nach Retraining mit Delta-BFS:
  python scripts/eval_hard_world.py --model models/ppo_phase5/best_model.zip
"""

from __future__ import annotations

import argparse
import json
import sys
import types
from pathlib import Path

import numpy as np

PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "build"))
sys.path.insert(0, str(PROJECT_ROOT / "python"))

CONFIG_PATH = PROJECT_ROOT / "assets" / "base" / "game_config.json"
EVAL_SEEDS_A = list(range(7000, 7050))
EVAL_SEEDS_B = list(range(8000, 8050))

# Nur die Felder, die sich gegenüber der Standard-Config ändern.
# Tiefes Dict-Merge: alle anderen Keys bleiben unberührt.
#
# ⚠️ WARNUNG (05.08.2026): Die *WallThreshold/*OreThreshold/*TreeThreshold-Keys sind
# TOT. Die Biom-Schwellwerte stehen hartkodiert in World::sampleBaseTile() (world.cpp)
# und sind nicht über game_config.json tunebar. Von diesem Override wirkt faktisch nur
# der procedural-Block. Und der macht die Welt nicht schwerer, sondern LEERER:
# enableCellularSmoothing senkt die Wanddichte von 0,238 auf 0,037 und den Umwegfaktor
# auf 1,001 (Messung: scripts/probe_world_geometry.py, CHANGELOG v2026-08-05.1).
# Dieses Skript misst also keine "harte" Welt. Vor Weiterverwendung reparieren.
HARD_WORLD_OVERRIDES: dict = {
    "worldgen": {
        "coldWallThreshold": 0.15,
        "warmWallThreshold": 0.15,
        "mossWallThreshold": 0.15,
        "coldOreThreshold":  0.05,
        "warmOreThreshold":  0.07,
        "mossOreThreshold":  0.04,
        "warmTreeThreshold": 0.08,
        "mossTreeThreshold": 0.08,
        "procedural": {
            "enableCellularSmoothing": True,
            "cellularIterations":      2,
        },
    }
}


def _deep_merge(base: dict, overrides: dict) -> dict:
    result = dict(base)
    for k, v in overrides.items():
        if isinstance(v, dict) and isinstance(result.get(k), dict):
            result[k] = _deep_merge(result[k], v)
        else:
            result[k] = v
    return result


def _patch_legacy_bfs(env) -> None:
    """Überschreibt env._bfs_field() mit der alten Absolutwert-Kodierung.

    Phase-4 und frühere Modelle wurden mit _BFS_MAX=64 (absolute Abstände)
    trainiert. Nach der Delta-BFS-Änderung (Änderung 8) würde ein neues
    stoneforge_env.py dem Modell komplett andere Feature-Werte liefern.
    """
    def _legacy_bfs(self) -> np.ndarray:
        cur   = self.core.current_bfs_distance_to_exit()
        up    = self.core.bfs_distance_at_offset( 0, -1)
        down  = self.core.bfs_distance_at_offset( 0,  1)
        left  = self.core.bfs_distance_at_offset(-1,  0)
        right = self.core.bfs_distance_at_offset( 1,  0)
        return np.clip(
            np.array([cur, up, down, left, right], dtype=np.float32) / 64.0,
            0.0, 1.0,
        )
    env._bfs_field = types.MethodType(_legacy_bfs, env)


def _run_seeds(model, env, seeds: list[int], deterministic: bool) -> dict:
    successes, lengths, returns = 0, [], []
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        done, ep_ret, steps, reached = False, 0.0, 0, False
        while not done and steps < 4000:
            action, _ = model.predict(obs, deterministic=deterministic)
            obs, r, term, trunc, info = env.step(int(action))
            ep_ret += float(r)
            steps  += 1
            if info.get("reached_exit", False):
                reached = True
            done = term or trunc
        successes += int(reached)
        lengths.append(steps)
        returns.append(ep_ret)
    n = len(seeds)
    return {
        "success":     successes,
        "n":           n,
        "rate":        successes / n,
        "mean_len":    float(np.mean(lengths)),
        "mean_return": float(np.mean(returns)),
    }


def _print_result(label: str, r: dict, mode: str) -> None:
    bar = "=" * 68
    print(f"\n{bar}")
    print(f"  {label}  [{mode}]")
    print(f"{bar}")
    print(f"  Success : {r['success']:2d} / {r['n']}  ({r['rate']:.1%})")
    print(f"  mean_len: {r['mean_len']:.1f}  |  mean_return: {r['mean_return']:.2f}")
    print(f"{bar}")


def _changelog_row(label: str, r: dict, mode: str) -> str:
    return (
        f"| PPO ({label}, {mode}) "
        f"| {r['success']} / 50 "
        f"| {r['rate']:.1%} "
        f"| {r['mean_len']:.1f} "
        f"| {r['mean_return']:.2f} "
        f"| TT.MM.JJJJ |"
    )


def main() -> None:
    parser = argparse.ArgumentParser(description="Stoneforge Hard-World Eval")
    parser.add_argument(
        "--model",
        default="models/ppo_phase4/final_model.zip",
        help="Pfad zum Modell relativ zum Projektroot",
    )
    parser.add_argument(
        "--legacy-bfs",
        action="store_true",
        help="Alte BFS-Absolutwert-Kodierung verwenden (Phase-4 und früher)",
    )
    parser.add_argument(
        "--det-only",
        action="store_true",
        help="Nur deterministischer Eval (deterministic=True)",
    )
    parser.add_argument(
        "--stoch-only",
        action="store_true",
        help="Nur stochastischer Eval (deterministic=False)",
    )
    args = parser.parse_args()

    model_path = PROJECT_ROOT / args.model
    if not model_path.exists():
        print(f"FEHLER: Modell nicht gefunden: {model_path}", file=sys.stderr)
        sys.exit(1)

    original_config_text = CONFIG_PATH.read_text(encoding="utf-8")
    base_cfg  = json.loads(original_config_text)
    hard_cfg  = _deep_merge(base_cfg, HARD_WORLD_OVERRIDES)

    encoding_label = "LEGACY (absolut, /64)" if args.legacy_bfs else "Delta (aktuell)"
    print("=== Stoneforge Hard-World Eval ===")
    print(f"Modell        : {model_path.relative_to(PROJECT_ROOT)}")
    print(f"BFS-Kodierung : {encoding_label}")
    print(f"Wall-Threshold: 0.05 → 0.15  |  Cellular Smoothing: an")
    print(f"forceGuaranteedPath: {hard_cfg['worldgen']['forceGuaranteedPath']}  (Pfad garantiert)")

    try:
        CONFIG_PATH.write_text(json.dumps(hard_cfg, indent=2), encoding="utf-8")

        from stable_baselines3 import PPO
        from stoneforge_env import StoneforgeWorldEnv

        env   = StoneforgeWorldEnv(exit_min=35, exit_max=45)
        model = PPO.load(str(model_path))

        if args.legacy_bfs:
            _patch_legacy_bfs(env)
            print("Legacy-BFS-Patch aktiv: env._bfs_field() → Absolutwerte /64")

        run_stoch = not args.det_only
        run_det   = not args.stoch_only
        results: dict[str, dict] = {}

        if run_stoch:
            print("\nStochastischer Eval (deterministic=False) — Testset A ...")
            results["stoch_A"] = _run_seeds(model, env, EVAL_SEEDS_A, deterministic=False)
            _print_result("Hard-World Testset A", results["stoch_A"], "stochastisch")

            print("\nStochastischer Eval (deterministic=False) — Testset B ...")
            results["stoch_B"] = _run_seeds(model, env, EVAL_SEEDS_B, deterministic=False)
            _print_result("Hard-World Testset B", results["stoch_B"], "stochastisch")

        if run_det:
            print("\nDeterministischer Eval (deterministic=True) — Testset A ...")
            results["det_A"] = _run_seeds(model, env, EVAL_SEEDS_A, deterministic=True)
            _print_result("Hard-World Testset A", results["det_A"], "deterministisch")

            print("\nDeterministischer Eval (deterministic=True) — Testset B ...")
            results["det_B"] = _run_seeds(model, env, EVAL_SEEDS_B, deterministic=True)
            _print_result("Hard-World Testset B", results["det_B"], "deterministisch")

        print("\n--- Changelog-Zeilen (kopieren nach CHANGELOG.md) ---")
        if run_stoch:
            print(_changelog_row("Hard-World Testset A", results["stoch_A"], "stoch."))
            print(_changelog_row("Hard-World Testset B", results["stoch_B"], "stoch."))
        if run_det:
            print(_changelog_row("Hard-World Testset A", results["det_A"],   "det."))
            print(_changelog_row("Hard-World Testset B", results["det_B"],   "det."))

    finally:
        CONFIG_PATH.write_text(original_config_text, encoding="utf-8")
        print("\nOriginal game_config.json wiederhergestellt.")


if __name__ == "__main__":
    main()
