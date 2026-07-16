---
id: literatur-lstm-groesse
title: Literatur — LSTM-Größe hat diminishing returns
type: reference
tags: [literatur, architektur, lstm]
related: [e3-lstm512, recurrent-ppo, det-stoch-gap, pleines-2022]
updated: 2026-07-17
---

# LSTM-Größe: diminishing returns

Recherche 09.07.2026, zur Einordnung des E3-Befunds ([[e3-lstm512]]).

## Befundlage

- LSTM-Hidden-Size zeigt **diminishing returns**: Jenseits einer task-spezifischen Schwelle bringt
  mehr Kapazität nur marginale Gewinne.
- Eine konkrete PPO-LSTM-Studie findet **512 optimal, 1024 schlechter** — mehr Kapazität
  *verschlechterte* das Ergebnis.
- "Breiteste Einstellung → ähnliche oder leicht niedrigere finale Returns" ist ein wiederkehrendes
  Muster.

## Einordnung — ehrlich bleiben

Die Literatur stützt die **allgemeine Aussage** (mehr Kapazität ≠ besser, es gibt ein Optimum) und
macht damit den E3-Befund plausibel statt merkwürdig. Sie ist aber **kein Beleg für unsere konkrete
Schwelle**: In der zitierten Studie war 512 das Optimum, bei uns liegt es bei 256. Die Schwelle ist
task-spezifisch — das ist gerade der Punkt.

Also: als Plausibilisierung zitieren, nicht als Beweis. Unser Beleg für 256 > 512 ist die eigene
Messung ([[e3-lstm512]]: A 44 % gegen 73 %, phasenweise durchgängig schlechter), nicht die
Literatur.

## Die eigentliche Pointe

Der Kernsatz, den E3 und diese Literatur gemeinsam tragen:

> **Mehr Gedächtnis-Kapazität ≠ besserer Gedächtnis-Nutzen.**

Gestützt von "Memory Retention Is Not Enough…" (arXiv 2601.15086): Der Flaschenhals ist das
*Abrufen* über lange Horizonte, nicht das *Behalten*. Deshalb zeigt der Ausblick auf
**strukturierten** Speicher (GTrXL, Neural Map, FFM, AMAGO) statt auf größere LSTMs —
siehe [[pleines-2022]].
