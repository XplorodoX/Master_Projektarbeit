#!/usr/bin/env python3
"""Lernkurve (det/stoch-SR pro Eval-Checkpoint) aus eval_history.json plotten.

Verwendung:
    python scripts/plot_run_curve.py models/ppo_lstm_curriculum_v11 \
        [-o docs/figures/fig_v11_lernkurve.pdf] [--title "..."]

Liest models/<run>/eval_history.json; det/stoch werden aus dem Label geparst
("... [det=X%|stoch=Y%]"), Phasengrenzen aus Label-Wechseln.
"""
from __future__ import annotations

import argparse
import json
import os
import re

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# Druckfreundliche Farben (hell, CVD-validiert)
C_STOCH = "#2a78d6"   # blau
C_DET   = "#1baf7a"   # aqua
C_MUTED = "#898781"
C_GRID  = "#e1e0d9"

LABEL_RE = re.compile(r"\[det=(\d+)%\|stoch=(\d+)%\]")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("run_dir", help="Modellordner mit eval_history.json")
    ap.add_argument("-o", "--out", default=None, help="Ausgabedatei (.pdf/.png)")
    ap.add_argument("--title", default=None)
    args = ap.parse_args()

    hist = json.load(open(os.path.join(args.run_dir, "eval_history.json")))
    steps, det, stoch, phases = [], [], [], []
    prev_phase, prev_raw, x = None, None, 0.0
    for e in hist:
        m = LABEL_RE.search(e.get("label", ""))
        if not m:
            continue
        # Beim Phasenwechsel wird das Bestmodell der Vorphase geladen — dessen
        # SB3-Stepzähler springt zurück. Kumulierte Trainingssteps rekonstruieren:
        raw = e["step"]
        if prev_raw is None:
            x = raw
        else:
            d = raw - prev_raw
            x += d if d > 0 else 25_000   # Reload → Eval-Intervall annehmen
        prev_raw = raw
        steps.append(x / 1e6)
        det.append(int(m.group(1)))
        stoch.append(int(m.group(2)))
        phase = e["label"].split(" [")[0]
        if phase != prev_phase:
            phases.append((x / 1e6, phase))
            prev_phase = phase

    if not steps:
        raise SystemExit("Keine det/stoch-Labels in eval_history.json gefunden "
                         "(Lauf von vor dem 07.07.2026?)")

    fig, ax = plt.subplots(figsize=(7.2, 3.6))
    ax.plot(steps, stoch, color=C_STOCH, lw=1.8, label="stochastisch")
    ax.plot(steps, det, color=C_DET, lw=1.8, label="deterministisch")

    for i, (x, phase) in enumerate(phases):
        if i > 0:
            ax.axvline(x, color=C_MUTED, lw=0.8, alpha=0.7)
        short = phase.replace("Phase ", "P").split(" (")[0]
        ax.text(x + 0.012, 97, short, fontsize=8, color=C_MUTED, va="top")

    ax.set_xlabel("Timesteps (Mio.)")
    ax.set_ylabel("Success Rate (%)")
    ax.set_ylim(0, 100)
    ax.set_xlim(left=0)
    if args.title:
        ax.set_title(args.title, fontsize=10)
    ax.grid(color=C_GRID, lw=0.6)
    ax.set_axisbelow(True)
    for spine in ("top", "right"):
        ax.spines[spine].set_visible(False)
    for spine in ("left", "bottom"):
        ax.spines[spine].set_color(C_MUTED)
    ax.tick_params(colors="#52514e", labelsize=9)
    ax.legend(frameon=False, fontsize=9, loc="upper right")

    out = args.out or os.path.join(args.run_dir, "lernkurve.pdf")
    fig.tight_layout()
    fig.savefig(out, bbox_inches="tight")
    print(f"→ {out}  ({len(steps)} Checkpoints, {len(phases)} Phasen)")


if __name__ == "__main__":
    main()
