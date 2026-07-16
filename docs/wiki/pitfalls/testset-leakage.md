---
id: testset-leakage
title: Fallstrick — Test-Set-Leakage über den Eval-Callback
type: pitfall
tags: [fallstrick, methodik, evaluation, kritisch]
related: [curriculum-learning, eval-protokoll, zielkriterium, train-curriculum]
status: offen
updated: 2026-07-17
---

# Test-Set-Leakage (P0.1)

**Das Problem:** Der Curriculum-Callback evaluiert und **selektiert** das `best_model` auf den Seeds
**7000–7049** — das ist Testset A ([[zielkriterium]]).

Damit ist Testset A **kein sauberes Testset mehr**, sondern faktisch ein Validierungsset: Das
ausgelieferte Modell wurde darauf ausgewählt. Wer über Hunderte Checkpoints hinweg den besten auf A
auswählt, überträgt Information aus A ins Modell — auch ohne je darauf zu trainieren.

**Die Lösung:** Auf eigene `VAL_SEEDS 6000–6049` umstellen. Dokumentiert in
`docs/IMPROVEMENT_PLAN.md` als **P0.1**.

## Status: methodisch bekannt, nicht behoben

Das ist ehrlich zu benennen — und das Wiki ist der falsche Ort, es schönzureden.

**Was den Schaden begrenzt:** Der **Holdout B (8000–8049)** ist von der Selektion völlig unberührt.
Und genau dort ist das Ergebnis **besser** (80 % gegen 73 % auf A, siehe [[v12-final]]). Hätte die
Selektion A nennenswert aufgebläht, müsste A über B liegen — es ist umgekehrt.

Das ist ein starkes empirisches Indiz, dass der Leakage-Effekt hier klein ist. Ein Beweis ist es
nicht.

## Empfehlung für die Arbeit

**B als Hauptzahl führen**, A als Vergleichswert, und das Leakage im Methodikteil **offen
benennen** — inklusive des B-über-A-Arguments als Entlastung. Das ist wissenschaftlich sauberer
als das Problem zu verschweigen, und es kostet nichts: B erfüllt sein Ziel (≥60 %) mit deutlichem
Abstand.

Ein Prüfer, der das Leakage selbst findet, ist ein Problem. Ein Prüfer, der liest, dass man es
kannte, eingegrenzt und begründet hat — der sieht Methodenkompetenz.
