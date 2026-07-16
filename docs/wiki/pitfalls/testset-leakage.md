---
id: testset-leakage
title: Test-Set-Leakage — historisch, seit 11.06.2026 BEHOBEN
type: pitfall
tags: [methodik, evaluation, historisch, korrektur]
related: [curriculum-learning, eval-protokoll, zielkriterium, train-curriculum, v12-final]
status: behoben
updated: 2026-07-17
---

# Test-Set-Leakage (P0.1) — behoben

> ⚠️ **Dieser Eintrag war bis zum 17.07.2026 falsch.** Er behauptete, das Leakage sei "methodisch
> bekannt, nicht behoben", und empfahl, es in der Arbeit als offenen Mangel zu beichten. **Das
> Gegenteil stimmt.** Der Fehler kam aus `docs/IMPROVEMENT_PLAN.md` (Stand 11.06.), der die Lücke
> als P0.1 auflistet — behoben wurde sie noch **am selben Tag**, was der Plan naturgemäß nicht mehr
> vermerkt.

## Das ursprüngliche Problem

Der Curriculum-Callback evaluierte und **selektierte** das `best_model` auf den Seeds 7000–7049 —
also auf Testset A ([[zielkriterium]]). Damit war A kein sauberes Testset, sondern faktisch ein
Validierungsset: Wer über Hunderte Checkpoints den besten auf A auswählt, überträgt Information aus
A ins Modell, auch ohne je darauf zu trainieren.

## Der Fix (CHANGELOG `v2026-06-11`, "Änderung 1 — Validierungs-Seeds & Beseitigung des Data-Leakages")

```python
# scripts/train_curriculum.py:37
VAL_SEEDS = list(range(6000, 6050))
```

Phasensteuerung **und** Modellselektion laufen seither ausschließlich über diese Validierungs-Seeds.
Die Testsets werden nur noch in der finalen Evaluation angefasst. Im Code verifiziert am 17.07.2026
(`train_curriculum.py:37`, `:132–134`, `:161`).

**Damit sind drei disjunkte Seed-Bereiche im Einsatz:**

| Bereich | Seeds | Rolle |
|---------|-------|-------|
| Validierung | 6000–6049 | Phasenwechsel + Modellselektion |
| Testset A | 7000–7049 | Evaluation |
| Holdout B | 8000–8049 | Evaluation |

Das ist die saubere Dreiteilung, die man sich wünscht — und sie ist in
`docs/Projektdokumentation.tex` bereits so dokumentiert.

## Wichtig für die Arbeit: das ist ein Pluspunkt, kein Makel

Alle berichteten v12-Zahlen ([[v12-final]]) entstanden **nach** dem Fix. Das heißt:

- **Testset A ist ein echtes Testset.** Keine Selektion darauf.
- Der Fix ist **im CHANGELOG mit Datum und Begründung dokumentiert** — also nachweisbar, nicht
  behauptet.
- Dass das Leakage **selbst gefunden und behoben** wurde, ist genau die Methodenkompetenz, die eine
  Projektarbeit zeigen soll. Das gehört aktiv erzählt, nicht stillschweigend richtig gemacht.

Die frühere Empfehlung ("B als Hauptzahl führen, Leakage offen beichten") ist damit **hinfällig**.
A und B sind beide sauber; beide werden berichtet.

Auch die alte Entlastungserzählung ("B > A beweist, dass das Leakage klein ist") wird nicht mehr
gebraucht — und sie war ohnehin schwächer als die Erklärung, die inzwischen in der Doku steht:
Testset A enthält zufällig mehr Welten mit langem Laufweg (38 % gegenüber 32 % mit ≥ 42 Feldern).
Siehe [[v12-final]].

## Lehre für das Wiki

Ein Verbesserungsplan ist eine **Momentaufnahme von Problemen**, keine Statusquelle. Wer daraus
Status ableitet, hält jeden je gefundenen Fehler für ewig offen. Status kommt aus Code und
CHANGELOG — beides wurde hier zuerst nicht geprüft.
