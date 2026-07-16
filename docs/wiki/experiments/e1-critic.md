---
id: e1-critic
title: E1 — stärkerer Critic (vf_coef 0.5 → 1.0)
type: experiment
tags: [negativergebnis, gap-experiment, e1]
path: models/exp_E1_critic
related: [det-stoch-gap, v12-final, e2-curriculum-sanft, e3-lstm512]
status: widerlegt
updated: 2026-07-17
---

# E1 — Stärkerer Critic

**Hypothese:** Der [[det-stoch-gap]] entsteht durch einen schwachen Critic in Phase 3
(`explained_variance` bricht ein). Ein höher gewichteter Value-Loss sollte den Belief stabilisieren
und damit argmax verlässlicher machen.

**Eingriff:** `vf_coef` 0.5 → **1.0**. Sonst v12-Konfiguration.

## Ergebnis

| Testset | stoch | det |
|---------|-------|-----|
| A | 56 % | 42 % |
| B | 72 % | 36 % |

Gegen die v12-Baseline (A 73,3 / det 32): **det leicht hoch, stoch deutlich runter.**

## Bewertung

Ein Tausch, kein Gewinn. Der stärkere Critic macht die Policy tatsächlich etwas entschlossener
(det 32 → 42 %), aber er kauft das mit Exploration — die stochastische SR fällt von 73 auf 56 %.
Da stochastisch die Primärmetrik ist ([[stochastische-eval]]), ist das netto ein **Verlust**.

Interessant für die Diskussion: Der Effekt zeigt, dass det und stoch **gegenläufig** an derselben
Stellschraube hängen. Man verschiebt Wahrscheinlichkeitsmasse zwischen Ausbeutung und Erkundung,
statt den Belief tatsächlich zu verbessern. Genau das erwartet man, wenn der Gap fundamental ist
und nicht an Tuning hängt ([[singh-1994]]).

CHANGELOG `v2026-07-08.3`. Kein Grund, die Baseline zu wechseln.
