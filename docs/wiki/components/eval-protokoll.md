---
id: eval-protokoll
title: Standardisiertes Eval-Protokoll
type: component
tags: [evaluation, protokoll, reproduzierbarkeit]
path: scripts/eval_comparison.py
related: [zielkriterium, eval-cap-4000, stochastische-eval, obs-shape-legacy, temperatur-sweep]
updated: 2026-07-17
---

# Eval-Protokoll

Jede berichtete Zahl entsteht so — Abweichungen machen Läufe unvergleichbar.

| Parameter | Wert |
|-----------|------|
| Testset A | Seeds 7000–7049 |
| Holdout B | Seeds 8000–8049 |
| Exit-Distanz | 35–45 |
| Episoden-Cap | **4000** (= Env-`maxSteps`) — siehe [[eval-cap-4000]] |
| Modi | deterministisch **und** stochastisch, beide berichtet |
| Aggregation | Mittelwert ± Std, `ddof=1` (Stichprobe!) |

A und B sind **disjunkt** und fest. Beide werden immer zusammen berichtet — dass B (77–80 %)
regelmäßig **über** A (73 %) liegt, ist selbst ein Ergebnis: Die Seed-Auswahl erzeugt mehr Varianz
als der Train/Holdout-Unterschied. Das ist ein Argument **gegen** Overfitting und stützt die
Generalisierungs-These.

## Ausführung

```bash
python scripts/eval_comparison.py              # deterministisch
python scripts/eval_comparison.py --stochastic # stochastisch
```

## Fallstricke, die die Zahlen still verfälschen

1. **Cap < 4000** → SR wird massiv unterschätzt (48 % @600 vs. 86 % @4000, gleiches Modell).
   Kostet nur ~21 s pro Eval. Nie sparen. → [[eval-cap-4000]]
2. **`ddof=0`** statt 1 → zu kleine Std bei n=3. Wir haben eine Stichprobe, keine Grundgesamtheit.
3. **LSTM-State** muss über die Episode korrekt mitgeführt und bei `reset()` genullt werden.
   Verifiziert 09.07.2026.
4. **Obs-Shape** des Modells prüfen, sonst Shape-Mismatch → [[obs-shape-legacy]].
5. **Einzelne Snapshots sind keine Validierung.** Die Lerndynamik kann chaotisch sein; transiente
   Hochphasen täuschen. Entscheidungen nur auf ganzen Eval-**Kurven** treffen — diese Lehre hat
   das Projekt eine falsche Hyperparameter-Freigabe gekostet ([[batch-size-8]]).

## Warum stochastisch die Primärmetrik ist

Siehe [[stochastische-eval]] und [[pomdp-charakter]]. Der deterministische Wert wird trotzdem
immer mitberichtet — er ist die Messung der Belief-Qualität und damit selbst ein Befund
([[det-stoch-gap]]).
