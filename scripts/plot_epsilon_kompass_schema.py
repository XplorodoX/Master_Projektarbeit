"""Erzeugt Abbildung 3.2 (Entscheidungsregel der eps-Kompass-Heuristik) neu.

Ersetzt die alte, statische Grafik, die den Nicht-Zufalls-Zweig fälschlich als
"(BFS-Gradient)" beschriftete. Laut Implementierung (scripts/eval_baselines.py,
CompassPolicy.__call__) wertet der Kompass ausschließlich die Luftlinien-
Richtungskomponenten (dx, dy) aus dem Beobachtungsvektor aus und geht einen
Schritt auf der laengeren Achse -- keine Breitensuche, kein Pfadwissen. Das ist
in Kapitel 4.2.6 und 6.1 explizit so beschrieben ("wandblind").

Aufruf:
    python scripts/plot_epsilon_kompass_schema.py
"""

from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

OUT_DIR = Path("docs/Doku/Bilder")

# Gleiche Palette wie in plot_eval_results.py, damit der Kompass ueberall
# dieselbe Farbe traegt (siehe Stilregel "eine Farbe pro Verfahren").
C_RANDOM = "#B8B7B0"
C_COMPASS_MAIN = "#2A78D6"
C_NEUTRAL = "#EEF0F2"
C_NEUTRAL_EDGE = "#9AA0A6"
C_TEXT = "#222222"
C_COMPASS = {
    0.3: "#9EC5F4", 0.5: "#5598E7", 0.6: "#2A78D6", 0.8: "#1C5CAB", 0.9: "#104281",
}

plt.rcParams.update({"font.size": 15, "figure.facecolor": "white"})


def box(ax, xy, w, h, text, facecolor, edgecolor, textcolor="#222222", fontsize=17, weight="bold"):
    x, y = xy
    patch = FancyBboxPatch(
        (x - w / 2, y - h / 2), w, h,
        boxstyle="round,pad=0.02,rounding_size=0.08",
        facecolor=facecolor, edgecolor=edgecolor, linewidth=1.8, mutation_aspect=1,
    )
    ax.add_patch(patch)
    ax.text(x, y, text, ha="center", va="center", fontsize=fontsize, fontweight=weight,
             color=textcolor, linespacing=1.4)
    return patch


def curved(ax, p0, p1, rad):
    arrow = FancyArrowPatch(
        p0, p1, connectionstyle=f"arc3,rad={rad}",
        arrowstyle="-", color="#6E7378", linewidth=1.6, mutation_scale=1,
    )
    ax.add_patch(arrow)


def plot_schema():
    fig, ax = plt.subplots(figsize=(15, 10))
    ax.set_xlim(0, 15)
    ax.set_ylim(0, 10.5)
    ax.axis("off")

    ax.text(7.5, 10.1, r"Entscheidungsregel der $\varepsilon$-Kompass-Heuristik",
             ha="center", va="top", fontsize=24, fontweight="bold", color=C_TEXT)

    top = (7.5, 8.7)
    box(ax, top, 6.2, 1.15, r"Agent an Position $s_t$", C_NEUTRAL, C_NEUTRAL_EDGE, fontsize=18)

    decision = (7.5, 6.9)
    box(ax, decision, 2.6, 1.7, r"Zufallszahl" + "\n" + r"$< \varepsilon$?",
        "#EEF0F2", "#333333", fontsize=17, weight="regular")

    left = (3.3, 4.6)
    box(ax, left, 4.6, 1.15, "Zufällige Aktion", C_RANDOM, C_RANDOM, textcolor="white", fontsize=18)

    right = (11.7, 4.6)
    box(ax, right, 5.6, 1.15, "Aktion Richtung Ziel\n(Luftlinie)",
        C_COMPASS_MAIN, C_COMPASS_MAIN, textcolor="white", fontsize=18)

    bottom = (7.5, 2.3)
    box(ax, bottom, 4.2, 1.15, r"Aktion $a_t$", C_NEUTRAL, C_NEUTRAL_EDGE, fontsize=18)

    curved(ax, (6.7, 6.1), (4.3, 5.2), 0.25)
    curved(ax, (8.3, 6.1), (10.7, 5.2), -0.25)
    curved(ax, (3.7, 4.0), (6.7, 2.75), -0.3)
    curved(ax, (11.3, 4.0), (8.3, 2.75), 0.3)

    ax.text(2.0, 5.85, r"ja  $(p = \varepsilon)$", ha="center", fontsize=15, color=C_TEXT)
    ax.text(13.0, 5.85, r"nein  $(p = 1-\varepsilon)$", ha="center", fontsize=15, color=C_TEXT)

    ax.text(0.3, 1.15, r"Ausgewertete $\varepsilon$-Werte:", ha="left", fontsize=15, color=C_TEXT)
    xs = [0.9, 3.4, 5.9, 8.4, 10.9]
    for x0, (eps, color) in zip(xs, sorted(C_COMPASS.items())):
        ax.add_patch(plt.Rectangle((x0, 0.25), 0.55, 0.55, facecolor=color, edgecolor="none"))
        ax.text(x0 + 0.275, -0.15, f"{eps:g}".replace(".", ","), ha="center", fontsize=14, color=C_TEXT)

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    path = OUT_DIR / "epsilon_kompass_schema.png"
    fig.savefig(path, dpi=200, bbox_inches="tight")
    print(f"gespeichert: {path}")
    plt.close(fig)


if __name__ == "__main__":
    plot_schema()
