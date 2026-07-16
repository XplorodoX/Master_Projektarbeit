---
id: v12-final
title: v12 — das Ausgabemodell der Arbeit
type: experiment
tags: [ergebnis, final, v12]
path: models/ppo_lstm_curriculum_v12_s{1,2,3}
related: [zielkriterium, eval-protokoll, batch-size-8, det-stoch-gap, projekt-status, henderson-2018]
status: bestätigt
updated: 2026-07-17
---

# v12 — Endergebnis

Drei Läufe (`models/ppo_lstm_curriculum_v12_s{1,2,3}`), Seeds 1–3, `batch_size=8`, alle vier
Phasen über Nacht komplett durchgelaufen (7,7–8,5 h, kein Absturz). CHANGELOG `v2026-07-08`.

## Zahlen — Vorsicht, es gibt zwei Sätze

**Wie am 08.07. gemessen und in der Doku ausformuliert (n=3, Tab. `tab:v12`):**

| Testset | stoch | det |
|---------|-------|-----|
| A (7000–7049) | **73,3 % ± 6,8** (Ziel ≥70 % ✓) | 32,0 % ± 4,9 |
| B (8000–8049) | **80,0 % ± 6,5** (Ziel ≥60 % ✓) | 42,0 % ± 4,3 |

Einzelläufe — A stoch: 64 / 76 / 80 · A det: 38 / 26 / 32 · B stoch: 72 / 80 / 88 · B det: 46 / 44 / 36.

⚠️ Diese ±Werte sind **`ddof=0`** (nachgerechnet 17.07.2026). Mit `ddof=1` — der bei n=3 korrekten
Stichproben-Variante — wären es A ± 8,3 / B ± 8,0. Siehe [[eval-protokoll]].

**Nachmessung 15.07. gegen Env v11 (dieselben Modelle, unabhängig reproduziert, hier `ddof=1`):**
A 72,7 % ± 9,5 stoch / 32,0 % ± 6,0 det · B 77,3 % ± 6,1 stoch / 40,0 % ± 5,3 det.
A det war **exakt identisch** (38/26/32), stoch im Rauschen. Offene Kleinigkeit: B det lag je Seed
1 Punkt niedriger als am 08.07. — Ursache ungeklärt.

## Das 95-%-Bootstrap-CI ist die tragende Zahl

Die Doku berichtet zusätzlich ein **stratifiziertes Bootstrap-CI** (10 000 Resamples über die drei
Läufe):

| | A | B |
|---|---|---|
| 95-%-CI stoch | **[66,7; 80,0]** | **[73,3; 86,0]** |
| 95-%-CI det | [24,7; 40,0] | [34,0; 50,0] |

Und sie sagt selbst, was daraus folgt: Auf **A schließt das Intervall die 70-%-Schwelle ein** — das
Ziel ist im Mittel, aber **nicht mit statistischer Sicherheit** übertroffen. Auf **B liegt das
Intervall deutlich über 60 %**. Das ist die ehrliche Fassung und sie steht bereits so in
`Projektdokumentation.tex`.

`v12_s1` liegt auf A einzeln mit 64 % **unter** dem Ziel; der Schnitt wird von s2/s3 getragen.
Deshalb die Aufstockung auf n=7 ([[projekt-status]]).

## Warum B > A — die belegte Erklärung

Nicht "Seed-Rauschen" als Handwedeln, sondern konkret gemessen (steht in der Doku): **Testset A
enthält zufällig mehr Welten mit langem Laufweg — 38 % gegenüber 32 % mit ≥ 42 Feldern.** Da der
Erfolg mit der Weglänge fällt ([[det-stoch-gap]]), ist A schlicht das schwerere Set. Beide Sets
stammen aus demselben Generator und derselben Distanzverteilung.

Das ist die stärkere Aussage als die frühere Formulierung ("Argument gegen Overfitting"), weil sie
eine **Ursache** nennt statt eine Abwesenheit zu behaupten. Dass kein Overfitting vorliegt, folgt
ohnehin schon daraus, dass die Modellselektion auf separaten Val-Seeds läuft
([[testset-leakage]]).

Der [[det-stoch-gap]] ist **nicht geschlossen** (32 % / 42 %). Das Phase-3-Annealing ist volatil,
die det-Maxima je Lauf lagen nur bei 54/26/36 %.

## `best_model` bleibt die Ausgabe

Gegengetestet gegen `phase2_best` (CHANGELOG `v2026-07-08.2`): final A 73,3 ± 6,8 / B 80,0 ± 6,5
gegen phase2 A 71,3 ± 8,1 / B 74,0 ± 15,0. Gleichwertig im Mittel, aber **deutlich stabiler** —
phase2 streut auf B mit ±15 doppelt so stark.

## Methodisch

Erstmals mit 3 Läufen + Mittelwert ± Std (Projektvorgabe, [[henderson-2018]]). Kein Best-of.
Eval-Skript `scratchpad/final_eval.py`, Ergebnisse `scratchpad/final_eval_results.json`.
