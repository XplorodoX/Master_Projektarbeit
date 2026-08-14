"""Erzeugt Abbildung 2.7 (Potentialfeld und Shaping-Belohnung, PBRS) neu.

Ersetzt die alte Version, die eine erlaeuternde Legendenzeile fest in die
Bilddatei eingebrannt hatte -- das stand optisch zwischen Grafik und der
eigentlichen LaTeX-Bildunterschrift und wich vom sonst einheitlichen
Abbildungsstil ab. Diese Fassung traegt dieselbe Information ausschliesslich
ueber Farbskala, Achsentitel und die Pfadannotationen im Bild selbst; der
erklaerende Satz bleibt allein der \\caption vorbehalten.

Reine Konzeptdarstellung, keine echten Weltdaten.

Aufruf:
    python scripts/plot_pbrs_potentialfeld.py
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, FancyArrowPatch

OUT_DIR = Path("docs/Doku/Bilder")

C_WALL = "#808080"
C_TEXT = "#222222"
C_PATH = "#222222"
C_GOAL_EDGE = "#EB6834"

# 7x7 Beispielgitter: 0 = frei, 1 = Wand. BFS-Distanz zum Ziel (oben rechts)
# bestimmt das Potential Phi(s) = -distanz.
GRID_WALLS = np.array([
    [0, 0, 0, 0, 0, 0, 0],
    [0, 1, 0, 1, 1, 0, 0],
    [0, 0, 0, 1, 0, 0, 1],
    [0, 1, 1, 0, 0, 1, 0],
    [0, 0, 0, 0, 1, 0, 0],
    [1, 0, 1, 0, 0, 1, 0],
    [0, 0, 0, 0, 0, 0, 0],
], dtype=bool)

GOAL = (6, 0)   # (col, row), row 0 = oben
START = (0, 6)


def bfs_distance(walls, goal):
    h, w = walls.shape
    dist = np.full((h, w), np.inf)
    gx, gy = goal
    dist[gy, gx] = 0
    frontier = [(gx, gy)]
    while frontier:
        nxt = []
        for x, y in frontier:
            for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < w and 0 <= ny < h and not walls[ny, nx] and dist[ny, nx] == np.inf:
                    dist[ny, nx] = dist[y, x] + 1
                    nxt.append((nx, ny))
        frontier = nxt
    return dist


def shortest_path(walls, start, goal, dist):
    path = [start]
    cur = start
    while cur != goal:
        x, y = cur
        best = None
        for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < walls.shape[1] and 0 <= ny < walls.shape[0] and not walls[ny, nx]:
                if best is None or dist[ny, nx] < dist[best[1], best[0]]:
                    best = (nx, ny)
        cur = best
        path.append(cur)
    return path


def main():
    dist = bfs_distance(GRID_WALLS, GOAL)
    phi = -np.where(np.isinf(dist), np.nanmax(dist[np.isfinite(dist)]) + 1, dist)
    path = shortest_path(GRID_WALLS, START, GOAL, dist)

    h, w = GRID_WALLS.shape
    fig, ax = plt.subplots(figsize=(8.2, 7.2))

    disp = np.ma.masked_array(phi, mask=GRID_WALLS)
    im = ax.imshow(disp, cmap="Blues", origin="upper", vmin=phi[np.isfinite(phi)].min(),
                   vmax=0)
    for y in range(h):
        for x in range(w):
            if GRID_WALLS[y, x]:
                ax.add_patch(Rectangle((x - 0.5, y - 0.5), 1, 1, facecolor=C_WALL, edgecolor="white"))

    px = [p[0] for p in path]
    py = [p[1] for p in path]
    ax.plot(px, py, "--", color=C_PATH, linewidth=1.6, zorder=4)
    ax.scatter(px[1:-1], py[1:-1], color=C_PATH, s=28, zorder=5)
    for i in range(1, len(path) - 1, 2):
        x, y = path[i]
        ax.annotate(rf"$\Phi={phi[y, x]:.0f}$", (x, y), textcoords="offset points",
                    xytext=(9, 6), fontsize=10, color=C_TEXT,
                    bbox=dict(facecolor="white", edgecolor="none", alpha=0.75, pad=1.0))

    ax.scatter([START[0]], [START[1]], s=260, facecolor="white", edgecolor=C_PATH,
               linewidth=1.8, zorder=6)
    ax.text(START[0], START[1], "S", ha="center", va="center", fontsize=13, fontweight="bold", zorder=7)
    ax.add_patch(Rectangle((GOAL[0] - 0.5, GOAL[1] - 0.5), 1, 1, fill=False,
                            edgecolor=C_GOAL_EDGE, linewidth=3, zorder=6))
    ax.text(GOAL[0], GOAL[1], "Z", ha="center", va="center", fontsize=13, fontweight="bold",
            color=C_GOAL_EDGE, zorder=7)

    ax.set_xticks([]); ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_visible(False)
    ax.set_title(r"Potentialfeld $\Phi(s)$ und Shaping-Belohnung entlang eines Beispielpfads",
                 fontsize=13.5, fontweight="bold", color=C_TEXT)

    cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.03)
    cbar.set_label(r"Potential $\Phi(s)$ = $-$BFS-Distanz zum Ziel (dunkler = näher)", fontsize=10.5)

    fig.tight_layout()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    path_out = OUT_DIR / "pbrs_potentialfeld.png"
    fig.savefig(path_out, dpi=200, bbox_inches="tight")
    print(f"gespeichert: {path_out}")
    plt.close(fig)


if __name__ == "__main__":
    main()
