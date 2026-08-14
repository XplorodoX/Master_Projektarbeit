"""Erzeugt die vier Evaluations-Abbildungen für Kapitel 5 aus den berichtsfähigen Zahlen.

Datenquellen (siehe CHANGELOG v2026-08-14.1 und CLAUDE.md, Berichtsfähiger Stand):
  * Random / Kompass-Referenzen sowie LSTM- und MLP-Stochastik (je Testset A,
    n=7): logs/eval_results/baselines.json (kanonisch, 5-Wiederholungs-Protokoll).
    Beide Modellreihen stammen damit aus derselben Messkampagne.
  * LSTM deterministisch: einmalige Zusatzmessung auf den Testset-Seeds
    (CHANGELOG v2026-07-17); im kanonischen Protokoll nicht enthalten.

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
# Alles Stochastische kommt direkt aus der kanonischen Messdatei, damit
# Abbildungen und Fliesstext nicht auseinanderlaufen koennen.
import json

_DATA = json.loads(Path("logs/eval_results/baselines.json").read_text())

RANDOM_SR = _DATA["random"]["A"]["sr_mean"]
RANDOM_EFF = _DATA["random"]["A"]["efficiency"]
_EPS = (0.3, 0.4, 0.5, 0.6, 0.8, 0.9)
COMPASS_SR = {e: _DATA[f"compass_eps{e}"]["A"]["sr_mean"] for e in _EPS}
COMPASS_EFF = {e: _DATA[f"compass_eps{e}"]["A"]["efficiency"] for e in _EPS}

# LSTM / MLP, Testset A, n=7, 5-Wiederholungs-Protokoll (baselines.json)
LSTM_STOCH = np.array([_DATA[f"v12_s{i}_stoch"]["A"]["sr_mean"] for i in range(1, 8)])
LSTM_EFF = float(np.mean([_DATA[f"v12_s{i}_stoch"]["A"]["efficiency"] for i in range(1, 8)]))
MLP_STOCH = np.array([_DATA[f"v12_mlp_s{i}_stoch"]["A"]["sr_mean"] for i in range(1, 8)])
MLP_EFF = float(np.mean([_DATA[f"v12_mlp_s{i}_stoch"]["A"]["efficiency"] for i in range(1, 8)]))

# LSTM deterministisch: einmalige Zusatzmessung auf den Testset-Seeds
# (CHANGELOG v2026-07-17), nicht Teil des 5-Wiederholungs-Protokolls.
# Deterministische MLP-Werte existieren nicht und werden nicht geplottet.
LSTM_DET = np.array([38, 26, 32, 40, 20, 20, 28], dtype=float)

assert abs(LSTM_STOCH.mean() - 64.6) < 0.1
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
    labels = ["Random", "MLP-PPO\nstoch.", "LSTM-PPO\ndet.",
              "LSTM-PPO\nstoch.", "Kompass\nε=0,3", "Kompass\nε=0,5", "Kompass\nε=0,6",
              "Kompass\nε=0,8", "Kompass\nε=0,9"]
    values = [RANDOM_SR, MLP_STOCH.mean(), LSTM_DET.mean(),
              LSTM_STOCH.mean(), COMPASS_SR[0.3], COMPASS_SR[0.5], COMPASS_SR[0.6],
              COMPASS_SR[0.8], COMPASS_SR[0.9]]
    errs = [0, MLP_STOCH.std(ddof=1), LSTM_DET.std(ddof=1),
            LSTM_STOCH.std(ddof=1), 0, 0, 0, 0, 0]
    colors = [C_RANDOM, C_MLP, C_LSTM_LIGHT, C_LSTM,
              C_COMPASS[0.3], C_COMPASS[0.5], C_COMPASS[0.6], C_COMPASS[0.8], C_COMPASS[0.9]]

    fig, ax = plt.subplots(figsize=(16, 7.5))
    bars = ax.bar(labels, values, yerr=errs, color=colors, edgecolor="#333333",
                   linewidth=1.1, capsize=5, error_kw={"linewidth": 1.6, "ecolor": "#333333"},
                   width=0.72)
    for bar, val, err in zip(bars, values, errs):
        ax.text(bar.get_x() + bar.get_width() / 2, val + err + 2.5, f"{val:.1f} %".replace(".", ","),
                ha="center", va="bottom", fontsize=13)
    ax.set_ylabel("Erfolgsquote (%)")
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
    fig, ax = plt.subplots(figsize=(12, 7.5))
    bars = ax.bar(labels, values, color=colors, edgecolor="#333333", linewidth=1.1)
    for bar, val in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width() / 2, val + 0.003, f"{val:.3f}".replace(".", ","),
                ha="center", va="bottom", fontsize=13)
    ax.set_ylabel("Wegverhältnis (BFS-Optimum / gelaufene Schritte)")
    ax.set_ylim(0, 0.20)
    ax.set_title("Wegverhältnis derselben Verfahren")
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

    # LSTM: Mittelwert-Diamant (keine Pro-Seed-Effizienz im kanonischen Lauf
    # dokumentiert, daher keine Einzelpunkte)
    ax.scatter([LSTM_STOCH.mean()], [LSTM_EFF], marker="D", s=320, color=C_LSTM,
               edgecolor="#333333", linewidth=2, zorder=4)
    ax.annotate("LSTM-PPO\n(Mittel über 7 Läufe)", (LSTM_STOCH.mean(), LSTM_EFF),
                xytext=(15, -55), textcoords="offset points", fontsize=14,
                color=C_LSTM, fontweight="bold")

    # MLP: Mittelwert-Diamant (siehe LSTM-Kommentar)
    ax.scatter([MLP_STOCH.mean()], [MLP_EFF], marker="D", s=320, color=C_MLP,
               edgecolor="#333333", linewidth=2, zorder=4)
    ax.annotate("MLP-PPO\n(Mittel über 7 Läufe)", (MLP_STOCH.mean(), MLP_EFF),
                xytext=(-205, 15), textcoords="offset points", fontsize=14,
                color="#6A6A5A", fontweight="bold")

    ax.set_xlabel("Erfolgsquote auf Testset A (%)")
    ax.set_ylabel("Wegverhältnis")
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 0.19)
    ax.set_title("Zielkonflikt zwischen Erfolgsquote und Wegverhältnis")
    savefig(fig, "eval_zielkonflikt.png")


# ---------------------------------------------------------------- Abbildung 4: Determinismus-Lücke
def plot_determinismus():
    fig, ax = plt.subplots(figsize=(11, 8.5))
    x = [0, 1]
    for det, stoch in zip(LSTM_DET, LSTM_STOCH):
        ax.plot(x, [det, stoch], "-", color=C_LSTM, linewidth=1.8, alpha=0.85, zorder=2)
        ax.scatter(x, [det, stoch], s=70, facecolor="white", edgecolor=C_LSTM, linewidth=2, zorder=3)
    ax.plot([], [], "-o", color=C_LSTM, markerfacecolor="white", label="LSTM-PPO (7 Läufe)")
    ax.legend(loc="upper left", frameon=False, fontsize=14)

    ax.set_xticks(x)
    ax.set_xticklabels(["deterministisch\n(Argmax)", "stochastisch\n(Sampling)"])
    ax.set_ylabel("Erfolgsquote (%)")
    ax.set_ylim(0, 100)
    ax.set_xlim(-0.15, 1.15)
    ax.set_title("Determinismus-Lücke des LSTM-PPO: dasselbe Modell, zwei Auswertungsmodi")
    savefig(fig, "eval_det_stoch.png")


if __name__ == "__main__":
    plot_erfolgsquote()
    plot_pfadeffizienz()
    plot_zielkonflikt()
    plot_determinismus()
