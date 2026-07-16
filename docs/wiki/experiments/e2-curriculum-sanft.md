---
id: e2-curriculum-sanft
title: E2 — sanfteres Curriculum (Phase 3 nur exit 25–35)
type: experiment
tags: [negativergebnis, gap-experiment, e2, curriculum]
path: models/exp_E2_curric
related: [curriculum-learning, det-stoch-gap, v12-final, e1-critic, e3-lstm512]
status: widerlegt
updated: 2026-07-17
---

# E2 — Sanfteres Curriculum

**Hypothese:** Der Sprung in Phase 3 (exit 25–**45**) ist zu abrupt. Ein sanfterer Übergang
(25–35) sollte stabiler lernen und den [[det-stoch-gap]] verkleinern.

**Eingriff:** Phase 3 auf exit 25–35 begrenzt. Sonst v12-Konfiguration.

## Ergebnis

| Testset | stoch |
|---------|-------|
| A | **26 %** |
| B | 42 % |

Gegen die v12-Baseline (A 73,3 / B 80): **klar schlechter**, nicht knapp.

## Bewertung

Der Grund ist im Nachhinein offensichtlich: Das Testset liegt bei Exit-Distanz **35–45**. Wer nur
bis 35 trainiert, ist auf dem Testbereich schlicht **untertrainiert**. Sanfter heißt hier nicht
"stabiler", sondern "an der Aufgabe vorbei".

Die Lehre ist allgemeiner als das Experiment: Ein Curriculum darf die Zielverteilung **glätten**,
aber nicht **verlassen**. Die letzte Phase muss die Testverteilung abdecken, sonst verschiebt man
das Problem nur an die Evaluationsgrenze.

Das erklärt auch, warum die harte Phase 3 in [[curriculum-learning]] trotz ihrer Volatilität
bleibt — sie ist unbequem, aber notwendig.

CHANGELOG `v2026-07-08.3`.
