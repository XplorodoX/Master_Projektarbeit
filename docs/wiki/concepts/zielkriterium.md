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

## Stand

Beide Ziele sind mit [[v12-final]] im Mittel **stochastisch erfüllt** (A 73,3 % ± 6,8 · B 80,0 % ± 6,5
bei n=3). Deterministisch werden beide **verfehlt** (32 % / 42 %). Dass die stochastische Zahl
zählt, ist keine Bequemlichkeit, sondern folgt aus [[pomdp-charakter]] — die Begründung muss in
der Arbeit stehen, sonst wirkt es wie Rosinenpickerei.

⚠️ Bei Testset A ist der Abstand zum Ziel (2,7 Punkte) **kleiner als die Streuung** (9,5 bei n=3
gegen Env v11). "Robust über 70 %" ist damit nicht belegbar — deshalb läuft die Aufstockung auf
n=7, siehe [[projekt-status]].

## Warum das der schwierige Teil ist

Der Agent sieht nur 15×15 Tiles um sich herum ([[observation-space]]). Der Exit liegt 35–45
BFS-Schritte entfernt und ist nie sichtbar. Ohne Gedächtnis ist die Aufgabe strukturell
unlösbar — genau das zeigt [[ablation-abc]].
