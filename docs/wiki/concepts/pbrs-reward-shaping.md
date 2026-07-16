---
id: pbrs-reward-shaping
title: PBRS — Potential-Based Reward Shaping auf BFS-Distanz
type: concept
tags: [reward, theorie, cpp]
related: [cpp-core, stoneforge-env, v11-env-bruch]
updated: 2026-07-17
---

# Potential-Based Reward Shaping (PBRS)

Implementiert in `src/core/simulation.cpp`. Parameter: **β = 2.5**, **γ = 0.999** (identisch zum
RL-γ — das ist die Bedingung für Policy-Invarianz, nicht Kosmetik).

## Das Potential läuft auf BFS-Distanz

Nicht auf Luftlinie, nicht auf Manhattan-Distanz. Der Unterschied ist entscheidend: Ein
Luftlinien-Potential belohnt den Agenten dafür, sich gegen eine Wand zu drücken, hinter der der
Exit liegt. Das BFS-Potential misst den **echten Laufweg** und belohnt nur tatsächlichen
Fortschritt.

> ⚠️ **Nie auf Luftlinien-/Manhattan-Potential zurückbauen.** Das ist eine der teuersten
> Fehlerquellen im Projekt und steht nicht ohne Grund in den Fallstricken.

Weil PBRS potentialbasiert ist (Differenz eines Potentials über γ), ist es **policy-invariant**:
Es beschleunigt das Lernen, verändert aber die optimale Policy nicht. Man kauft sich also
Lerngeschwindigkeit, ohne die Aufgabe zu verfälschen — der Grund, warum das methodisch sauber ist.

## Das BFS steckt im Reward, nicht in der Observation

Dieser Unterschied trägt die ganze Arbeit. Der Agent bekommt BFS-**Fortschritt** als Lernsignal,
aber **keine BFS-Distanz zum Anschauen** ([[observation-space]]). Er muss die Navigation selbst
lernen; das BFS sagt ihm im Training nur, ob es besser wurde. Zur Testzeit ist kein Orakel dabei.
Vergleich der Varianten: [[ablation-abc]].

## Straf-Stacking (mit v11 entschärft)

Frühere Versionen stapelten Strafen (Wand-Penalty + Loop-Penalty + Stagnation), bis das
Reward-Signal dominiert wurde und der Agent Bewegung generell mied. Mit [[v11-env-bruch]]:
Wand-Penalty **entfernt**, Loop-Penalty von −0.15 auf **−0.05** reduziert.
