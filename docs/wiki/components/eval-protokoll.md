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
| Aggregation | Mittelwert ± Std über die Läufe, **`ddof=1`** (vereinheitlicht 18.07.2026) |

## ✅ Behoben (18.07.2026): ddof war inkonsistent (gefunden 17.07.2026)

**Erledigt:** Die Doku rechnet jetzt durchgehend `ddof=1` — die n=7-Tabelle (`tab:v12`) tat das
schon, die im Text zitierte n=3-Zwischenauswertung wurde nachgezogen (A 73,3 ± 8,3 / B 80,0 ± 8,0
statt ± 6,8 / ± 6,5). CHANGELOG `v2026-07-18`. Der ursprüngliche Befund bleibt unten als Protokoll
stehen.

Die berichteten v12-Zahlen verwenden **`ddof=0`** (Populations-Std), nicht `ddof=1`. Nachgerechnet
aus den Einzelwerten des CHANGELOG:

| Größe | Einzelwerte | berichtet | = ddof | mit `ddof=1` wäre es |
|-------|-------------|-----------|--------|---------------------|
| A stoch | 64 / 76 / 80 | 73,3 ± **6,8** | 0 | ± **8,3** |
| A det | 38 / 26 / 32 | 32,0 ± **4,9** | 0 | ± **6,0** |
| B stoch | 72 / 80 / 88 | 80,0 ± **6,5** | 0 | ± **8,0** |
| B det | 46 / 44 / 36 | 42,0 ± **4,3** | 0 | ± **5,3** |

Die Nachmessung vom 15.07. rechnet dagegen **mit `ddof=1`** (A 72,7 ± 9,5). Zwei Messungen desselben
Modells, zwei Rechenwege — die ±Werte sind untereinander nicht vergleichbar.

**Warum das zählt:** Bei n=3 sind drei Läufe eine **Stichprobe**, keine Grundgesamtheit — `ddof=1`
ist die korrekte Wahl, und `ddof=0` **unterschätzt die Streuung systematisch**. Das trifft
ausgerechnet die Zahl, an der [[zielkriterium]] A hängt: Der Abstand zum 70-%-Ziel beträgt 3,3
Punkte, die Streuung ± 6,8 (ddof=0) oder ± 8,3 (ddof=1).

**Entwarnung:** Die Doku hängt ihre Aussage nicht am ±Std, sondern an einem **95-%-Bootstrap-CI**
([66,7; 80,0] auf A, stratifiziert, 10 000 Resamples) — und benennt dort ausdrücklich, dass das
Intervall die 70-%-Schwelle einschließt. Die inhaltliche Aussage ist also robust; die Inkonsistenz
betrifft die ±Angabe daneben.

**To-do (erledigt 18.07.):** `ddof=1` festgelegt, n=3-Textstellen in der Doku nachgezogen.

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
2. **`ddof`** einheitlich halten — siehe den Befund oben. Bei n=3 ist `ddof=1` korrekt.
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
