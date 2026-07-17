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

Exakte Werte aus `PHASES` in `train_curriculum.py:52–67` (verifiziert 17.07.2026):

| Phase | Exit-Distanz (Train) | Eval auf | Budget | Gate | **Gate-Metrik** |
|-------|---------------------|----------|--------|------|-----------------|
| 1 | 5–12 | 5–12 | 500 k | SR ≥ 85 % | stochastisch |
| 2 | 12–25 | 12–25 | 500 k | SR ≥ 70 % | stochastisch |
| 3 | 25–45 | **35–45** | 1 M | SR ≥ 70 % | **deterministisch** |
| 4 (Greedy Fine-Tune) | 25–45 | 35–45 | 200 k | SR ≥ 60 % | **deterministisch** |

Zwei Details, die man leicht übersieht:

- **Der Gate-Wechsel stoch → det ab Phase 3** ist Absicht: In Phase 1/2 ist die Policy mit
  `ent_coef=0.05` bewusst stochastisch, ein det-Gate trüge dort kein Signal; ab Phase 3 (nach dem
  Annealing-Start) ist det die Zielmetrik (Kommentar `train_curriculum.py:44–46`).
- **Phase 3 trainiert auf 25–45, evaluiert aber nur auf 35–45** — dem Zielbereich.

**Entropie-Annealing in Phase 3:** linear **0.05 → 0.001 über 500 k Steps**
(`EntropyAnnealingCallback`, Aufruf Z. 457 mit `start_ent=RPPO_KWARGS["ent_coef"]`). Phase 4 setzt
dann hart `ent_coef = 0.0001`.

> ⚠️ Der **Docstring** des Callbacks (Z. 240) behauptet "from 0.01 to 0.001" — falsch, der Aufruf
> übergibt 0.05. Der Kommentar an der Aufrufstelle (Z. 454–456) erklärt sogar, warum 0.05 sein
> MUSS (sonst springt die Entropie beim Phasenwechsel abrupt). Dieselbe falsche 0.01 steht auch
> als Formel im Aalen-Dokument → [[doku-worldgen-veraltet]].

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
