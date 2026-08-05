"""Erzeugt docs/figures/fig_v12_endergebnis.pdf -- zentrales Endergebnis (v12, n=7).

Bislang existierte fuer die Tabelle tab:v12 (Abschnitt sec:endergebnis) keine
Abbildung; die dort ausfuehrlich diskutierte Streuung von rund +-12 Punkten
war nur als Zahl sichtbar, nicht als Fehlerbalken. Diese Abbildung zeigt Mittel
+- Stichproben-Std (ddof=1) je Bedingung, die sieben Einzellaeufe als
Streupunkte und die vorab festgelegten Erfolgsschwellen aus Abschnitt sec:ziele.

Rohdaten identisch zu Tabelle tab:v12 in der Dokumentation.
"""
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

OUT = "docs/figures/fig_v12_endergebnis.pdf"

# Rohdaten der sieben v12-Laeufe (Tabelle tab:v12)
a_stoch = np.array([62, 84, 76, 74, 58, 50, 56])
a_det   = np.array([38, 26, 32, 40, 20, 20, 28])
b_stoch = np.array([76, 76, 80, 74, 62, 50, 50])
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
        ax.annotate(f"{mean:.1f}", (x, mean + std), textcoords="offset points",
                     xytext=(0, 5), ha="center", fontsize=9.5, fontweight="bold")

    # vorab festgelegte Erfolgsschwelle dieser Testmenge
    ax.hlines(threshold, xc - bar_w - 0.12, xc + bar_w + 0.12, color="#2E2E2E",
               linestyle="--", linewidth=1.3, zorder=5)
    ax.annotate(f"Kriterium {threshold} %", (xc + bar_w + 0.14, threshold),
                 va="center", fontsize=8.5, color="#2E2E2E")

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
