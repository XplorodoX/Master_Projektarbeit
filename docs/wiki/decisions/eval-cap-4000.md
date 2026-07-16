---
id: eval-cap-4000
title: Entscheidung — Eval-Cap immer 4000
type: decision
tags: [evaluation, entscheidung, messfehler]
related: [eval-protokoll, batch-size-forensik, stoneforge-env]
status: bestätigt
decided: 2026-07-07
updated: 2026-07-17
---

# Eval-Episoden-Cap = 4000, immer

## Entscheidung

Jede Evaluation läuft mit Cap **4000** = `maxSteps` des Env. Keine Ausnahme, auch nicht in
Zwischenevals während des Trainings.

## Warum das keine Kleinigkeit ist

Ein zu kleiner Cap **verfälscht die Success Rate massiv**. Gemessen am selben Modell
(Seed-0 `phase1_best`):

| Cap | gemessene SR |
|-----|--------------|
| 600 | 48 % |
| **4000** | **86 %** |

Fast eine Verdoppelung — allein durch das Messgerät. Die SR saturiert erst bei 4000. Der Grund
ist die stochastische Policy: Sie ist erfolgreich, aber **ineffizient** (viel Zickzack, siehe
[[demo-und-visualisierung]]). Ein Cap von 600 schneidet Episoden ab, die noch erfolgreich geworden
wären, und misst damit nicht Erfolg, sondern Effizienz.

## Der Schaden, den das angerichtet hat

Die am 06.07. eingeführten Caps (Phase 1 = 600, Phase 2 = 1200) haben eine **"Stagnation"
vorgetäuscht**, die es großteils nicht gab. Seed 0 und 1 sahen aus, als lernten sie nicht — sie
lernten, wir maßen falsch. Das hat die Diagnose der echten Instabilität ([[batch-size-forensik]])
zusätzlich verschleiert: zwei überlagerte Fehler, einer im System, einer im Messgerät.

Der Phase-3-Kollaps dagegen war **real** — nicht jeder Befund war Artefakt.

## Der Preis

Eine volle Eval mit Cap 4000 kostet **~21 Sekunden**. Dafür war die Verkürzung nie gedacht — sie
sparte Sekunden und kostete Tage. Caps in `train_curriculum.py` sind seit 07.07.2026 auf 4000
zurückgesetzt. CHANGELOG `v2026-07-07.2`.
