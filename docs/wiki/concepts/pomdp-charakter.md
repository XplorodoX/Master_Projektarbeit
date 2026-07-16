---
id: pomdp-charakter
title: Stoneforge ist ein POMDP — die zentrale These
type: concept
tags: [theorie, pomdp, these, projektarbeit]
related: [det-stoch-gap, stochastische-eval, singh-1994, ghosh-2021, observation-space]
updated: 2026-07-17
---

# Stoneforge ist ein POMDP

Das ist die theoretische Achse der ganzen Arbeit. Alles andere — die Wahl der Eval-Methode, die
Deutung des [[det-stoch-gap]], die Architekturentscheidung für ein LSTM — hängt daran.

## Die Kriterien sind erfüllt

- Der Agent sieht **nur 15×15 Tiles** (`observationRadius=7`) → kein Vollzustand.
- Die **Exit-Position ist nicht sichtbar**, nur Richtungsfeatures (`exitDx`/`exitDy`) plus das
  Potentialfeld aus [[pbrs-reward-shaping]].
- **Wände verbergen Pfade** → die Karte ist unvollständig, Sackgassen sind nicht vorhersehbar.

## Die Konsequenz, die viele überspringen

Im MDP ist **immer** eine deterministische Policy optimal. Im POMDP nicht — [[singh-1994]] zeigt,
dass eine stochastische Policy ohne Zustandsschätzung erheblich mehr Reward holen kann als jede
deterministische. Das ist kein Randfall, sondern der Normalfall bei partieller Beobachtbarkeit.

[[ghosh-2021]] verschärft das noch: Schon der Anspruch auf **Generalisierung zu unbekannten Levels**
erzeugt für sich genommen partielle Beobachtbarkeit (der "epistemic POMDP") — auch wenn das
einzelne Level vollständig beobachtbar wäre. Das Projekt tut genau das (unbekannte Seeds), liegt
also doppelt im POMDP-Regime.

## Was daraus folgt

Das LSTM nutzt seinen Hidden State als **approximierten Belief State**. Er konvergiert nicht
vollständig — deshalb kompensiert stochastisches Sampling den Rest. Der gemessene Gap ist damit
eine erwartbare Signatur der Problemklasse, **keine Fehlfunktion**. Genau so gehört er in die
Diskussion: als Limitation mit Theoriebezug, nicht als Bug.

## Textbaustein für die Arbeit

> Da Stoneforge ein POMDP darstellt (partiell beobachtbare Karte, lokale 15×15-Sicht), ist eine
> stochastische Evaluation nach wissenschaftlichem Standard legitim [Singh et al. 1994; Ghosh et
> al. 2021]. Das LSTM-Modell nutzt seinen Hidden State als approximierten Belief State;
> stochastisches Sampling kompensiert die verbleibende Unsicherheit. Der Deterministic-Stochastic-
> Gap zeigt, dass der Belief State noch nicht vollständig konvergiert ist.

Deterministische Evaluation wäre Pflicht bei **vollständig** beobachtbaren Umgebungen. Hier ist
sie eine Zusatzinformation über die Belief-Qualität — und wird als solche berichtet, nicht
verschwiegen.
