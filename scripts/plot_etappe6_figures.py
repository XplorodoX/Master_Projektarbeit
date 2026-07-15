#!/usr/bin/env python3
"""Abbildungen für Etappe 6 (07.07.2026): Eval-Cap-Artefakt + Batch-Größen-Forensik.

Erzeugt:
    docs/figures/fig_cap_artefakt.pdf   — SR desselben Modells als Funktion des Eval-Caps
    docs/figures/fig_batch_vergleich.pdf — batch=8 vs. batch=64 (SR-Kurven + explained_variance)

Datenquellen: CHANGELOG v2026-07-07.2/.4 (Cap-Messung), logs/train_bare_batch8_seed1.log,
logs/train_a1_repro_seed1.log, logs/train_v11_seed1.log (Seed-1-Kontrollarm, Cap-4000-Evals).
"""
from __future__ import annotations

import os
import re

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# Druckfreundliche Farben (CVD-validiert, s. plot_run_curve.py / validate_palette)
C_BLUE   = "#2a78d6"   # batch=8 (nackt)
C_ORANGE = "#c98500"   # batch=64 (nackt, A1-Repro)
C_GRAY   = "#898781"   # batch=64 (Curriculum-Kontrollarm)
C_GREEN  = "#1baf7a"   # det-Serie (Cap-Plot)
C_GRID   = "#e1e0d9"

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "docs", "figures")

# ── Messdaten (07.07.2026) ────────────────────────────────────────────────────
# SR(Cap) von Seed-0 phase1_best (v11), VAL-Seeds 6000–6049, ein Lauf @Cap 4000,
# Steps-bis-Erfolg pro Seed geloggt → SR für jedes Cap ableitbar.
CAPS        = [600, 1000, 1500, 2000, 2500, 3000, 4000]
SR_CAP_STOCH = [48, 62, 72, 82, 82, 82, 86]
SR_CAP_DET   = [18, 18, 18, 18, 18, 18, 18]

# Phase-1-Evals (VAL-Seeds, Cap 4000, stochastisch), Steps 25k–150k:
STEPS_K   = [25, 50, 75, 100, 125, 150]
B8_STOCH  = [54, 66, 64, 74, 88, 84]        # nackter Lauf, batch=8, Seed 1
B64_BARE  = [74, 44, 66, 24, 50, 48]        # nackter Lauf, batch=64, Seed 1 (A1-Repro)
B64_CURR  = [16, 16, 10, 20, 16, 20]        # Curriculum-Kontrollarm, batch=64, Seed 1


def _style(ax):
    ax.grid(True, color=C_GRID, linewidth=0.7)
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    for s in ("left", "bottom"):
        ax.spines[s].set_color(C_GRAY)


def fig_cap() -> None:
    fig, ax = plt.subplots(figsize=(6.4, 3.4))
    ax.plot(CAPS, SR_CAP_STOCH, color=C_BLUE, linewidth=2, marker="o", markersize=5)
    ax.plot(CAPS, SR_CAP_DET, color=C_GREEN, linewidth=2, marker="o", markersize=5)
    ax.annotate("stochastisch", (CAPS[-1], SR_CAP_STOCH[-1]), xytext=(-4, 8),
                textcoords="offset points", ha="right", color="#3a3935", fontsize=10)
    ax.annotate("deterministisch", (CAPS[-1], SR_CAP_DET[-1]), xytext=(-4, 8),
                textcoords="offset points", ha="right", color="#3a3935", fontsize=10)
    # Die beiden Betriebspunkte des Gates markieren
    ax.scatter([600], [48], s=90, facecolors="none", edgecolors=C_ORANGE,
               linewidths=2, zorder=5)
    ax.annotate("Gate-Messung bis 07.07.\n(Cap 600): 48 %", (600, 48), xytext=(10, -26),
                textcoords="offset points", fontsize=9, color="#3a3935")
    ax.scatter([4000], [86], s=90, facecolors="none", edgecolors=C_ORANGE,
               linewidths=2, zorder=5)
    ax.annotate("volle Episode\n(Cap 4000): 86 %", (4000, 86), xytext=(-8, -30),
                textcoords="offset points", ha="right", fontsize=9, color="#3a3935")
    ax.set_xlabel("Episoden-Cap der Evaluation (Schritte)")
    ax.set_ylabel("Success Rate (%)")
    ax.set_ylim(0, 100)
    ax.set_xlim(400, 4200)
    _style(ax)
    fig.tight_layout()
    out = os.path.join(OUT_DIR, "fig_cap_artefakt.pdf")
    fig.savefig(out)
    print("→", out)


def _ev_series(path: str, limit: int = 155_000):
    """(timesteps, EV)-Paare aus einem SB3-Log; EV → nächstfolgendes total_timesteps."""
    ev, out = None, []
    for line in open(path, errors="ignore"):
        m = re.search(r"explained_variance\s*\|\s*(-?[\d.e+-]+)", line)
        if m:
            ev = float(m.group(1))
            continue
        m = re.search(r"total_timesteps\s*\|\s*(\d+)", line)
        if m and ev is not None:
            t = int(m.group(1))
            if t <= limit:
                out.append((t, ev))
            ev = None
    return out


def fig_batch() -> None:
    root = os.path.join(os.path.dirname(__file__), "..")
    ev8 = _ev_series(os.path.join(root, "logs", "train_bare_batch8_seed1.log"))
    ev64 = _ev_series(os.path.join(root, "logs", "train_v11_seed1.log"))

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(6.6, 5.6), sharex=True,
                                   height_ratios=[3, 2])
    ax1.plot(STEPS_K, B8_STOCH, color=C_BLUE, linewidth=2.2, marker="o", markersize=5)
    ax1.plot(STEPS_K, B64_BARE, color=C_ORANGE, linewidth=2, marker="o", markersize=5)
    ax1.plot(STEPS_K, B64_CURR, color=C_GRAY, linewidth=2, marker="o", markersize=5)
    for y, label, dy in [(B8_STOCH[-1], "batch=8 (nackt)", 6),
                         (B64_BARE[-1], "batch=64 (nackt)", 6),
                         (B64_CURR[-1], "batch=64 (Curriculum)", -14)]:
        ax1.annotate(label, (150, y), xytext=(6, dy), textcoords="offset points",
                     fontsize=9, color="#3a3935")
    ax1.set_ylabel("SR stochastisch (%)")
    ax1.set_ylim(0, 100)
    ax1.set_xlim(20, 195)
    _style(ax1)

    def ema(vals, alpha=0.35):
        out, m = [], vals[0]
        for v in vals:
            m = alpha * v + (1 - alpha) * m
            out.append(m)
        return out

    for series, color in [(ev8, C_BLUE), (ev64, C_GRAY)]:
        xs = [t / 1000 for t, _ in series]
        ys = [e for _, e in series]
        ax2.plot(xs, ys, color=color, linewidth=0.9, alpha=0.3)
        ax2.plot(xs, ema(ys), color=color, linewidth=2)
    ax2.annotate("batch=8", (ev8[-1][0] / 1000, ema([e for _, e in ev8])[-1]),
                 xytext=(6, 0), textcoords="offset points", fontsize=9, color="#3a3935")
    ax2.annotate("batch=64 (Curr.)", (ev64[-1][0] / 1000, ema([e for _, e in ev64])[-1]),
                 xytext=(6, 0), textcoords="offset points", fontsize=9, color="#3a3935")
    ax2.set_xlabel("Trainingsschritte (Tausend)")
    ax2.set_ylabel("explained variance\n(EMA, roh blass)")
    ax2.set_ylim(-0.1, 1.0)
    _style(ax2)

    fig.tight_layout()
    out = os.path.join(OUT_DIR, "fig_batch_vergleich.pdf")
    fig.savefig(out)
    print("→", out)


if __name__ == "__main__":
    fig_cap()
    fig_batch()
