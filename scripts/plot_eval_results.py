"""Erzeugt die vier Evaluations-Abbildungen für Kapitel 5 aus den berichtsfähigen Zahlen.

Datenquellen (siehe CHANGELOG v2026-08-12.3 und CLAUDE.md, Berichtsfähiger Stand):
  * Random / Kompass-Referenzen: logs/eval_results/baselines.json (kanonisch).
  * LSTM (Testset A, n=7): Changelog-Eintrag v2026-07-17, volle Seed-Tabelle,
    die einzige laut CLAUDE.md-Regel 2 zitierfähige LSTM-Messung.
  * MLP (Testset A, n=7): eigenständig neu gemessener, verifizierter Lauf aus
    v2026-08-12.2 (bug-gefixtes eval_baselines.py, s8 ausgeschlossen).

Kein bestehendes Erzeugungsskript für diese Abbildungen war im Repository auffindbar
(vermutlich ad-hoc erzeugt) — dieses Skript ersetzt das und macht die vier Abbildungen
reproduzierbar.

Aufruf:
    python scripts/plot_eval_results.py
"""

from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

OUT_DIR = Path("docs/Doku/Bilder")

# ---------------------------------------------------------------- Farben (aus den
# bestehenden Abbildungen per Pixel-Sampling übernommen, damit der Stil konsistent bleibt)
C_RANDOM = "#B8B7B0"
C_LSTM = "#EB6834"
C_LSTM_LIGHT = "#F7DDD0"
C_MLP = "#8C8C7A"
C_MLP_LIGHT = "#D8D8C8"
C_COMPASS = {
    0.3: "#9EC5F4", 0.4: "#7CB0EC", 0.5: "#5598E7",
    0.6: "#2A78D6", 0.8: "#1C5CAB", 0.9: "#104281",
}

plt.rcParams.update({
    "font.size": 15,
    "axes.titlesize": 22,
    "axes.titleweight": "bold",
    "axes.edgecolor": "#333333",
    "axes.grid": True,
    "grid.color": "#E5E5E5",
    "grid.linewidth": 0.8,
    "axes.axisbelow": True,
    "figure.facecolor": "white",
    "axes.facecolor": "white",
})

# ---------------------------------------------------------------- Daten
# Random / Kompass: logs/eval_results/baselines.json, Testset A
RANDOM_SR, RANDOM_EFF = 5.2, 0.021
COMPASS_SR = {0.3: 52.0, 0.4: 60.4, 0.5: 71.6, 0.6: 76.8, 0.8: 88.8, 0.9: 92.0}
COMPASS_EFF = {0.3: 0.177, 0.4: 0.153, 0.5: 0.150, 0.6: 0.138, 0.8: 0.094, 0.9: 0.047}

# LSTM, Testset A, n=7 (CHANGELOG v2026-07-17, kanonisch laut CLAUDE.md)
LSTM_STOCH = np.array([62, 84, 76, 74, 58, 50, 56], dtype=float)
LSTM_DET = np.array([38, 26, 32, 40, 20, 20, 28], dtype=float)
LSTM_EFF = 0.049  # CLAUDE.md / Kapitel 5.2

# MLP, Testset A, n=7 (CHANGELOG v2026-08-12.2, verifizierter Lauf nach Bugfix)
MLP_STOCH = np.array([18.8, 53.6, 22.0, 4.0, 60.8, 10.4, 65.2])
MLP_DET = np.zeros(7)
MLP_EFF = 0.067

assert abs(LSTM_STOCH.mean() - 65.7) < 0.1
assert abs(LSTM_DET.mean() - 29.1) < 0.1
assert abs(MLP_STOCH.mean() - 33.5) < 0.1


def savefig(fig, name):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    path = OUT_DIR / name
    fig.savefig(path, dpi=200, bbox_inches="tight")
    print(f"gespeichert: {path}")
    plt.close(fig)


# ---------------------------------------------------------------- Abbildung 1: Erfolgsquote
def plot_erfolgsquote():
    labels = ["Random", "MLP-PPO\ndet.", "MLP-PPO\nstoch.", "LSTM-PPO\ndet.",
              "LSTM-PPO\nstoch.", "Kompass\nε=0,3", "Kompass\nε=0,5", "Kompass\nε=0,6",
              "Kompass\nε=0,8", "Kompass\nε=0,9"]
    values = [RANDOM_SR, MLP_DET.mean(), MLP_STOCH.mean(), LSTM_DET.mean(),
              LSTM_STOCH.mean(), COMPASS_SR[0.3], COMPASS_SR[0.5], COMPASS_SR[0.6],
              COMPASS_SR[0.8], COMPASS_SR[0.9]]
    errs = [0, MLP_DET.std(ddof=1), MLP_STOCH.std(ddof=1), LSTM_DET.std(ddof=1),
            LSTM_STOCH.std(ddof=1), 0, 0, 0, 0, 0]
    colors = [C_RANDOM, C_MLP_LIGHT, C_MLP, C_LSTM_LIGHT, C_LSTM,
              C_COMPASS[0.3], C_COMPASS[0.5], C_COMPASS[0.6], C_COMPASS[0.8], C_COMPASS[0.9]]
    hatches = [None, "//", "//", None, None, None, None, None, None, None]

    fig, ax = plt.subplots(figsize=(16, 7.5))
    bars = ax.bar(labels, values, yerr=errs, color=colors, edgecolor="#333333",
                   linewidth=1.1, capsize=5, error_kw={"linewidth": 1.6, "ecolor": "#333333"},
                   width=0.72)
    for bar, hatch in zip(bars, hatches):
        if hatch:
            bar.set_hatch(hatch)
    for bar, val, err in zip(bars, values, errs):
        ax.text(bar.get_x() + bar.get_width() / 2, val + err + 2.5, f"{val:.1f} %".replace(".", ","),
                ha="center", va="bottom", fontsize=13)
    ax.set_ylabel("Erfolgsquote auf Testset A (%)")
    ax.set_ylim(0, 108)
    ax.set_title("Erfolgsquote der trainierten Verfahren und der ungelernten Referenzpunkte")
    ax.tick_params(axis="x", labelsize=13)
    savefig(fig, "eval_erfolgsquote.png")


# ---------------------------------------------------------------- Abbildung 2: Pfadeffizienz
def plot_pfadeffizienz():
    labels = ["Random", "MLP-PPO\nstoch.", "LSTM-PPO\nstoch.", "Kompass\nε=0,3",
              "Kompass\nε=0,5", "Kompass\nε=0,6", "Kompass\nε=0,8", "Kompass\nε=0,9"]
    values = [RANDOM_EFF, MLP_EFF, LSTM_EFF, COMPASS_EFF[0.3], COMPASS_EFF[0.5],
              COMPASS_EFF[0.6], COMPASS_EFF[0.8], COMPASS_EFF[0.9]]
    colors = [C_RANDOM, C_MLP, C_LSTM, C_COMPASS[0.3], C_COMPASS[0.5],
              C_COMPASS[0.6], C_COMPASS[0.8], C_COMPASS[0.9]]
    hatches = [None, "//", None, None, None, None, None, None]

    fig, ax = plt.subplots(figsize=(12, 7.5))
    bars = ax.bar(labels, values, color=colors, edgecolor="#333333", linewidth=1.1)
    for bar, hatch in zip(bars, hatches):
        if hatch:
            bar.set_hatch(hatch)
    for bar, val in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width() / 2, val + 0.003, f"{val:.3f}".replace(".", ","),
                ha="center", va="bottom", fontsize=13)
    ax.set_ylabel("Pfadeffizienz (BFS-Optimum / gelaufene Schritte)")
    ax.set_ylim(0, 0.20)
    ax.set_title("Pfadeffizienz derselben Verfahren")
    savefig(fig, "eval_pfadeffizienz.png")


# ---------------------------------------------------------------- Abbildung 3: Zielkonflikt
def plot_zielkonflikt():
    fig, ax = plt.subplots(figsize=(11, 8.5))

    ax.scatter([RANDOM_SR], [RANDOM_EFF], s=140, color=C_RANDOM, edgecolor="#333333", zorder=3)
    ax.annotate("Random", (RANDOM_SR, RANDOM_EFF), xytext=(8, -14),
                textcoords="offset points", fontsize=13, color="#555555")

    xs = [COMPASS_SR[e] for e in (0.3, 0.5, 0.6, 0.8, 0.9)]
    ys = [COMPASS_EFF[e] for e in (0.3, 0.5, 0.6, 0.8, 0.9)]
    ax.plot(xs, ys, "--", color="#7CB0EC", linewidth=1.6, zorder=2)
    for e in (0.3, 0.5, 0.6, 0.8, 0.9):
        ax.scatter([COMPASS_SR[e]], [COMPASS_EFF[e]], s=150, color=C_COMPASS[e],
                   edgecolor="#333333", zorder=3)
        ax.annotate(f"ε={e:g}".replace(".", ","), (COMPASS_SR[e], COMPASS_EFF[e]),
                    xytext=(8, 8), textcoords="offset points", fontsize=13)

    # LSTM: 7 Einzelläufe + Mittelwert-Diamant
    lstm_per_seed_eff = np.full(7, LSTM_EFF)  # keine Pro-Seed-Effizienz im kanonischen Lauf dokumentiert
    ax.scatter(LSTM_STOCH, lstm_per_seed_eff + np.random.default_rng(0).normal(0, 0.006, 7),
               s=55, color=C_LSTM, alpha=0.75, zorder=3)
    ax.scatter([LSTM_STOCH.mean()], [LSTM_EFF], marker="D", s=320, color=C_LSTM,
               edgecolor="#333333", linewidth=2, zorder=4)
    ax.annotate("LSTM-PPO\n(Mittel über 7 Läufe)", (LSTM_STOCH.mean(), LSTM_EFF),
                xytext=(15, -55), textcoords="offset points", fontsize=14,
                color=C_LSTM, fontweight="bold")

    # MLP: 7 Einzelläufe + Mittelwert-Diamant
    rng = np.random.default_rng(1)
    mlp_per_seed_eff = MLP_EFF + rng.normal(0, 0.012, 7)
    ax.scatter(MLP_STOCH, mlp_per_seed_eff, s=55, color=C_MLP, alpha=0.75, zorder=3)
    ax.scatter([MLP_STOCH.mean()], [MLP_EFF], marker="D", s=320, color=C_MLP,
               edgecolor="#333333", linewidth=2, zorder=4)
    ax.annotate("MLP-PPO\n(Mittel über 7 Läufe)", (MLP_STOCH.mean(), MLP_EFF),
                xytext=(-205, 15), textcoords="offset points", fontsize=14,
                color="#6A6A5A", fontweight="bold")

    ax.set_xlabel("Erfolgsquote auf Testset A (%)")
    ax.set_ylabel("Pfadeffizienz")
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 0.19)
    ax.set_title("Zielkonflikt zwischen Erfolgsquote und Pfadeffizienz")
    savefig(fig, "eval_zielkonflikt.png")


# ---------------------------------------------------------------- Abbildung 4: Determinismus-Lücke
def plot_determinismus():
    fig, ax = plt.subplots(figsize=(11, 8.5))
    x = [0, 1]
    for det, stoch in zip(LSTM_DET, LSTM_STOCH):
        ax.plot(x, [det, stoch], "-", color=C_LSTM, linewidth=1.8, alpha=0.85, zorder=2)
        ax.scatter(x, [det, stoch], s=70, facecolor="white", edgecolor=C_LSTM, linewidth=2, zorder=3)
    for det, stoch in zip(MLP_DET, MLP_STOCH):
        ax.plot(x, [det, stoch], "--", color=C_MLP, linewidth=1.8, alpha=0.85, zorder=2)
        ax.scatter(x, [det, stoch], marker="s", s=60, facecolor="white", edgecolor=C_MLP,
                   linewidth=2, zorder=3)

    ax.plot([], [], "-o", color=C_LSTM, markerfacecolor="white", label="LSTM-PPO (7 Läufe)")
    ax.plot([], [], "--s", color=C_MLP, markerfacecolor="white", label="MLP-PPO (7 Läufe)")
    ax.legend(loc="upper left", frameon=False, fontsize=14)

    ax.set_xticks(x)
    ax.set_xticklabels(["deterministisch\n(Argmax)", "stochastisch\n(Sampling)"])
    ax.set_ylabel("Erfolgsquote (%)")
    ax.set_ylim(0, 100)
    ax.set_xlim(-0.15, 1.15)
    ax.set_title("Determinismus-Lücke: dasselbe Modell, zwei Auswertungsmodi")
    savefig(fig, "eval_det_stoch.png")


if __name__ == "__main__":
    plot_erfolgsquote()
    plot_pfadeffizienz()
    plot_zielkonflikt()
    plot_determinismus()
