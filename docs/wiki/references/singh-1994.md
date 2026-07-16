---
id: singh-1994
title: Singh, Jaakkola & Jordan (1994) — Learning Without State-Estimation in POMDPs
type: reference
tags: [literatur, pomdp, kernzitat]
bibkey: singh1994pomdp
url: https://www.cs.utexas.edu/~shivaram/readings/b2hd-SinghJJ1994.html
venue: ICML 1994
related: [pomdp-charakter, det-stoch-gap, stochastische-eval, temperatur-sweep]
updated: 2026-07-17
---

# Singh, Jaakkola & Jordan (1994)

**Kernaussage:** In POMDPs kann eine **stochastische memoryless-Policy erheblich höheren Reward**
erzielen als jede deterministische. Im MDP ist stets eine deterministische Policy optimal — im
POMDP nicht.

## Warum das das wichtigste Zitat der Arbeit ist

Es verwandelt den größten Schwachpunkt in einen Befund. Ohne dieses Resultat sieht
"73 % stochastisch, 32 % deterministisch" aus wie ein kaputtes Modell plus günstige Metrikwahl.
Mit ihm ist es die **erwartete Signatur der Problemklasse**.

Der gemessene Gap ([[det-stoch-gap]]) ist damit direkt gedeckt, und die Entscheidung für
[[stochastische-eval]] ist keine Notlösung, sondern folgt aus der Theorie.

Verstärkt wird das durch den [[temperatur-sweep]]: Das monotone Ergebnis ist genau das, was Singh
et al. vorhersagen — die Stochastizität ist Teil der guten Policy, nicht Rauschen darauf.

## Verwendung

BibTeX-Key `singh1994pomdp`, in `docs/references.bib`. Integriert in
`docs/Projektdokumentation.tex`, §"Det/Stoch-Gap und POMDP-These".

Zusammen mit [[ghosh-2021]] die zwei Kernzitate für Related Work und Diskussion.
