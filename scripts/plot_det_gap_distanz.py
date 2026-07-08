#!/usr/bin/env python3
"""Det/Stoch-Gap in Abhängigkeit von der Weglänge zum Ausgang.

Kernbefund (08.07.2026): Die deterministische Erfolgsrate bricht mit der
Startdistanz ein, während die stochastische hoch bleibt. Bei kurzen Wegen
verschwindet der Gap fast — der Agent IST zielstrebig, solange sein Gedächtnis
den Belief-State über den Weg eindeutig halten kann.

Verwendung:
    python scripts/plot_det_gap_distanz.py \
        [--model models/ppo_lstm_curriculum_v12_s3/best_model.zip] \
        [--seeds 6000 6120] [-o docs/figures/fig_det_gap_distanz.pdf]

Misst auf Validierungs-Seeds (Standard 6000–6119, breite Distanz 5–45),
speichert Rohwerte als JSON neben der Abbildung und plottet gruppierte Balken.
"""
from __future__ import annotations

import argparse
import json
import os

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from sb3_contrib import RecurrentPPO
from stoneforge_env import StoneforgeWorldEnv, env_kwargs_for_model

# Druckfreundliche Farben (konsistent mit plot_run_curve.py)
C_STOCH = "#2a78d6"   # blau — variierende Spielweise
C_DET   = "#1baf7a"   # aqua — sture Spielweise
C_MUTED = "#898781"
C_GRID  = "#e1e0d9"

BINS = [(5, 15, "5–15"), (15, 25, "15–25"), (25, 35, "25–35"), (35, 45, "35–45")]


def which_bin(d: int) -> str:
    for lo, hi, label in BINS:
        if lo <= d < hi or (hi == 45 and d <= 45):
            return label
    return BINS[-1][2]


def measure(model, env, seeds):
    data = {label: {"det": [], "stoch": []} for _, _, label in BINS}
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        d0 = env.core.bfs_distance_at_offset(0, 0)
        b = which_bin(d0)
        for det in (True, False):
            obs, _ = env.reset(seed=seed)
            st = None
            es = np.ones((1,), dtype=bool)
            steps = 0
            reached = False
            done = False
            while not done and steps < 4000:
                a, st = model.predict(obs.reshape(1, -1), state=st,
                                      episode_start=es, deterministic=det)
                es = np.zeros((1,), dtype=bool)
                obs, _, term, trunc, info = env.step(int(a[0]))
                steps += 1
                if info.get("reached_exit"):
                    reached = True
                done = term or trunc
            data[b]["det" if det else "stoch"].append(int(reached))
    return data


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", default="models/ppo_lstm_curriculum_v12_s3/best_model.zip")
    ap.add_argument("--seeds", type=int, nargs=2, default=[6000, 6120],
                    help="Seed-Bereich [start, ende) — Validierungs-Seeds")
    ap.add_argument("-o", "--out", default="docs/figures/fig_det_gap_distanz.pdf")
    args = ap.parse_args()

    model = RecurrentPPO.load(args.model, device="cpu")
    env = StoneforgeWorldEnv(exit_min=5, exit_max=45, **env_kwargs_for_model(model))
    seeds = range(args.seeds[0], args.seeds[1])
    print(f"Messe {len(seeds)} Seeds (det + stoch) …")
    data = measure(model, env, seeds)

    labels = [lab for _, _, lab in BINS]
    det_sr = [100 * np.mean(data[l]["det"]) if data[l]["det"] else 0 for l in labels]
    sto_sr = [100 * np.mean(data[l]["stoch"]) if data[l]["stoch"] else 0 for l in labels]
    ns = [len(data[l]["det"]) for l in labels]

    print("\nStartdistanz | n | det-SR | stoch-SR")
    for l, d, s, n in zip(labels, det_sr, sto_sr, ns):
        print(f"  {l:6s}    | {n:2d} | {d:5.1f}% | {s:5.1f}%")

    # --- Plot: gruppierte Balken ---
    fig, ax = plt.subplots(figsize=(7.2, 4.2), dpi=150)
    x = np.arange(len(labels))
    w = 0.38
    b1 = ax.bar(x - w / 2, sto_sr, w, label="stochastisch (variierend)", color=C_STOCH)
    b2 = ax.bar(x + w / 2, det_sr, w, label="deterministisch (stur)", color=C_DET)
    for bars in (b1, b2):
        for r in bars:
            ax.text(r.get_x() + r.get_width() / 2, r.get_height() + 1.5,
                    f"{r.get_height():.0f}", ha="center", va="bottom", fontsize=9)

    ax.set_ylim(0, 108)
    ax.set_ylabel("Erfolgsrate (%)")
    ax.set_xlabel("Weglänge zum Ausgang (BFS-Distanz, Felder)")
    ax.set_xticks(x)
    ax.set_xticklabels([f"{l}\n(n={n})" for l, n in zip(labels, ns)])
    ax.set_title("Det/Stoch-Gap skaliert mit der Weglänge\n"
                 "bei kurzen Wegen ist der Agent zielstrebig, bei langen nur stochastisch",
                 fontsize=11)
    ax.grid(axis="y", color=C_GRID, linewidth=0.8, zorder=0)
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    ax.legend(frameon=False, fontsize=9, loc="lower left")
    fig.tight_layout()

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    fig.savefig(args.out)
    png = os.path.splitext(args.out)[0] + ".png"
    fig.savefig(png, dpi=150)
    js = os.path.splitext(args.out)[0] + ".json"
    json.dump({"model": args.model, "seeds": list(args.seeds),
               "labels": labels, "n": ns, "det_sr": det_sr, "stoch_sr": sto_sr},
              open(js, "w"), indent=2)
    print(f"\nGespeichert: {args.out} · {png} · {js}")


if __name__ == "__main__":
    main()
