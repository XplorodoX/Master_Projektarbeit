---
id: det-stoch-gap
title: Det/Stoch-Gap — der Diskussionskern der Arbeit
type: concept
tags: [befund, evaluation, lstm, diskussion]
related: [pomdp-charakter, temperatur-sweep, e1-critic, e3-lstm512, singh-1994, v12-final]
status: bestätigt
updated: 2026-07-17
---

# Der Deterministic/Stochastic-Gap

Dasselbe Modell erreicht stochastisch (Sampling aus der Policy) rund **73 %** Success Rate,
deterministisch (argmax) nur **32 %**. Dieser Abstand ist der interessanteste Befund des Projekts
und der Kern des Diskussionsteils.

## Mechanik

Deterministisch pendeln die LSTM-Ausgaben in unsicheren Situationen bei etwa A = 51 % / B = 49 %.
argmax wählt dann **immer** A → der Agent läuft in eine Endlosschleife und verbrennt die Episode.
Stochastisch "wackelt" er sich per Sampling aus der Sackgasse heraus. Das Sampling kompensiert
also den unvollständigen Belief State.

## Der Gap skaliert mit der Weglänge

Gemessen 08.07.2026 (`scripts/plot_det_gap_distanz.py`, Figur `docs/figures/fig_det_gap_distanz.pdf`):

| Exit-Distanz | det | stoch |
|--------------|-----|-------|
| 5–15 Felder | 79 % | 100 % |
| 35–45 Felder | 31 % | 82 % |

Kurze Wege löst der Agent auch deterministisch. Je länger der Weg, desto stärker **verschwimmt der
LSTM-Belief** und desto häufiger kippt argmax in Loops. Das ist der Beleg dafür, dass es um
Gedächtnis über lange Horizonte geht — nicht um zu wenig Kapazität.

## Es ist kein argmax-Artefakt

Der [[temperatur-sweep]] fährt den Übergang argmax → volles Sampling durch. Das Ergebnis ist
**monoton steigend** (A: 32 → 52 → 63 → 63 → 71 %). Keine Zwischentemperatur schlägt T = 1.0.
Gäbe es einen billigen Trick, hätte der Sweep ihn gefunden — es gibt keinen.

## Es ist nicht wegtunebar

Drei Versuche, den Gap zu schließen, sind alle gescheitert: [[e1-critic]] (stärkerer Critic),
[[e2-curriculum-sanft]] (sanfteres Curriculum), [[e3-lstm512]] (größeres LSTM). Keiner schlägt die
Baseline. Zusammen mit dem Sweep und [[singh-1994]] ergibt das die Aussage der Arbeit: **Der Gap
ist teils fundamental (POMDP-Eigenschaft), teils ein Gedächtnis-Limit — aber kein
Hyperparameter-Problem.**

## Ausblick (steht so im Fazit)

Nicht mehr LSTM-Kapazität, sondern **strukturierter Speicher**: GTrXL, Neural Map, FFM oder
In-Context-RL (AMAGO). Belege in [[pleines-2022]]. Praktischer Betriebspunkt heute: T = 0.5 holt
den Großteil zurück.
