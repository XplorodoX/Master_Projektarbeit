"""Erzeugt die Schema-Abbildungen für Abschnitt 2.1 (Grundlagen der prozeduralen
Weltgenerierung): Value Noise, Domain Warping, zelluläre Automaten, BFS.

Alle vier sind Konzept-Illustrationen der zugrunde liegenden Algorithmen (wie die
bereits vorhandenen RL-Grundlagen-Abbildungen fig:ppo_clip, fig:pbrs_potential,
fig:rnn_policy), keine Messdaten aus der Projektarbeit — die Rauschfunktionen sind
eigenständig implementiert, aber pädagogisch motiviert und nicht mit dem
tatsächlichen `world.cpp`-Rauschkern identisch. Die Geburts-/Überlebensregel des
Zellulären-Automaten-Beispiels (B5/S4) entspricht der in der Projektkonfiguration
hinterlegten Standardregel (siehe CLAUDE.md, Bekannte Fallstricke).

Aufruf:
    python scripts/plot_grundlagen_figures.py
"""

from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

OUT_DIR = Path("docs/Doku/Bilder")

C_BLUE = "#4C8BD4"
C_BLUE_DARK = "#1B4F91"
C_ORANGE = "#EB6834"
C_GRAY = "#6B6B6B"
C_WALL = "#4A4A46"
C_FLOOR = "#EDEFF1"

plt.rcParams.update({
    "font.size": 15,
    "axes.titlesize": 18,
    "axes.edgecolor": "#333333",
    "axes.grid": True,
    "grid.color": "#E5E5E5",
    "grid.linewidth": 0.8,
    "axes.axisbelow": True,
    "figure.facecolor": "white",
    "axes.facecolor": "white",
})


def savefig(fig, name):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    path = OUT_DIR / name
    fig.savefig(path, dpi=200, bbox_inches="tight")
    print(f"gespeichert: {path}")
    plt.close(fig)


def smoothstep(t):
    return 3 * t**2 - 2 * t**3


# ---------------------------------------------------------------- 1: Value Noise
def plot_value_noise():
    rng = np.random.default_rng(7)
    nodes_x = np.arange(0, 5)
    nodes_y = rng.random(5)

    x = np.linspace(0, 4, 400)
    y_lerp = np.interp(x, nodes_x, nodes_y)

    y_smooth = np.zeros_like(x)
    for i, xv in enumerate(x):
        cell = min(int(xv), 3)
        t = xv - cell
        s = smoothstep(t)
        y_smooth[i] = nodes_y[cell] * (1 - s) + nodes_y[cell + 1] * s

    fig, ax = plt.subplots(figsize=(10, 5.5))
    ax.plot(x, y_lerp, "--", color=C_GRAY, linewidth=1.6, label="lineare Interpolation")
    ax.plot(x, y_smooth, "-", color=C_BLUE_DARK, linewidth=2.4, label="Smoothstep-Interpolation")
    ax.scatter(nodes_x, nodes_y, s=110, color=C_ORANGE, zorder=5, edgecolor="#333333",
               linewidth=1.2, label="Knotenwerte (pseudozufällig)")
    for gx in nodes_x:
        ax.axvline(gx, color="#DDDDDD", linewidth=1, zorder=0)

    ax.set_xlabel("Position $x$")
    ax.set_ylabel("Rauschwert")
    ax.set_title("Value Noise: Knotenwerte und Interpolation")
    ax.legend(loc="upper right", frameon=False, fontsize=13)
    savefig(fig, "grundlagen_value_noise.png")


# ---------------------------------------------------------------- 2: Domain Warping
def value_noise_2d(shape, cell=10, seed=0):
    rng = np.random.default_rng(seed)
    gy, gx = shape[0] // cell + 2, shape[1] // cell + 2
    grid = rng.random((gy, gx))
    out = np.zeros(shape)
    ys, xs = np.indices(shape)
    fy, fx = ys / cell, xs / cell
    iy, ix = fy.astype(int), fx.astype(int)
    ty, tx = smoothstep(fy - iy), smoothstep(fx - ix)
    v00 = grid[iy, ix]
    v10 = grid[iy, ix + 1]
    v01 = grid[iy + 1, ix]
    v11 = grid[iy + 1, ix + 1]
    out = (v00 * (1 - tx) + v10 * tx) * (1 - ty) + (v01 * (1 - tx) + v11 * tx) * ty
    return out


def plot_domain_warping():
    shape = (120, 120)
    base = value_noise_2d(shape, cell=14, seed=1)

    warp_x = value_noise_2d(shape, cell=18, seed=2)
    warp_y = value_noise_2d(shape, cell=18, seed=3)
    ys, xs = np.indices(shape)
    amp = 22
    wx = np.clip(xs + (warp_x - 0.5) * amp, 0, shape[1] - 1).astype(int)
    wy = np.clip(ys + (warp_y - 0.5) * amp, 0, shape[0] - 1).astype(int)
    warped = base[wy, wx]

    fig, axes = plt.subplots(1, 2, figsize=(11, 5.2))
    for ax, field, title in zip(axes, [base, warped], ["Unverzerrt", "Domain-Warped"]):
        ax.imshow(field, cmap="Blues_r", origin="lower")
        ax.set_title(title)
        ax.set_xticks([])
        ax.set_yticks([])
        ax.grid(False)
    fig.suptitle("Domain Warping einer Rauschfunktion", fontsize=18, y=1.02)
    savefig(fig, "grundlagen_domain_warping.png")


# ---------------------------------------------------------------- 3: Zelluläre Automaten
def cellular_step(grid, birth=5, survive=4):
    padded = np.pad(grid, 1, mode="constant", constant_values=1)
    neighbor_walls = sum(
        np.roll(np.roll(padded, dy, axis=0), dx, axis=1)
        for dy in (-1, 0, 1) for dx in (-1, 0, 1) if not (dy == 0 and dx == 0)
    )[1:-1, 1:-1]
    new = np.where(grid == 1, neighbor_walls >= survive, neighbor_walls >= birth)
    return new.astype(int)


def plot_cellular_automata():
    rng = np.random.default_rng(3)
    grid = (rng.random((50, 50)) < 0.45).astype(int)
    initial = grid.copy()
    for _ in range(4):
        grid = cellular_step(grid, birth=5, survive=4)

    fig, axes = plt.subplots(1, 2, figsize=(11, 5.2))
    for ax, field, title in zip(axes, [initial, grid],
                                 ["Initialzustand (Rauschen)", "Nach 4 Iterationen (B5/S4)"]):
        ax.imshow(field, cmap="gray_r", origin="lower", vmin=0, vmax=1)
        ax.set_title(title)
        ax.set_xticks([])
        ax.set_yticks([])
        ax.grid(False)
    fig.suptitle("Zelluläre Automaten zur Gefügeglättung", fontsize=18, y=1.02)
    savefig(fig, "grundlagen_zellulaere_automaten.png")


# ---------------------------------------------------------------- 4: BFS
def plot_bfs():
    rng = np.random.default_rng(11)
    size = 15
    walls = (rng.random((size, size)) < 0.22).astype(bool)
    start = (1, 1)
    walls[start] = False
    walls[size - 2, size - 2] = False

    dist = np.full((size, size), -1)
    dist[start] = 0
    frontier = [start]
    parent = {}
    while frontier:
        nxt = []
        for (y, x) in frontier:
            for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                ny, nx = y + dy, x + dx
                if 0 <= ny < size and 0 <= nx < size and not walls[ny, nx] and dist[ny, nx] == -1:
                    dist[ny, nx] = dist[y, x] + 1
                    parent[(ny, nx)] = (y, x)
                    nxt.append((ny, nx))
        frontier = nxt

    goal = (size - 2, size - 2)
    path = [goal]
    while path[-1] != start:
        path.append(parent[path[-1]])
    path = path[::-1]

    fig, ax = plt.subplots(figsize=(7.5, 7.5))
    display = np.ma.masked_where(walls, dist)
    im = ax.imshow(display, cmap="Blues", origin="lower", vmin=0)
    wall_overlay = np.ma.masked_where(~walls, np.ones_like(dist))
    ax.imshow(wall_overlay, cmap="Greys", origin="lower", vmin=0, vmax=1.4)

    py = [p[0] for p in path]
    px = [p[1] for p in path]
    ax.plot(px, py, "-", color=C_ORANGE, linewidth=3, zorder=5)
    ax.scatter(*start[::-1], s=160, color="white", edgecolor=C_ORANGE, linewidth=2.5, zorder=6)
    ax.text(start[1], start[0], "S", ha="center", va="center", fontweight="bold", fontsize=13, zorder=7)
    ax.scatter(*goal[::-1], s=160, color=C_ORANGE, edgecolor="#333333", linewidth=1.5, zorder=6)
    ax.text(goal[1], goal[0], "Z", ha="center", va="center", color="white",
            fontweight="bold", fontsize=13, zorder=7)

    ax.set_xticks([])
    ax.set_yticks([])
    ax.grid(False)
    ax.set_title("BFS-Distanzfeld und kürzester Pfad")
    cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cbar.set_label("BFS-Distanz vom Start")
    savefig(fig, "grundlagen_bfs.png")


if __name__ == "__main__":
    plot_value_noise()
    plot_domain_warping()
    plot_cellular_automata()
    plot_bfs()
