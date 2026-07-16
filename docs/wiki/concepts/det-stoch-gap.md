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

## Drei Schließungsversuche sind gescheitert

[[e1-critic]] (stärkerer Critic), [[e2-curriculum-sanft]] (sanfteres Curriculum), [[e3-lstm512]]
(größeres LSTM). Keiner schlägt die Baseline.

## ⚠️ Genau hier NICHT überinterpretieren

Die frühere Fassung schloss daraus: *"Der Gap ist teils fundamental, teils ein Gedächtnis-Limit —
aber **kein Hyperparameter-Problem**."* Der CHANGELOG (`v2026-07-08.3`) warnt **wörtlich** vor
dieser Zuspitzung:

> "**Wichtig — NICHT überinterpretieren:** Damit ist NICHT gezeigt, dass „nur Architektur hilft".
> Es bleiben ungetestete, teils billige Hebel: (a) temperaturkalibrierte Evaluation […]
> (b) Hilfs-/Repräsentationsverluste; (c) gezielt mehr Langdistanz-Training."

Getestet wurden **drei** Hebel, jeder mit **n=1** (E1/E2 starten aus einem gemeinsamen
Phase-2-Checkpoint; E3 ist ein voller Lauf). Das erlaubt zu sagen: *Diese drei naheliegenden Hebel
schließen den Gap nicht.* Es erlaubt **nicht** zu sagen: *Der Gap ist kein Hyperparameter-Problem.*

Hebel (a) ist inzwischen abgehakt — der [[temperatur-sweep]] hat ihn geprüft und die Kurve ist
monoton. (b) und (c) sind **weiterhin ungetestet**. Wer die starke Version behauptet, hat drei
Stichproben und einen Prüfer, der genau da nachfragt.

## Die belastbare Aussage

> Der Gap ist mit den getesteten Mitteln nicht zu schließen und teilweise fundamental
> ([[singh-1994]]): Er skaliert mit der Weglänge, überlebt eine vollständige Temperatur-Kalibrierung
> und widersteht drei naheliegenden Eingriffen. Eine erschöpfende Suche war das nicht.

## Ausblick — vorsichtiger als früher formuliert

Naheliegend wäre "nicht mehr LSTM-Kapazität, sondern **strukturierter Speicher**" (GTrXL, Neural
Map, FFM, In-Context-RL wie AMAGO). Aber: **Die Literatur trägt das nicht so klar**, wie hier
lange behauptet wurde — [[memory-rewriting-2026]] findet klassische rekurrente Modelle **besser**
als strukturierte Speicher und Transformer, und [[popgym-2023]] sieht RNNs konkurrenzfähig. Also
als **offene Richtung** ausweisen, nicht als sichere Lösung. Details: [[literatur-lstm-groesse]].

Praktischer Betriebspunkt heute: T = 0.5 holt den Großteil zurück (A +31, B +29 Punkte über argmax)
bei geringerer Varianz.
