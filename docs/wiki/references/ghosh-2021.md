---
id: ghosh-2021
title: Ghosh et al. (2021) — Why Generalization in RL is Difficult: Epistemic POMDPs
type: reference
tags: [literatur, pomdp, generalisierung, kernzitat]
bibkey: ghosh2021epistemic
url: https://arxiv.org/pdf/2107.06277
venue: arXiv 2107.06277
related: [pomdp-charakter, stochastische-eval, zielkriterium, singh-1994]
updated: 2026-07-17
---

# Ghosh et al. (2021) — Epistemic POMDPs

**Kernaussage:** Generalisierung auf unbekannte Level erzeugt **selbst dann** partielle
Beobachtbarkeit, wenn das einzelne Level vollständig beobachtbar wäre — der "epistemic POMDP".
Die optimale Policy ist dort stochastisch.

## Warum das für dieses Projekt passt wie angegossen

Das Projekt evaluiert per Definition auf **unbekannten Seeds** ([[zielkriterium]]). Damit greift
das Argument unabhängig von der 15×15-Sicht: Selbst mit voller Sicht auf die Karte wäre die
Generalisierungsaufgabe ein POMDP, weil die Unsicherheit **über die Weltverteilung** besteht, nicht
nur über den Zustand.

Stoneforge liegt also **doppelt** im POMDP-Regime — durch die begrenzte Sicht (klassisch, gedeckt
von [[singh-1994]]) *und* durch den Generalisierungsanspruch (epistemisch, gedeckt hier). Das macht
[[stochastische-eval]] zu einer robusten Entscheidung: Selbst wer die erste Begründung anzweifelt,
landet bei derselben Schlussfolgerung.

## Verwendung

BibTeX-Key `ghosh2021epistemic`, in `docs/references.bib`. Integriert in
`docs/Projektdokumentation.tex`. Zusammen mit [[singh-1994]] das Fundament der POMDP-These
([[pomdp-charakter]]).
