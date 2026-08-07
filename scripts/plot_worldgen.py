#!/usr/bin/env python3
"""Erzeugt die Abbildungen zum Weltgenerator fuer die Projektdokumentation.

    figures/fig_worldgen_pipeline.pdf   Stufe 0 -> Stufe 1 -> fertige Welt
    figures/fig_noise_vergleich.pdf     Hash-Rauschen gegen Wertrauschen

Die Tile-Panels stammen aus dem C++-Kern (`tile_at`), sind also Grundwahrheit.
Das Biomfeld ist im Binding nicht exponiert und wird aus einem Nachbau der
Generatorfunktionen berechnet; der Nachbau reproduziert die Tile-Typen des
Kerns auf allen geprueften Feldern exakt (siehe `verify()` unten).

Aufruf:  python scripts/plot_worldgen.py
"""
from __future__ import annotations

import math
import os
import sys
from collections import deque

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import ListedColormap
from matplotlib.patches import Patch

import stoneforge_sim as sf

# --------------------------------------------------------------------------- Setup

SEED = 7000
RADIUS = 45                      # halbe Kantenlaenge des dargestellten Fensters
OUT = os.path.join(os.path.dirname(__file__), "..", "docs", "figures")

MASK64 = (1 << 64) - 1
BIOME_SALT, DENSITY_SALT, ORE_SALT, TREE_SALT = (
    2882395322, 270544960, 2575857510, 1430532898,
)
CHUNK = 8

BIOME_NAMES = ["Grasland", "Wald", "Wüste", "Bergland", "Steppe", "Tundra", "Hölle"]
# wallThreshold, oreThreshold, treeThreshold  (World::sampleBaseTile)
THRESHOLDS = {
    0: (0.10, 0.030, 0.05), 1: (0.07, 0.015, 0.23), 2: (0.19, 0.080, 0.00),
    3: (0.25, 0.100, 0.02), 4: (0.11, 0.030, 0.09), 5: (0.14, 0.050, 0.07),
    6: (0.23, 0.090, 0.06),
}

# --------------------------------------------------------------- Generator-Nachbau


def _u64(v: int) -> int:
    return v & MASK64


def _mix(v: int) -> int:
    """SplitMix64-Finalizer, identisch zu World::mix()."""
    v = _u64(v)
    v ^= v >> 30
    v = _u64(v * 0xBF58476D1CE4E5B9)
    v ^= v >> 27
    v = _u64(v * 0x94D049BB133111EB)
    v ^= v >> 31
    return v


def noise01(x: int, y: int, salt: int, seed: int = SEED) -> float:
    v = _u64(_u64(x) * 0x9E3779B185EBCA87)
    v ^= _u64(_u64(y) * 0xC2B2AE3D27D4EB4F)
    v ^= _u64(seed * 0x165667B19E3779F9)
    v ^= _u64(salt)
    v = _mix(v)
    return (v & 0xFFFFFFFFFFFF) / float(0xFFFFFFFFFFFF)


def _smoothstep(t: float) -> float:
    t = min(max(t, 0.0), 1.0)
    return t * t * (3.0 - 2.0 * t)


def _sample_value(x: float, y: float, salt: int) -> float:
    """Interpoliertes Wertrauschen ueber einem groben Stuetzgitter."""
    x0, y0 = math.floor(x), math.floor(y)
    tx, ty = _smoothstep(x - x0), _smoothstep(y - y0)
    v00 = noise01(x0, y0, salt)
    v10 = noise01(x0 + 1, y0, salt)
    v01 = noise01(x0, y0 + 1, salt)
    v11 = noise01(x0 + 1, y0 + 1, salt)
    top = v00 + (v10 - v00) * tx
    bot = v01 + (v11 - v01) * tx
    return top + (bot - top) * ty


def biome_field(cx: int, cy: int) -> float:
    x, y = cx * 0.22, cy * 0.22
    wx = _sample_value(x * 0.57 + 19.3, y * 0.57 - 7.1, BIOME_SALT ^ 0x9F4A7C15)
    wy = _sample_value(x * 0.57 - 11.8, y * 0.57 + 13.4, BIOME_SALT ^ 0xC2B2AE35)
    x += (wx - 0.5) * 2.8
    y += (wy - 0.5) * 2.8
    n1 = _sample_value(x, y, BIOME_SALT ^ 0x31F2A3B6)
    n2 = _sample_value(x * 2.1 - 3.7, y * 2.1 + 1.9, BIOME_SALT ^ 0x7E2D4C91)
    n3 = _sample_value(x * 4.2 + 8.4, y * 4.2 - 5.6, BIOME_SALT ^ 0x4B9AA21D)
    return min(max(n1 * 0.62 + n2 * 0.28 + n3 * 0.10, 0.0), 1.0)


def biome_tag(cx: int, cy: int) -> int:
    n = biome_field(cx, cy)
    for i in range(1, 7):
        if n < i / 7.0:
            return i - 1
    return 6


def _cdiv(a: int, b: int) -> int:
    """C++-Integer-Division: trunkiert zur Null hin."""
    return -((-a) // b) if a < 0 else a // b


def is_lake(x: int, y: int) -> bool:
    a = noise01(_cdiv(x, 7), _cdiv(y, 7), DENSITY_SALT ^ 0x77AA44CC)
    b = noise01(_cdiv(x, 3), _cdiv(y, 3), TREE_SALT ^ 0x11CC88DD)
    return 0.75 * a + 0.25 * b > 0.86


def base_tile(x: int, y: int) -> int:
    """Stufe 1 fuer ein einzelnes Feld. 0=leer 1=Wand 2=Erz 4=Baum."""
    tag = biome_tag(x // CHUNK, y // CHUNK)
    wall_t, ore_t, tree_t = THRESHOLDS[tag]
    if is_lake(x, y):
        return 0
    tile = 0
    if noise01(x, y, DENSITY_SALT) < wall_t:
        tile = 1
    if tag == 3 and noise01(x, y, ORE_SALT) < ore_t:
        tile = 2
    if tree_t > 0.0 and tile == 0 and noise01(x, y, TREE_SALT) < tree_t:
        tile = 4
    return tile


# --------------------------------------------------------------------- Hilfsmittel


def make_env():
    env = sf.StoneforgeCoreEnv(42)
    env.configure_world_generation(
        exit_min_distance=35, exit_max_distance=45,
        force_guaranteed_path=False, disable_mobs=True, disable_energy=True,
    )
    return env


def verify(env) -> tuple[int, int]:
    """Prueft den Nachbau gegen den C++-Kern. Landmarken (Stufe 3) und die
    freigeraeumten Felder um Spawn und Exit sind ausgenommen."""
    ex, ey = env.exit_pos()
    ok = bad = 0
    for y in range(-RADIUS, RADIUS + 1):
        for x in range(-RADIUS, RADIUS + 1):
            if abs(x) <= 2 and abs(y) <= 2:
                continue
            if abs(x - ex) <= 1 and abs(y - ey) <= 1:
                continue
            actual = env.tile_at(x, y)
            if actual in (8, 9, 10, 11, 12, 13, 14):     # Landmarken
                continue
            ok, bad = (ok + 1, bad) if base_tile(x, y) == actual else (ok, bad + 1)
    return ok, bad


def shortest_path(env, start, goal):
    """BFS ueber begehbare Felder, Vierer-Nachbarschaft."""
    passable = {0, 3}
    prev = {start: None}
    q = deque([start])
    while q:
        cur = q.popleft()
        if cur == goal:
            break
        cx, cy = cur
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nxt = (cx + dx, cy + dy)
            if nxt in prev or max(abs(nxt[0]), abs(nxt[1])) > RADIUS + 5:
                continue
            if env.tile_at(*nxt) not in passable:
                continue
            prev[nxt] = cur
            q.append(nxt)
    if goal not in prev:
        return []
    path, node = [], goal
    while node is not None:
        path.append(node)
        node = prev[node]
    return path[::-1]


# ------------------------------------------------------------------- Abbildung 1

TILE_COLORS = {
    0: "#f2efe6",   # leer
    1: "#5a5a5a",   # Wand
    2: "#b8860b",   # Erz
    3: "#d62728",   # Exit
    4: "#4a7c3f",   # Baum
}
STRUCT_COLOR = "#8d6e9e"


def tile_grid(env):
    n = 2 * RADIUS + 1
    grid = np.zeros((n, n), dtype=int)
    for iy, y in enumerate(range(-RADIUS, RADIUS + 1)):
        for ix, x in enumerate(range(-RADIUS, RADIUS + 1)):
            t = env.tile_at(x, y)
            grid[iy, ix] = 5 if t >= 8 else {0: 0, 1: 1, 2: 2, 3: 3, 4: 4}.get(t, 0)
    return grid


def plot_pipeline(env):
    n = 2 * RADIUS + 1
    coords = range(-RADIUS, RADIUS + 1)

    # Panel A: Biomfeld je Chunk
    biome = np.zeros((n, n), dtype=int)
    for iy, y in enumerate(coords):
        for ix, x in enumerate(coords):
            biome[iy, ix] = biome_tag(x // CHUNK, y // CHUNK)

    # Panel B: Basisbelegung (Stufe 1, ohne Landmarken)
    base = np.zeros((n, n), dtype=int)
    for iy, y in enumerate(coords):
        for ix, x in enumerate(coords):
            base[iy, ix] = {0: 0, 1: 1, 2: 2, 4: 4}[base_tile(x, y)]

    # Panel C: fertige Welt aus dem Kern
    final = tile_grid(env)
    ex, ey = env.exit_pos()
    path = shortest_path(env, (0, 0), (ex, ey))

    fig, axes = plt.subplots(1, 3, figsize=(13.2, 4.9))
    extent = [-RADIUS - 0.5, RADIUS + 0.5, -RADIUS - 0.5, RADIUS + 0.5]

    biome_cmap = ListedColormap(
        ["#cfe3b4", "#7fa96a", "#e8d9a0", "#a9a9a9", "#d9dfa8", "#dfeaf0", "#c98f8f"]
    )
    axes[0].imshow(biome, cmap=biome_cmap, origin="lower", extent=extent,
                   vmin=0, vmax=6, interpolation="nearest")
    axes[0].set_title("Stufe 0: Biomfeld\n(ein Biom je Chunk)", fontsize=10)
    for c in range(-RADIUS // CHUNK, RADIUS // CHUNK + 2):
        axes[0].axhline(c * CHUNK - 0.5, color="white", lw=0.35, alpha=0.55)
        axes[0].axvline(c * CHUNK - 0.5, color="white", lw=0.35, alpha=0.55)
    present = sorted(set(biome.flatten()))
    axes[0].legend(
        handles=[Patch(facecolor=biome_cmap(t), label=BIOME_NAMES[t]) for t in present],
        loc="upper left", fontsize=6.5, framealpha=0.9, borderpad=0.35,
    )

    tile_cmap = ListedColormap([TILE_COLORS[0], TILE_COLORS[1], TILE_COLORS[2],
                                TILE_COLORS[3], TILE_COLORS[4], STRUCT_COLOR])
    axes[1].imshow(base, cmap=tile_cmap, origin="lower", extent=extent,
                   vmin=0, vmax=5, interpolation="nearest")
    axes[1].set_title("Stufe 1: Basisbelegung\n(Rauschwert gegen Schwelle)", fontsize=10)

    axes[2].imshow(final, cmap=tile_cmap, origin="lower", extent=extent,
                   vmin=0, vmax=5, interpolation="nearest")
    if path:
        axes[2].plot([p[0] for p in path], [p[1] for p in path],
                     color="#d62728", lw=1.6, alpha=0.95, label=f"BFS-Weg ({len(path)-1} Schritte)")
    axes[2].plot(0, 0, "o", color="#1f77b4", ms=7, mec="white", mew=1.2, label="Spawn")
    axes[2].plot(ex, ey, "*", color="#d62728", ms=15, mec="white", mew=1.0, label="Exit")
    axes[2].set_title("Fertige Welt mit kürzestem Weg\n(Stufe 3 + Exit-Platzierung)",
                      fontsize=10)
    axes[2].legend(loc="upper left", fontsize=6.5, framealpha=0.9, borderpad=0.35)

    for ax in axes:
        ax.set_xticks([]); ax.set_yticks([])
        for s in ax.spines.values():
            s.set_linewidth(0.6)

    fig.suptitle(f"Weltgenerierung, Seed {SEED} "
                 f"({2*RADIUS+1}×{2*RADIUS+1}-Ausschnitt um den Spawn)",
                 fontsize=11, y=0.99)
    fig.tight_layout(rect=(0, 0, 1, 0.95))
    out = os.path.abspath(os.path.join(OUT, "fig_worldgen_pipeline.pdf"))
    fig.savefig(out, bbox_inches="tight")
    fig.savefig(out.replace(".pdf", ".png"), dpi=170, bbox_inches="tight")
    plt.close(fig)
    return out, len(path) - 1 if path else -1


# ------------------------------------------------------------------- Abbildung 2


def plot_noise_comparison():
    """Warum Hash-Rauschen keine Barrieren erzeugt und Wertrauschen schon."""
    n, thr = 64, 0.30
    hashed = np.zeros((n, n))
    valued = np.zeros((n, n))
    for y in range(n):
        for x in range(n):
            hashed[y, x] = noise01(x, y, 0xABCDEF)
            valued[y, x] = _sample_value(x / 6.0, y / 6.0, 0xABCDEF)

    fig, axes = plt.subplots(2, 2, figsize=(7.4, 7.2))
    for col, (field, name) in enumerate(
        ((hashed, "Hash-Rauschen\n(jedes Feld unabhängig)"),
         (valued, "Wertrauschen\n(Stützgitter + Interpolation)")),
    ):
        axes[0, col].imshow(field, cmap="gray", origin="lower", vmin=0, vmax=1,
                            interpolation="nearest")
        axes[0, col].set_title(name, fontsize=9.5)
        axes[1, col].imshow(field < thr, cmap=ListedColormap(["#f2efe6", "#5a5a5a"]),
                            origin="lower", interpolation="nearest")
        axes[1, col].set_title(f"nach Schwellwert $<{thr}$", fontsize=9.5)
    for ax in axes.flatten():
        ax.set_xticks([]); ax.set_yticks([])
        for s in ax.spines.values():
            s.set_linewidth(0.6)
    axes[0, 0].set_ylabel("Rohes Feld", fontsize=9)
    axes[1, 0].set_ylabel("Als Hindernis gelesen", fontsize=9)

    fig.tight_layout()
    out = os.path.abspath(os.path.join(OUT, "fig_noise_vergleich.pdf"))
    fig.savefig(out, bbox_inches="tight")
    fig.savefig(out.replace(".pdf", ".png"), dpi=170, bbox_inches="tight")
    plt.close(fig)
    return out


# --------------------------------------------------------------------------- Main


def main() -> int:
    env = make_env()
    env.reset(SEED)

    ok, bad = verify(env)
    print(f"Nachbau gegen C++-Kern: {ok} übereinstimmend, {bad} abweichend")
    if bad:
        print("FEHLER: Nachbau weicht ab, Abbildung wäre nicht belastbar.")
        return 1

    pipeline, steps = plot_pipeline(env)
    print(f"geschrieben: {pipeline}  (BFS-Weg {steps} Schritte)")
    noise = plot_noise_comparison()
    print(f"geschrieben: {noise}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
