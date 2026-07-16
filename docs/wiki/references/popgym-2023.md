---
id: popgym-2023
title: Morad et al. (2023) — POPGym: Benchmarking Partially Observable RL
type: reference
tags: [literatur, benchmark, memory, pomdp]
bibkey: morad2023popgym
url: https://arxiv.org/abs/2303.01859
code: https://github.com/proroklab/popgym
venue: "ICLR 2023 (arXiv 2303.01859)"
authors: "Steven Morad, Ryan Kortvelesy, Matteo Bettini, Stephan Liwicki, Amanda Prorok"
verified: 2026-07-17
related: [literatur-lstm-groesse, e3-lstm512, memory-rewriting-2026, pomdp-charakter]
updated: 2026-07-17
---

# Morad et al. (2023) — POPGym

**Vollständig:** Steven Morad, Ryan Kortvelesy, Matteo Bettini, Stephan Liwicki, Amanda Prorok:
*POPGym: Benchmarking Partially Observable Reinforcement Learning.* ICLR 2023, arXiv:2303.01859.
Code: `github.com/proroklab/popgym`.

Aus dem Abstract (verifiziert 17.07.2026):

> "Real world applications of Reinforcement Learning (RL) are often partially observable, thus
> requiring memory. Despite this, partial observability is still largely ignored by contemporary RL
> benchmarks and libraries. We introduce Partially Observable Process Gym (POPGym) […] 15 partially
> observable environments […] 13 memory model baselines — the most in a single RL library. […] we
> execute the largest comparison across RL memory models to date."

## Wozu es hier taugt

Zwei Dinge:

1. **Es normalisiert die Fragestellung des Projekts.** Partielle Beobachtbarkeit wird von den
   üblichen Benchmarks ignoriert — das ist genau die Lücke, in der diese Arbeit sitzt. Nützlich für
   die Motivation ([[pomdp-charakter]]).
2. **Es ist der domänenrichtige Ersatz** für die verworfene Aktienhandels-Quelle in
   [[literatur-lstm-groesse]], wenn es um "welches Gedächtnismodell taugt" geht.

## ⚠️ Vorsicht bei der Zuspitzung

Eine Sekundärquelle fasst POPGym mit *"GRUs emerge as the best general-purpose memory model"*
zusammen. **Das steht so nicht im Abstract**, und eine Nachprüfung am 17.07.2026 konnte es im
zugänglichen Volltext nicht wörtlich bestätigen; eine Zusammenfassung nennt lediglich, dass
"contemporary RNNs such as GRUs performed admirably", bei gleichzeitiger "dissonance between
supervised learning performances and RL outcomes".

**Konsequenz:** Wenn diese Aussage in die Arbeit soll, vorher **selbst im PDF nachschlagen** und
die Stelle zitieren. Nicht aus zweiter Hand übernehmen — genau so ist der Fehler in
[[literatur-lstm-groesse]] entstanden.

Belastbar zitierbar ist ohne Nachschlagen: die Existenz des Benchmarks, die 15 Umgebungen, die
13 Gedächtnis-Baselines und "largest comparison across RL memory models to date".

## To-do

Noch nicht in `docs/references.bib`. BibTeX-Key-Vorschlag `morad2023popgym`.
