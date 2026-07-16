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

**Wie am 08.07. gemessen und in der Doku ausformuliert (n=3):**

| Testset | stoch | det |
|---------|-------|-----|
| A (7000–7049) | **73,3 % ± 6,8** (Ziel ≥70 % ✓) | 32 % |
| B (8000–8049) | **80,0 % ± 6,5** (Ziel ≥60 % ✓) | 42 % |

Einzelläufe A stoch: 64 / 76 / 80. B stoch: 72 / 80 / 88.

**Nachmessung 15.07. gegen Env v11 (dieselben Modelle, unabhängig reproduziert):**
A 72,7 % ± 9,5 stoch / 32,0 % ± 6,0 det · B 77,3 % ± 6,1 stoch / 40,0 % ± 5,3 det.
A det war **exakt identisch** (38/26/32), stoch im Rauschen. Offene Kleinigkeit: B det lag je Seed
1 Punkt niedriger als am 08.07. — Ursache ungeklärt.

## Beide Ziele im Mittel erfüllt — aber ehrlich bleiben

Bei Testset A ist der Abstand zum Ziel (2,7 Punkte) **kleiner als die Streuung**. `v12_s1` liegt
einzeln mit 62–64 % **unter** dem Ziel; der Schnitt wird von s2/s3 getragen. Bei n=3 lässt sich
"robust über 70 %" nicht behaupten — genau deshalb läuft die Aufstockung auf n=7
([[projekt-status]]). Das CI reicht laut Doku bis 66,7 % hinunter.

**B > A** (80 vs. 73): Die Seed-Auswahl erzeugt mehr Varianz als der Train/Holdout-Unterschied.
Das ist ein Argument **gegen** Overfitting und stützt die Generalisierungs-These.

Der [[det-stoch-gap]] ist **nicht geschlossen** (32 % / 42 %). Das Phase-3-Annealing ist volatil,
die det-Maxima je Lauf lagen nur bei 54/26/36 %.

## `best_model` bleibt die Ausgabe

Gegengetestet gegen `phase2_best` (CHANGELOG `v2026-07-08.2`): final A 73,3 ± 6,8 / B 80,0 ± 6,5
gegen phase2 A 71,3 ± 8,1 / B 74,0 ± 15,0. Gleichwertig im Mittel, aber **deutlich stabiler** —
phase2 streut auf B mit ±15 doppelt so stark.

## Methodisch

Erstmals mit 3 Läufen + Mittelwert ± Std (Projektvorgabe, [[henderson-2018]]). Kein Best-of.
Eval-Skript `scratchpad/final_eval.py`, Ergebnisse `scratchpad/final_eval_results.json`.
