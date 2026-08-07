"""Weltgeometrie vermessen: Wie schwer ist die Umgebung wirklich?

Hintergrund (CHANGELOG v2026-08-05.1): Ein Gutachtenvorschlag wollte die Umgebung
ueber die zellulaere Glaettung haerten, damit die Kompass-Heuristik (92 % SR)
einbricht. Dieses Skript hat die Annahme widerlegt, bevor eine einzige
Trainingsstunde verbrannt wurde.

Leitmetrik ist der **Umwegfaktor** = BFS-Distanz / Manhattan-Distanz. Er misst,
ob es ueberhaupt etwas zu umrunden gibt:
  1,0  = kuerzester Weg ist die Luftlinie -> jede Richtungsheuristik kommt durch
  >1,3 = echte Hindernisse, Navigation noetig

Die Success Rate allein taugt dafuer nicht: bei Cap 4000 saettigt sie, und ein
Zufallslauf mit leichtem Drift erreicht 92 %, ohne irgendetwas zu koennen.

Aufruf:
    python scripts/probe_world_geometry.py            # Status quo vs. Glaettung an
    python scripts/probe_world_geometry.py --sweep    # Regel-Sweep der Glaettung

Kein Rebuild noetig: StoneforgeCoreEnv liest game_config.json im Konstruktor
(src/python/py_module.cpp:44-48), jedes neu gebaute Env uebernimmt den Patch.
Die Originalconfig wird in finally immer wiederhergestellt.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import numpy as np

PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "build"))
sys.path.insert(0, str(PROJECT_ROOT / "python"))

CONFIG_PATH = PROJECT_ROOT / "assets" / "base" / "game_config.json"
TESTSET_A = list(range(7000, 7050))
TESTSET_B = list(range(8000, 8050))

# TileType (src/include/stoneforge/types.hpp): Empty=0, Exit=3 sind begehbar.
PASSABLE = {0, 3}

# (enable, iterations, birth, survival)
SWEEP_GRID = [
    (False, 0, 0, 0),
    (True,  2, 5, 4),   # Default aus der JSON
    (True,  2, 4, 3),
    (True,  1, 3, 3),
    (True,  2, 3, 2),
    (True,  4, 3, 2),
    (True,  2, 2, 1),
]


def deep_merge(base: dict, ovr: dict) -> dict:
    out = dict(base)
    for k, v in ovr.items():
        out[k] = deep_merge(out[k], v) if isinstance(v, dict) and isinstance(out.get(k), dict) else v
    return out


def measure(label: str, seeds: list[int]) -> dict:
    """Misst Loesbarkeit, Wanddichte und Umwegfaktor ueber die Seeds."""
    from stoneforge_env import StoneforgeWorldEnv

    env = StoneforgeWorldEnv(exit_min=35, exit_max=45)
    solvable, bfs, manh, detour, density = 0, [], [], [], []

    for seed in seeds:
        env.reset(seed=seed)
        ok = bool(env.core.is_path_to_exit_reachable())
        solvable += int(ok)

        px, py = env.core.player_pos()
        ex, ey = env.core.exit_pos()
        m = abs(ex - px) + abs(ey - py)
        d = env.core.current_bfs_distance_to_exit()
        if ok and d > 0 and m > 0:
            bfs.append(d)
            manh.append(m)
            detour.append(d / m)

        # Wanddichte im Bounding-Rechteck Spawn<->Exit, jedes 2. Tile als Schaetzer
        x0, x1 = min(px, ex) - 5, max(px, ex) + 5
        y0, y1 = min(py, ey) - 5, max(py, ey) + 5
        solid = total = 0
        for x in range(x0, x1 + 1, 2):
            for y in range(y0, y1 + 1, 2):
                total += 1
                solid += int(env.core.tile_at(x, y) not in PASSABLE)
        density.append(solid / max(total, 1))

    env.close()
    return {
        "label": label,
        "solvable": solvable,
        "n": len(seeds),
        "wall_density": float(np.mean(density)),
        "manhattan_mean": float(np.mean(manh)) if manh else float("nan"),
        "bfs_mean": float(np.mean(bfs)) if bfs else float("nan"),
        "detour_mean": float(np.mean(detour)) if detour else float("nan"),
        "detour_p90": float(np.percentile(detour, 90)) if detour else float("nan"),
        "detour_max": float(np.max(detour)) if detour else float("nan"),
    }


def print_row(r: dict) -> None:
    print(f"{r['label']:34s} {r['solvable']:3d}/{r['n']:<4d} {r['wall_density']:10.3f} "
          f"{r['bfs_mean']:8.1f} {r['detour_mean']:9.3f} {r['detour_p90']:8.3f}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--sweep", action="store_true",
                    help="Regel-Sweep statt nur AUS/AN")
    ap.add_argument("--out", default="logs/eval_results/world_geometry.json")
    args = ap.parse_args()

    seeds = TESTSET_A if args.sweep else TESTSET_A + TESTSET_B
    grid = SWEEP_GRID if args.sweep else [(False, 0, 0, 0), (True, 2, 5, 4)]

    original = CONFIG_PATH.read_text()
    results = []
    print(f"Seeds: {len(seeds)}  (Testset A{'' if args.sweep else ' + B'})\n")
    print(f"{'Konfiguration':34s} {'loesbar':>8s} {'Wanddichte':>10s} "
          f"{'BFS Ø':>8s} {'Umweg Ø':>9s} {'p90':>8s}")
    print("-" * 88)
    try:
        base = json.loads(original)
        for enable, iters, birth, surv in grid:
            label = "Glaettung AUS" if not enable else f"AN it={iters} b={birth} s={surv}"
            patched = deep_merge(base, {"worldgen": {"procedural": {
                "enableCellularSmoothing": enable,
                "cellularIterations": iters,
                "cellularBirthMinNeighbors": birth,
                "cellularSurvivalMinNeighbors": surv,
            }}}) if enable else deep_merge(base, {"worldgen": {"procedural": {
                "enableCellularSmoothing": False}}})
            CONFIG_PATH.write_text(json.dumps(patched, indent=2))
            r = measure(label, seeds)
            print_row(r)
            results.append(r)
    finally:
        CONFIG_PATH.write_text(original)
        print("\nOriginal game_config.json wiederhergestellt.")

    out = PROJECT_ROOT / args.out
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(results, indent=2))
    print(f"Gespeichert: {out}")


if __name__ == "__main__":
    main()
