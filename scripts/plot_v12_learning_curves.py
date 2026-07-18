#!/usr/bin/env python3
"""Lernkurven der sieben finalen v12-Läufe (Validierungs-Seeds 6000–6049).

Ein Panel je Curriculum-Phase (innerhalb einer Phase ist die Schwierigkeit
konstant und die Läufe sind vergleichbar; eine kumulative Achse würde wegen
des leistungsbasierten Gatings Werte verschiedener Phasen mischen). Je Panel:
alle sieben stochastischen SR-Verläufe als dünne Linien plus Ensemble-Mittel
(interpoliert, nur wo mindestens 4 Läufe Daten haben).

Verwendung:
    python scripts/plot_v12_learning_curves.py \
        [-o docs/figures/fig_v12_lernkurven.pdf]
"""
from __future__ import annotations

import argparse
import json
import os
import re

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# Druckfreundliche Farben (konsistent mit plot_run_curve.py / plot_det_gap_distanz.py)
C_RUN = "#2a78d6"    # blau — Einzelläufe (dünn)
C_MEAN = "#0f3f78"   # dunkelblau — Ensemble-Mittel stochastisch
C_DET = "#1baf7a"    # aqua — Ensemble-Mittel deterministisch
C_MUTED = "#898781"
C_GRID = "#e1e0d9"

RUNS = [f"ppo_lstm_curriculum_v12_s{i}" for i in range(1, 8)]
PHASES = [
    (1, "Phase 1 (Exit 5–12)"),
    (2, "Phase 2 (Exit 12–25)"),
    (3, "Phase 3 (Exit 25–45, Eval 35–45)"),
    (4, "Phase 4 (Greedy Fine-Tune)"),
]
MIN_RUNS_FOR_MEAN = 4


def load_run(path: str):
    """Eval-Historie laden, je Phase (aus dem Label) getrennt sammeln.

    Das Feld ``sr`` enthält die Gating-Metrik der Phase (stoch in P1/2, det
    in P3/4); det- und stoch-Wert werden deshalb aus dem Label geparst.
    """
    with open(path) as f:
        entries = json.load(f)
    entries.sort(key=lambda e: e["timestamp"])
    per_phase: dict[int, list[tuple[int, float, float]]] = {p: [] for p, _ in PHASES}
    for e in entries:
        m = re.match(r"Phase (\d)", e["label"])
        v = re.search(r"det=(\d+)%\|stoch=(\d+)%", e["label"])
        if not m or not v:
            continue
        per_phase[int(m.group(1))].append(
            (e["step"], float(v.group(2)), float(v.group(1))))
    out = {}
    for p, pts in per_phase.items():
        if not pts:
            continue
        s = np.array([s for s, _, _ in pts], float)
        stoch = np.array([y for _, y, _ in pts], float)
        det = np.array([y for _, _, y in pts], float)
        # Zähler auf Phasenstart normalisieren: der rohe Wert läuft je nach
        # Phase weiter bzw. übernimmt den Stand des geladenen Checkpoints.
        out[p] = (s - s[0], stoch, det)
    return out


def ensemble_mean(curves, grid):
    """NaN-maskiertes Mittel über die Läufe, nur wo genug Läufe Daten haben."""
    stack = np.full((len(curves), len(grid)), np.nan)
    for i, (s, y) in enumerate(curves):
        mask = grid <= s[-1]
        stack[i, mask] = np.interp(grid[mask], s, y)
    mean = np.nanmean(stack, axis=0)
    mean[np.sum(~np.isnan(stack), axis=0) < MIN_RUNS_FOR_MEAN] = np.nan
    return mean


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("-o", "--out", default="docs/figures/fig_v12_lernkurven.pdf")
    args = ap.parse_args()

    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    runs = []
    for name in RUNS:
        data = load_run(os.path.join(root, "models", name, "eval_history.json"))
        runs.append(data)
        print(f"{name}: " + ", ".join(
            f"P{p}={len(data[p][0])}pt/{data[p][0][-1]/1e3:.0f}k" for p in sorted(data)))

    fig, axes = plt.subplots(2, 2, figsize=(7.6, 5.2), sharey=True)
    for ax, (phase, title) in zip(axes.flat, PHASES):
        data = [r[phase] for r in runs if phase in r]
        for s, stoch, _ in data:
            ax.plot(s / 1e3, stoch, color=C_RUN, lw=0.8, alpha=0.35, zorder=2)
        grid = np.arange(0, max(s[-1] for s, _, _ in data) + 1, 25_000, float)
        mean_stoch = ensemble_mean([(s, y) for s, y, _ in data], grid)
        mean_det = ensemble_mean([(s, y) for s, _, y in data], grid)
        ax.plot(grid / 1e3, mean_stoch, color=C_MEAN, lw=2.0, zorder=3,
                solid_capstyle="round")
        ax.plot(grid / 1e3, mean_det, color=C_DET, lw=2.0, zorder=3,
                solid_capstyle="round")

        ax.set_title(title, fontsize=9.5, color="black")
        ax.set_ylim(0, 100)
        ax.grid(True, color=C_GRID, lw=0.6)
        ax.set_axisbelow(True)
        for spine in ("top", "right"):
            ax.spines[spine].set_visible(False)
        for spine in ("left", "bottom"):
            ax.spines[spine].set_color(C_MUTED)
        ax.tick_params(colors=C_MUTED, labelcolor="black")

    for ax in axes[1]:
        ax.set_xlabel("Schritte seit Phasenstart [Tsd.]", fontsize=9)
    for ax in axes[:, 0]:
        ax.set_ylabel("SR [%]", fontsize=9)
    # Direktbeschriftung im Phase-4-Panel (dort ist oben Platz) statt Legende
    axes[1, 1].annotate("Einzelläufe s1–s7 (stoch.)", xy=(0.97, 0.92),
                        xycoords="axes fraction", color=C_RUN, alpha=0.85,
                        fontsize=8.5, ha="right")
    axes[1, 1].annotate("Mittel stoch. (≥4 Läufe)", xy=(0.97, 0.82),
                        xycoords="axes fraction", color=C_MEAN, fontsize=8.5,
                        ha="right")
    axes[1, 1].annotate("Mittel det. (≥4 Läufe)", xy=(0.97, 0.72),
                        xycoords="axes fraction", color=C_DET, fontsize=8.5,
                        ha="right")

    fig.suptitle("v12: Success Rate auf den Validierungs-Seeds, je Curriculum-Phase",
                 fontsize=10.5)
    fig.tight_layout(rect=(0, 0, 1, 0.97))
    out = os.path.join(root, args.out) if not os.path.isabs(args.out) else args.out
    fig.savefig(out, bbox_inches="tight")
    fig.savefig(os.path.splitext(out)[0] + ".png", dpi=150, bbox_inches="tight")
    print(f"gespeichert: {out}")


if __name__ == "__main__":
    main()
