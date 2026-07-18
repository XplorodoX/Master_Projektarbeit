---
id: zielkriterium
title: Problemstellung und Zielkriterium der Projektarbeit
type: concept
tags: [projektarbeit, evaluation, ziel]
related: [eval-protokoll, v12-final, stochastische-eval, det-stoch-gap]
updated: 2026-07-17
---

# Problemstellung und Zielkriterium

Ein RL-Agent soll in prozedural generierten 2D-Welten (Stoneforge) den Exit finden. Die
eigentliche Frage ist **nicht** "findet er den Ausgang", sondern **generalisiert er auf Seeds,
die er nie gesehen hat**. Ein Agent, der eine Karte auswendig lernt, hat nichts gelöst.

## Harte Kriterien

| Testset | Seeds | Ziel |
|---------|-------|------|
| A | 7000–7049 | ≥ 70 % Success Rate |
| B (Holdout) | 8000–8049 | ≥ 60 % Success Rate |

Zusätzlich: mindestens 3 Trainingsläufe pro Konfiguration, Ergebnis als Mittelwert ±
Standardabweichung. Kein Best-of-Runs — siehe [[henderson-2018]].

## ⚠️ Diese Kriterien sind eine REVISION des Exposés

Das **Exposé setzte ≥ 90 % über 100 unbekannte Seeds** an (unter *Reward Design und Evaluierung*,
nicht unter den *Projektzielen* — deren Muss- und Nice-to-Have-Punkte sind alle erfüllt und
tragen keine Prozentzahl). Die 70/60-Kriterien sind eine im Projektverlauf begründete Absenkung.

Drei Fakten machen die Revision verteidigbar (alle verifiziert 18.07.2026):

1. **Fixiert am 08.06.2026 im CHANGELOG** — einen Monat vor den finalen v12-Läufen (08.07.) und
   der n=7-Auswertung (17.07.). Keine Anpassung in Kenntnis der Endergebnisse.
2. **Begründet** durch Erkenntnisse, die beim Exposé fehlten: POMDP-Charakter ([[pomdp-charakter]]),
   Luftlinien-Bug (35–45 hieß real 42–75, [[v11-env-bruch]]), Streuung ±12 pp.
3. **Das alte Kriterium wird mitberichtet**: Gegen die Exposé-Metrik (A+B = exakt 100 Seeds) sind
   es **66,3 % ± 12,1** stoch (bester Lauf 80,0; det 30,9) gegen die 90-%-Schwelle — deutlich
   verfehlt, und das steht so in der Arbeit (§ Revision des Erfolgskriteriums).

Die Offenlegung ist Pflicht, nicht Kür: Stillschweigend angepasste Kriterien, als a priori
ausgegeben, wären HARKing (Kerr 1998, `kerr1998harking` in der `references.bib`).

> **Verworfen (18.07.):** Das Argument „unsere 66 % liegen über den publizierten 25–53 % für
> prozedurale Labyrinthe" — die Spanne erwies sich als Metrik-Verwechslung (Procgen berichtet
> normalisierten Return, keine SR; MiniGrid erreicht bis 100 %). Details:
> [[literatur-lstm-groesse]] (Abschnitt „25–53 %"). Nicht wiederverwenden.

## Stand (n=7, 17.07.2026 — maßgeblich)

| Testset | stoch | Ziel | |
|---------|-------|------|---|
| A | **65,7 % ± 12,4** | ≥ 70 % | **✗ verfehlt** |
| B | **66,9 % ± 12,8** | ≥ 60 % | ✓ erfüllt |

Deterministisch werden **beide** verfehlt (29,1 % / 32,6 %). Von den vier Kombinationen hält nur
**B stochastisch** — und dessen 95-%-CI reicht bis 55,0, also unter die Schwelle.

Dass die stochastische Zahl überhaupt zählt, folgt aus [[pomdp-charakter]] und ist keine
Bequemlichkeit — die Begründung muss in der Arbeit stehen, sonst wirkt es wie Rosinenpickerei.
Details, Einzelwerte und die ausgeschlossenen Alternativerklärungen: [[v12-final]].

> **Überholt:** Bis zum 17.07. galt "beide Ziele erfüllt" (n=3: A 73,3 / B 80,0). Die Aufstockung
> auf n=7 hat den Mittelwert **gesenkt**. Der n=3-Wert war zu optimistisch — was die Doku selbst
> vorhergesagt hatte (CI [66,7; 80,0] schloss die 70 %-Schwelle ein). Genau dafür fordert
> [[henderson-2018]] mehrere Seeds.

## Warum das der schwierige Teil ist

Der Agent sieht nur 15×15 Tiles um sich herum ([[observation-space]]). Der Exit liegt 35–45
BFS-Schritte entfernt und ist nie sichtbar. Ohne Gedächtnis ist die Aufgabe strukturell
unlösbar — genau das zeigt [[ablation-abc]].
