---
id: ghosh-2021
title: Ghosh et al. (2021) — Why Generalization in RL is Difficult: Epistemic POMDPs
type: reference
tags: [literatur, pomdp, generalisierung, kernzitat]
bibkey: ghosh2021epistemic
url: https://arxiv.org/abs/2107.06277
neurips: https://proceedings.neurips.cc/paper/2021/hash/d5ff135377d39f1de7372c95c74dd962-Abstract.html
venue: "NeurIPS 2021 (Advances in Neural Information Processing Systems 34), arXiv 2107.06277"
authors: "Dibya Ghosh, Jad Rahme, Aviral Kumar, Amy Zhang, Ryan P. Adams, Sergey Levine"
verified: 2026-07-17
related: [pomdp-charakter, stochastische-eval, zielkriterium, singh-1994]
updated: 2026-07-17
---

# Ghosh et al. (2021) — Epistemic POMDPs

**Vollständig:** Dibya Ghosh, Jad Rahme, Aviral Kumar, Amy Zhang, Ryan P. Adams, Sergey Levine:
*Why Generalization in RL is Difficult: Epistemic POMDPs and Implicit Partial Observability.*
NeurIPS 2021, arXiv:2107.06277. (Verifiziert 17.07.2026.)

**Kernaussage (Abstract):** Generalisierung auf ungesehene Testbedingungen aus einer begrenzten Zahl
von Trainingsbedingungen **induziert implizite partielle Beobachtbarkeit** — sie verwandelt selbst
vollständig beobachtbare MDPs in POMDPs. Die Autoren formulieren Generalisierung im RL als das Lösen
dieses induzierten POMDP ("epistemic POMDP") um, zeigen Fehlermodi von Algorithmen, die diese
partielle Beobachtbarkeit ignorieren, und schlagen ein ensemblebasiertes Näherungsverfahren vor.
Empirisch: deutliche Generalisierungsgewinne auf **Procgen**.

> ⚠️ **Belegtiefe beachten (geprüft 17.07.2026):** Dass die optimale Policy im epistemic POMDP
> **stochastisch** ist, konnte im zugänglichen Abstract **nicht wörtlich bestätigt** werden — dort
> stehen die Umformulierung als POMDP und die Fehlermodi, nicht die Charakterisierung der optimalen
> Policy. Die Aussage ist über den POMDP-Charakter zwar sachlich impliziert (und über [[singh-1994]]
> gedeckt), aber wer sie **Ghosh zuschreiben** will, muss die Stelle vorher im Volltext nachschlagen
> und zitieren. Sonst: Ghosh für "Generalisierung ⇒ partielle Beobachtbarkeit" zitieren und den
> Schritt zur stochastischen Policy über Singh führen.

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
