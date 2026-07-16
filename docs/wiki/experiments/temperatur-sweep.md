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

Die Frage kam nicht aus der Luft, sondern aus der Literatur:
[[stochastic-deterministic-minds-2025]] berichtet Umgebungen, in denen die deterministische Politik
das stochastische Standardvorgehen schlägt.

## Ergebnis — monoton steigend

| Temperatur | Testset A | Holdout B |
|------------|-----------|-----------|
| argmax (T = 0) | 32,0 ± 4,9 % | 42,0 ± 4,3 % |
| T = 0.25 | 52,0 ± 0,0 % | 52,7 ± 1,9 % |
| T = 0.5 | 63,3 ± 4,1 % | 71,3 ± 6,6 % |
| T = 0.75 | 63,3 ± 7,5 % | 72,0 ± 6,5 % |
| **stoch (T = 1.0)** | **70,7 ± 8,2 %** | **79,3 ± 6,6 %** |

**Keine Zwischentemperatur schlägt T = 1.0.** Es gibt kein Optimum in der Mitte.

> Hinweis: T = 1.0 ergibt hier **70,7 %** auf A, die Headline-Zahl in [[v12-final]] ist **73,3 %**.
> Kein Widerspruch — zwei unabhängige Messungen derselben Modelle, Differenz im Sampling-Rauschen.
> In der Arbeit nicht vermischen: Der Sweep zeigt den *Verlauf*, `tab:v12` das *Endergebnis*.

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
