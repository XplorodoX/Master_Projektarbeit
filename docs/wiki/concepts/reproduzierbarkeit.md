---
id: reproduzierbarkeit
title: Reproduzierbarkeit — Ist-Stand und offene Punkte
type: concept
tags: [methodik, reproduzierbarkeit, projektarbeit]
related: [pineau-2021, henderson-2018, train-curriculum, rebuild-pflicht, eval-protokoll, v12-final, testset-leakage, projekt-status]
updated: 2026-07-17
---

# Reproduzierbarkeit

Gemessen an [[pineau-2021]] und [[henderson-2018]]. Wichtiger, als er wirkt: Der Abschnitt holt bei
"wissenschaftlicher Sauberkeit" viele Punkte für wenig Arbeit — vor allem, weil das Meiste **schon
getan ist** und nur benannt werden muss.

> Korrigiert am 17.07.2026: Zwei Punkte standen hier als "offen", die **erledigt** sind
> (Bootstrap-CIs, Leakage-Fix), und einer war **überzeichnet** (Git-Stamping). Details unten.

## Erfüllt (im Text aktiv als bewusste Methodik verkaufen)

- ✓ **3 Seeds, Mittelwert ± Std** ([[v12-final]]) — Hendersons Forderung, kein Best-of.
  Aufstockung auf n=7 läuft ([[projekt-status]]).
- ✓ **95-%-Bootstrap-CI** (stratifiziert, 10 000 Resamples) in der Endergebnis-Tabelle
  `tab:v12` — **war fälschlich als offen gelistet, ist längst drin.** Die Doku benennt damit selbst,
  dass das CI auf A die 70-%-Schwelle einschließt.
- ✓ **Drei disjunkte Seed-Bereiche**: Val 6000–6049 (Selektion), Testset A 7000–7049, Holdout B
  8000–8049 (nur finale Eval). Data-Leakage seit 11.06.2026 behoben → [[testset-leakage]].
- ✓ **CHANGELOG** mit jeder Änderung (Problem/Lösung/Ergebnis) **inklusive Negativergebnissen** →
  deckt Pineaus "durchsuchter Hyperparameter-Bereich" ab.
- ✓ **`config.json` je Lauf** mit exakten Hyperparametern + `results.json` (Trainingszeit) +
  `eval_history.json` (ganze Eval-Kurve, nicht nur Endpunkt).
- ✓ **Standardisiertes Eval-Protokoll** ([[eval-protokoll]]).
- ✓ **Versionen gepinnt**: `requirements.txt` mit `==` (inkl. der zuvor **fehlenden** sb3-contrib
  und torch), getestet mit Python 3.12.13 auf macOS arm64, + `requirements.lock.txt` (pip freeze).
- ✓ **Reproducibility-Anhang** in `Projektdokumentation.tex`: Systemumgebungs-Tabelle +
  Pineau/Henderson-Checkliste.
- ✓ **Setup-Tabelle**: Basiskonfiguration + Iterationen (v12/E1/E2/E3) in der Methodik.

## ⚠️ Git-Hash-Stamping — schwächer als behauptet (geprüft 17.07.2026)

Die frühere Fassung sagte: *"✓ Git-Hash-Stamping … v12/E1/E2/E3 laufen auf ~97ab30d."* Das
suggeriert, die berichteten Läufe seien an einen Code-Stand **gestempelt**. Sind sie nicht:

| Lauf | `_git_commit` in `config.json` |
|------|-------------------------------|
| `v12_s1` (berichtet) | **fehlt** |
| `exp_E3_lstm512` (berichtet) | **fehlt** |
| `v12_s4` (n=7-Aufstockung) | `3c8fa26-dirty` |

`doc_logger.save_run_config` schreibt `_git_commit` erst **seit 09.07.2026** — also **nach** den
v12- und E1/E2/E3-Läufen. Die Angabe "~97ab30d" ist eine **nachträgliche Zuschreibung** aus dem
Gedächtnis, kein Artefakt.

**Ehrliche Formulierung:** Die berichteten Läufe sind über CHANGELOG-Datum und `config.json` an den
damaligen Stand gebunden, aber nicht kryptografisch gestempelt; ab 09.07. ist das Stamping aktiv.
Das ist immer noch besser als bei den meisten Arbeiten — nur eben nicht das, was behauptet wurde.

**Zusatzbefund für n=7:** `v12_s4` trägt `3c8fa26-dirty`, die Läufe s1–s3 liefen auf ~`97ab30d`.
Ein n=7-Mittel mischt damit **zwei Code-Stände** — siehe [[projekt-status]].

## Projektspezifisch, was Standardchecklisten nicht abdecken

Der **C++-Build ist Teil der Umgebung**. Ein Experiment ist nur reproduzierbar mit Commit-Hash
**und** `game_config.json`-Stand — sonst rekonstruiert man eine andere Welt ([[rebuild-pflicht]],
[[v11-env-bruch]]). Das unterscheidet diese Arbeit von einem reinen Python-Projekt und gehört
explizit in den Anhang.

## Rest-Nichtdeterminismus (ehrlich benennen)

torch/numpy/env-Seeds werden gesetzt (`--seed`). Verbleibende Quellen: **DummyVecEnv-Reihenfolge**
und **CPU-Threading**. Bit-genaue Reproduktion ist damit nicht garantiert — statistische schon.

## Tatsächlich offen

1. **`ddof` vereinheitlichen.** Die berichteten ±Std sind `ddof=0`, die Nachmessung nutzt `ddof=1`.
   → [[eval-protokoll]]
2. **Ein-Kommando-Reproduktion** ("so entsteht Tabelle X": Build → PYTHONPATH → train → eval). Die
   Bausteine stehen im `CLAUDE.md`, es fehlt die Zusammenfassung.
3. **Code-Stand-Mix bei n=7** klären (s1–s3 vs. s4–s7), siehe oben.
