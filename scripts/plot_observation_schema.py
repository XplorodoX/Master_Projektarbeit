"""Schemazeichnung des 229-dimensionalen Beobachtungsvektors (Kapitel 4.2.2).

Konzeptzeichnung, keine echten Weltdaten: veranschaulicht die Zusammensetzung
225 (15x15-Sichtfeld) + Trefferpunkte + exitDx + exitDy + Schrittbudget-Anteil.

Aufruf:
    python scripts/plot_observation_schema.py
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Rectangle

OUT_DIR = Path("docs/Doku/Bilder")

C_FLOOR = "#EDEEF0"
C_WALL = "#4A4A4A"
C_AGENT = "#EB6834"
C_EXIT = "#1BAF7A"
C_FEATURE = "#2A78D6"
C_TEXT = "#222222"

rng = np.random.default_rng(3)


def build_demo_grid(n=15):
    grid = rng.random((n, n)) < 0.22
    grid[n // 2, n // 2] = False
    return grid


def main():
    n = 15
    grid = build_demo_grid(n)

    fig, ax = plt.subplots(figsize=(14, 8))
    ax.set_xlim(-1, 24)
    ax.set_ylim(-1.5, n + 1.5)
    ax.axis("off")
    ax.invert_yaxis()

    ax.set_title("Aufbau des Beobachtungsvektors (229 Dimensionen), Konzeptdarstellung",
                  fontsize=16, fontweight="bold", color=C_TEXT, y=1.03)

    for y in range(n):
        for x in range(n):
            color = C_WALL if grid[y, x] else C_FLOOR
            ax.add_patch(Rectangle((x, y), 1, 1, facecolor=color, edgecolor="white", linewidth=0.6))
    ax.add_patch(Rectangle((n // 2, n // 2), 1, 1, facecolor=C_AGENT, edgecolor="white", linewidth=0.6))
    ax.add_patch(Rectangle((-0.15, -0.15), n + 0.3, n + 0.3, fill=False,
                            edgecolor=C_TEXT, linewidth=2.2))
    ax.text(n / 2, -0.9, r"$15 \times 15$ Sichtfeld um den Agenten (225 Kacheln)",
            ha="center", fontsize=13, color=C_TEXT)
    ax.annotate("Agent", (n // 2 + 0.5, n // 2 + 0.5), textcoords="offset points",
                xytext=(0, -26), ha="center", fontsize=11, fontweight="bold", color=C_AGENT)

    feat_x = n + 3.2
    features = [
        ("Trefferpunkte", "HP"),
        ("exitDx", r"$\Delta x$ zum Ausgang, normiert"),
        ("exitDy", r"$\Delta y$ zum Ausgang, normiert"),
        ("Schrittbudget-Anteil", "verbrauchter Anteil des Limits"),
    ]
    box_h = 1.7
    gap = 0.55
    total_h = len(features) * box_h + (len(features) - 1) * gap
    y_start = (n - total_h) / 2
    centers = []
    for i, (name, desc) in enumerate(features):
        y = y_start + i * (box_h + gap)
        centers.append(y + box_h / 2)
        box = FancyBboxPatch((feat_x, y), 5.4, box_h, boxstyle="round,pad=0.02,rounding_size=0.12",
                              facecolor=C_FEATURE, edgecolor="none")
        ax.add_patch(box)
        ax.text(feat_x + 2.7, y + box_h / 2 - 0.28, name, ha="center", va="center",
                fontsize=12.5, fontweight="bold", color="white")
        ax.text(feat_x + 2.7, y + box_h / 2 + 0.42, desc, ha="center", va="center",
                fontsize=9.5, color="white")

    grid_c = (n / 2, n / 2)
    join_x = feat_x + 6.4
    ax.annotate("", xy=(join_x, n / 2), xytext=(n + 0.3, n / 2),
                arrowprops=dict(arrowstyle="-", color=C_TEXT, lw=0))
    arr = FancyArrowPatch((n + 0.3, n / 2), (join_x - 0.3, n / 2),
                           connectionstyle="arc3,rad=0", arrowstyle="-|>",
                           color=C_TEXT, linewidth=1.6, mutation_scale=18)
    ax.add_patch(arr)
    for cy in centers:
        arr2 = FancyArrowPatch((feat_x - 0.05, cy), (join_x - 0.3, n / 2),
                                connectionstyle="arc3,rad=0.0", arrowstyle="-",
                                color=C_TEXT, linewidth=1.1, alpha=0.55, zorder=1)
        ax.add_patch(arr2)

    vec_box = FancyBboxPatch((join_x, n / 2 - 1.1), 4.6, 2.2,
                              boxstyle="round,pad=0.02,rounding_size=0.14",
                              facecolor="white", edgecolor=C_TEXT, linewidth=1.8)
    ax.add_patch(vec_box)
    ax.text(join_x + 2.3, n / 2 - 0.25, r"$o_t \in \mathbb{R}^{229}$", ha="center", va="center",
            fontsize=15, fontweight="bold", color=C_TEXT)
    ax.text(join_x + 2.3, n / 2 + 0.55, "225 + 4", ha="center", va="center",
            fontsize=11, color="#555555")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    path = OUT_DIR / "observation_schema.png"
    fig.savefig(path, dpi=200, bbox_inches="tight")
    print(f"gespeichert: {path}")
    plt.close(fig)


if __name__ == "__main__":
    main()
