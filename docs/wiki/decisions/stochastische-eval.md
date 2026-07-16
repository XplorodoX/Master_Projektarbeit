---
id: stochastische-eval
title: Entscheidung — stochastische Evaluation als Primärmetrik
type: decision
tags: [evaluation, entscheidung, methodik, projektarbeit]
related: [pomdp-charakter, det-stoch-gap, temperatur-sweep, singh-1994, ghosh-2021, zielkriterium]
status: bestätigt
decided: 2026-06-08
updated: 2026-07-17
---

# Stochastische Evaluation ist die Primärmetrik

Die Entscheidung mit dem höchsten Rechtfertigungsdruck im Projekt — weil sie von außen wie
Rosinenpickerei aussieht (73 % klingt besser als 32 %). Sie ist es nicht, aber das muss **belegt**
werden, nicht behauptet.

## Entscheidung

Berichtet werden **beide** Werte. Die stochastische SR ist die Primärmetrik, gegen die die
Zielkriterien ([[zielkriterium]]) geprüft werden. Die deterministische wird immer mitgeführt.

## Drei unabhängige Standbeine

1. **Theoretisch:** Stoneforge ist ein POMDP ([[pomdp-charakter]]). [[singh-1994]] zeigt, dass dort
   eine stochastische Policy strikt besser sein kann als jede deterministische — im MDP unmöglich.
   [[ghosh-2021]] ergänzt: Schon der Generalisierungsanspruch auf unbekannte Seeds erzeugt für sich
   partielle Beobachtbarkeit.
2. **Empirisch:** Der [[temperatur-sweep]] ist **monoton** — keine Zwischentemperatur schlägt volles
   Sampling. Der Gap ist damit nachweislich kein argmax-Artefakt.
3. **Praktisch:** Der ausgelieferte Controller *ist* stochastisch (LSTM + Sampling). Wir evaluieren,
   was wir ausliefern.

## Was die Entscheidung NICHT ist

Sie ist **keine Ausrede für den Gap**. Der deterministische Wert wird nicht versteckt, sondern
prominent als Limitation diskutiert ([[det-stoch-gap]]) — inklusive der drei gescheiterten
Schließungsversuche ([[e1-critic]], [[e2-curriculum-sanft]], [[e3-lstm512]]). Das ist der
Unterschied zwischen einer begründeten Metrikwahl und Schönrechnen: Wir berichten die unbequeme
Zahl **und** erklären sie.

Deterministische Eval wäre Pflicht bei vollständig beobachtbaren Umgebungen. Hier ist sie eine
Messung der Belief-Qualität — und als solche selbst ein Ergebnis.

## Wenn im Kolloquium gefragt wird

Kurzfassung: *"Im MDP ist argmax optimal, im POMDP nicht — das ist ein Standardresultat (Singh et
al. 1994). Wir haben zusätzlich per Temperatur-Sweep gemessen, dass es kein Artefakt ist: die
Kurve steigt monoton bis zum vollen Sampling. Den deterministischen Wert berichten wir trotzdem,
weil er zeigt, wie weit der Belief State noch von der Konvergenz entfernt ist."*
