"""Erzeugt docs/figures/fig_entwicklung.pdf — Entwicklung ueber die Modellgenerationen.

Wichtig: Die Balken stammen aus ZWEI verschiedenen Eval-Protokollen. Die
Mai-Messungen liefen auf der Umgebung vor v11 unter einem Kurzdistanz-Protokoll,
die Juni-Messungen unter dem Standardprotokoll (Exit 35-45). Die Abbildung
trennt beide Gruppen deshalb sichtbar; eine fruehere Fassung stellte alle
Balken unter eine gemeinsame Bildunterschrift "Exit 35-45", was den Vergleich
mit Tabelle "ablation" (ppo_phase4: 32 % stoch) unaufloesbar machte.
"""
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

OUT = "docs/figures/fig_entwicklung.pdf"

labels = [
    "Phase 3\n(16.05., MLP+BFS)",
    "Phase 4\n(16.05., MLP+BFS)",
    "Delta-BFS\n(18.05., MLP+BFS)",
    "Phase 4 nachgemessen\n(MLP+BFS)",
    "LSTM-Curriculum\n(08.06., ohne BFS)",
]
stoch = [86, 98, 100, 32, 86]
det = [2, 2, 42, 0, 36]
split = 3   # ab hier: Protokoll Exit 35-45

x = np.arange(len(labels), dtype=float)
x[split:] += 0.6        # Luecke zwischen den Protokollgruppen
w = 0.38

fig, ax = plt.subplots(figsize=(9.2, 4.3))
b1 = ax.bar(x - w / 2, stoch, w, label="stochastisch", color="#4C72B0")
b2 = ax.bar(x + w / 2, det, w, label="deterministisch (Argmax)", color="#DD8452")

for bars in (b1, b2):
    for r in bars:
        ax.annotate(f"{int(r.get_height())}", (r.get_x() + r.get_width() / 2, r.get_height()),
                    textcoords="offset points", xytext=(0, 3), ha="center", fontsize=9)

sep = (x[split - 1] + x[split]) / 2
ax.axvline(sep, color="0.55", linestyle="--", linewidth=1)

ax.text((x[0] + x[split - 1]) / 2, 116, "Kurzdistanz-Protokoll (Umgebung vor v11)",
        ha="center", fontsize=9.5, style="italic", color="0.25")
ax.text((x[split] + x[-1]) / 2, 116, "Protokoll Exit 35–45",
        ha="center", fontsize=9.5, style="italic", color="0.25")

ax.set_ylabel("Success Rate (%)")
ax.set_title("Testset A — Werte nur innerhalb einer Protokollgruppe vergleichbar", fontsize=10.5)
ax.set_xticks(x)
ax.set_xticklabels(labels, fontsize=8.5)
ax.set_ylim(0, 126)
ax.set_yticks([0, 25, 50, 75, 100])
ax.legend(loc="upper center", bbox_to_anchor=(0.5, -0.16), ncol=2,
          fontsize=9, frameon=False)
ax.spines[["top", "right"]].set_visible(False)
ax.grid(axis="y", alpha=0.25)
fig.tight_layout()
fig.savefig(OUT)
print("geschrieben:", OUT)
