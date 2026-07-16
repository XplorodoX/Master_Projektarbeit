---
id: temperatur-sweep
title: Temperatur-Sweep — der Gap ist monoton
type: experiment
tags: [evaluation, befund, gap]
path: scratchpad/temp_sweep.py, scripts/eval_temperature.py
related: [det-stoch-gap, stochastische-eval, singh-1994, eval-protokoll]
status: bestätigt
updated: 2026-07-17
---

# Temperatur-Sweep der Evaluation

**Frage:** Ist der [[det-stoch-gap]] nur ein **argmax-Artefakt**? Vielleicht liegt das Optimum bei
einer milden Temperatur — leicht stochastisch, aber entschlossener als volles Sampling. Dann wäre
der Gap ein Messproblem und kein Befund.

**Aufbau:** `scratchpad/temp_sweep.py` (LSTM-korrekt), 3 v12-Modelle, Testsets A + B, Cap 4000.
Sweep von argmax bis volles Sampling (T = 1.0).

## Ergebnis — monoton steigend

| | argmax | → | → | → | T = 1.0 |
|---|---|---|---|---|---|
| **A** | 32 % | 52 % | 63 % | 63 % | **71 %** |
| **B** | 42 % | 53 % | 71 % | 72 % | **79 %** |

**Keine Zwischentemperatur schlägt T = 1.0.** Es gibt kein Optimum in der Mitte.

## Bewertung

Das ist der sauberste Beleg des Projekts dafür, dass der Gap **echt** ist:

- Wäre er ein argmax-Artefakt (nur die Endlosschleifen), müsste schon ein bisschen Rauschen den
  Großteil zurückholen und die Kurve würde früh abflachen oder ein Zwischenmaximum zeigen.
- Stattdessen wird es **bis zum vollen Sampling durchgehend besser**. Die Stochastizität ist nicht
  Schmiermittel gegen Loops, sondern **Teil der guten Policy** — genau die empirische Signatur, die
  [[singh-1994]] für POMDPs vorhersagt.

Damit ist die Wahl von stochastisch als Primärmetrik nicht nur theoretisch begründet
([[pomdp-charakter]]), sondern **empirisch gemessen**. Das ist die stärkere Version des Arguments
und gehört so in die Verteidigung.

**Praktischer Nebenbefund:** T = 0.5 holt den Großteil zurück (63 % / 71 %) und ist ein brauchbarer
Betriebspunkt, wenn man weniger Zickzack will. Für die berichteten Zahlen bleibt T = 1.0.

CHANGELOG `v2026-07-09`. Tabelle `tab:temperatur` in der Doku.
