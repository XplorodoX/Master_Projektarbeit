"""Erzeugt das Zielkriterium-Endergebnis (v12, n=7) fuer Kapitel 6.

Zeigt Mittel +- Stichproben-Std (ddof=1) je Bedingung, die sieben Einzellaeufe
als Streupunkte und die vorab festgelegten Erfolgsschwellen aus dem Ziel-
kriterium (70 % / 60 %). Das Kriterium gilt laut Projektdokumentation nur fuer
den stochastischen Modus (kanonisches Protokoll misst fuer Testset A/B
ausschliesslich stochastisch); die Schwellenlinie ueberspannt deshalb nur den
stochastischen Balken, nicht den nachrichtlich mitgezeigten deterministischen.

Rohdaten identisch zu den Tabellen in Anhang C der Dokumentation.
"""
import json

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

OUT = "docs/Doku/Bilder/v12_zielkriterium.png"

# Stochastik: kanonische Messdatei (5-Wiederholungs-Protokoll), damit die
# Abbildung dieselbe Quelle nutzt wie Fliesstext und Anhang C.
with open("logs/eval_results/baselines.json") as fh:
    _DATA = json.load(fh)
a_stoch = np.array([_DATA[f"v12_s{i}_stoch"]["A"]["sr_mean"] for i in range(1, 8)])
b_stoch = np.array([_DATA[f"v12_s{i}_stoch"]["B"]["sr_mean"] for i in range(1, 8)])
# Deterministisch: einmalige Zusatzmessung auf den Testset-Seeds
# (CHANGELOG v2026-07-17), im kanonischen Protokoll nicht enthalten.
a_det   = np.array([38, 26, 32, 40, 20, 20, 28])
b_det   = np.array([46, 44, 36, 38, 34, 14, 16])

groups = [
    ("Testset A", a_stoch, a_det, 70),
    ("Holdout B", b_stoch, b_det, 60),
]

C_STOCH = "#4C72B0"
C_DET = "#DD8452"

fig, ax = plt.subplots(figsize=(7.4, 4.6))

bar_w = 0.32
centers = np.array([0, 1.1])
rng = np.random.default_rng(0)

for gi, (name, stoch, det, threshold) in enumerate(groups):
    xc = centers[gi]
    for off, data, color, label in (
        (-bar_w / 2 - 0.02, stoch, C_STOCH, "stochastisch"),
        (bar_w / 2 + 0.02, det, C_DET, "deterministisch (Argmax)"),
    ):
        mean = data.mean()
        std = data.std(ddof=1)
        x = xc + off
        ax.bar(x, mean, bar_w, color=color, edgecolor="black", linewidth=0.6,
               zorder=2, label=label if gi == 0 else None)
        ax.errorbar(x, mean, yerr=std, fmt="none", ecolor="black",
                     elinewidth=1.3, capsize=5, capthick=1.3, zorder=3)
        jitter = rng.uniform(-0.055, 0.055, size=len(data))
        ax.scatter(np.full(len(data), x) + jitter, data, s=16, facecolor="white",
                    edgecolor="black", linewidth=0.6, zorder=4, alpha=0.9)
        ax.annotate(f"{mean:.1f}".replace(".", ","), (x, mean + std), textcoords="offset points",
                     xytext=(0, 5), ha="center", fontsize=9.5, fontweight="bold")

    # vorab festgelegte Erfolgsschwelle dieser Testmenge
    stoch_x = xc - bar_w / 2 - 0.02
    ax.hlines(threshold, stoch_x - bar_w / 2 - 0.03, stoch_x + bar_w / 2 + 0.03,
               color="#2E2E2E", linestyle="--", linewidth=1.3, zorder=5)
    ax.annotate(f"Kriterium {threshold} % (stoch.)",
                 (stoch_x - bar_w / 2 - 0.04, threshold),
                 textcoords="offset points", xytext=(-4, 0), ha="right",
                 va="center", fontsize=8, color="#2E2E2E", zorder=6)

ax.set_xticks(centers)
ax.set_xticklabels([g[0] for g in groups], fontsize=11)
ax.set_ylabel("Success Rate (%)", fontsize=11)
ax.set_ylim(0, 100)
ax.tick_params(axis="y", labelsize=10)
ax.set_title("v12-Endergebnis: Mittel ± Stichproben-Std ($n=7$), Einzelläufe als Punkte",
             fontsize=11)
ax.legend(loc="upper center", bbox_to_anchor=(0.5, -0.13), ncol=2, fontsize=9.5,
          frameon=False)
ax.spines[["top", "right"]].set_visible(False)
ax.grid(axis="y", alpha=0.25, zorder=0)
fig.tight_layout()
fig.savefig(OUT)
print("geschrieben:", OUT)
