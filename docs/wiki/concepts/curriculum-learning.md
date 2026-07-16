---
id: curriculum-learning
title: Leistungsbasiertes Curriculum in 4 Phasen
type: concept
tags: [training, curriculum]
related: [train-curriculum, e2-curriculum-sanft, v12-final, testset-leakage]
updated: 2026-07-17
---

# Curriculum Learning

Implementiert in [[train-curriculum]]. Der Agent lernt nicht direkt auf Exit-Distanz 35–45 — das ist zu schwer für einen frischen Agenten
(Ablation B kollabiert auf 0 %). Stattdessen vier Phasen mit wachsender Distanz:

| Phase | Exit-Distanz | Gate |
|-------|--------------|------|
| 1 | 5–12 | SR-Ziel erreicht (85 %) |
| 2 | 12–25 | SR-Ziel erreicht |
| 3 | 25–45 | SR-Ziel erreicht |
| 4 | Greedy Fine-Tune | `ent_coef` → 0.0001 |

## Leistungsbasiert, nicht zeitbasiert

Das ist eine harte Lehre aus v1.0: Ein **zeitbasiertes** Curriculum (nach X Steps zur nächsten
Phase) führte zum **Reward-Kollaps** — der Agent wurde in Phase 2 geschoben, bevor er Phase 1
konnte, und verlor beides. Seit v1.1 gilt: Phasenwechsel erst, wenn das SR-Gate hält.

## Phase 4 schließt den Gap nicht

Die Idee war naheliegend: Entropie gegen null annealen → die Policy wird von selbst
deterministisch → [[det-stoch-gap]] verschwindet. Funktioniert nicht. Das Annealing in Phase 3/4
ist **volatil** und zieht die stochastische SR mit runter; det-Maxima je Lauf lagen nur bei
54/26/36 %. Das finale `best_model` bleibt trotzdem die beste Wahl (gegengetestet gegen
`phase2_best`, siehe [[v12-final]]).

## Sanfter ist nicht besser

[[e2-curriculum-sanft]] hat Phase 3 auf exit 25–35 begrenzt statt 25–45 — mit dem Ergebnis, dass
der Agent auf langen Wegen schlicht **untertrainiert** blieb (A 26 %). Die harte Phase 3 ist
notwendig, weil das Testset genau dort liegt.

## Fallstrick

Der Eval-Callback im Curriculum darf **nicht** auf Testset A selektieren → [[testset-leakage]].
