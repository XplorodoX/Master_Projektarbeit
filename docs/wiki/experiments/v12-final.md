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

**Sieben Läufe** (`models/ppo_lstm_curriculum_v12_s{1..7}`), `batch_size=8`, alle vier Phasen
komplett durchgelaufen. s1–s3: 07./08.07. (7,7–8,5 h). s4–s7: 15.–17.07. (29,5–36,3 h, gedrosselt
wegen zugeklapptem Laptop — ohne Ergebnisrelevanz).

## 🔴 Endstand n=7 (17.07.2026) — Ziel A wird VERFEHLT

CHANGELOG `v2026-07-17`. **Das ist der maßgebliche Stand.** Die n=3-Zahlen darunter sind
historisch.

| Testset | stoch | 95-%-CI (t, df=6) | Ziel | erfüllt? |
|---------|-------|-------------------|------|----------|
| **A** (7000–7049) | **65,7 % ± 12,4** | [54,2; 77,2] | ≥ 70 % | **✗ NEIN** |
| **B** (8000–8049) | **66,9 % ± 12,8** | [55,0; 78,7] | ≥ 60 % | ✓ ja |

det: A **29,1 % ± 8,0** · B **32,6 % ± 12,7** (Std durchgehend `ddof=1`).

Einzelwerte A stoch: 62 / 84 / 76 / 74 / 58 / 50 / 56 · B stoch: 76 / 76 / 80 / 74 / 62 / 50 / 50.

**Die Aufstockung hat die Zahlen gesenkt, nicht stabilisiert:** A 73,3 → 65,7, B 80,0 → 66,9. Der
n=3-Mittelwert war zu optimistisch. Von den vier Ziel/Modus-Kombinationen hält nur noch **B
stochastisch** — und dessen CI reicht bis 55,0, also unter die Schwelle.

**Das ist kein Betriebsunfall, sondern der Zweck der Übung.** Die Doku hatte das Risiko selbst
benannt (CI [66,7; 80,0] schließt 70 % ein). [[henderson-2018]] fordert genau deshalb mehrere
Seeds — hier hat die Forderung geliefert, was sie soll.

## Drei Alternativerklärungen geprüft, alle ausgeschlossen

| Verdacht | Ergebnis |
|---|---|
| **Messfehler** | ✗ Das neue Eval-Skript reproduziert **alle sechs det-Werte von s1–s3 bitgenau** (38/26/32, 46/44/36 = identisch mit dem 08.07.-Stand) |
| **Code-Stand** (s1–s3 auf `~97ab30d`, s4–s7 auf `3c8fa26-dirty`) | ✗ `git diff` über `src/core src/python src/include python/ game_config.json train_curriculum.py` = **genau eine Datei**: `doc_logger.py` (+22 Z., die `_git_commit()`-Stempelfunktion). Simulation, Env, Trainingsskript **identisch**. Die übrigen Diffs sind Client-Dateien ([[cpp-core]]). |
| **Umgebung** | ✗ `torch` (14.05.) und `sb3_contrib` (15.05.) im venv sind älter als **beide** Chargen; die `requirements.txt`-Pinnung vom 09.07. war Deklaration, kein Reinstall |

**Es ist Seed-Varianz.** s4 liegt mit 74 % mitten im s1–s3-Band; Welch-Test s1–s3 (74,0) vs. s4–s7
(59,5): **p = 0,149, nicht signifikant**. Die n=7-Zahlen sind belastbar.

## Was bleibt — und was fällt

**Fällt:** der Anspruch „beide Zielkriterien erfüllt".

**Bleibt unberührt:** Der [[det-stoch-gap]] besteht bei n=7 unverändert (det 29,1/32,6, Trainings-Evals
der neuen Läufe 0,26–0,42 = dasselbe Band wie s1–s3). Und der Kernbeitrag [[ablation-abc]]
("Gedächtnis schlägt Orakel") hängt an der *Relation* der Bedingungen, nicht an der Höhe der SR.

---

## Historisch: n=3 (08.07.) — überholt, nicht mehr als Ergebnis berichten

**Wie am 08.07. gemessen und in der Doku ausformuliert (n=3, Tab. `tab:v12`):**

| Testset | stoch | det |
|---------|-------|-----|
| A (7000–7049) | **73,3 % ± 6,8** (Ziel ≥70 % ✓) | 32,0 % ± 4,9 |
| B (8000–8049) | **80,0 % ± 6,5** (Ziel ≥60 % ✓) | 42,0 % ± 4,3 |

Einzelläufe — A stoch: 64 / 76 / 80 · A det: 38 / 26 / 32 · B stoch: 72 / 80 / 88 · B det: 46 / 44 / 36.

⚠️ Diese ±Werte sind **`ddof=0`** (nachgerechnet 17.07.2026). Mit `ddof=1` — der bei n=3 korrekten
Stichproben-Variante — wären es A ± 8,3 / B ± 8,0. Siehe [[eval-protokoll]].
**Update 18.07.:** Die Doku zitiert die n=3-Zwischenauswertung jetzt durchgehend mit `ddof=1`
(± 8,3 / ± 8,0); die ddof=0-Werte hier bleiben als historisches Protokoll stehen.

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
