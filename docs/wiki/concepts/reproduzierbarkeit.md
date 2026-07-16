---
id: reproduzierbarkeit
title: Reproduzierbarkeit — Ist-Stand und offene Punkte
type: concept
tags: [methodik, reproduzierbarkeit, projektarbeit]
related: [pineau-2021, henderson-2018, train-curriculum, rebuild-pflicht, eval-protokoll, v12-final]
updated: 2026-07-17
---

# Reproduzierbarkeit

Gemessen an [[pineau-2021]] und [[henderson-2018]]. Der Abschnitt ist wichtiger, als er wirkt: Er
holt bei "wissenschaftlicher Sauberkeit" viele Punkte für wenig Arbeit — vor allem, weil das
Meiste **schon getan ist** und nur benannt werden muss.

## Erfüllt (im Text aktiv als bewusste Methodik verkaufen)

- ✓ **3 Seeds, Mittelwert ± Std, `ddof=1`** — kein Best-of ([[v12-final]]).
- ✓ **CHANGELOG** mit jeder Änderung (Problem/Lösung/Ergebnis) **inklusive Negativergebnissen** →
  deckt Pineaus "durchsuchter Hyperparameter-Bereich" ab.
- ✓ **`config.json` je Lauf** mit exakten Hyperparametern + `results.json` (Trainingszeit).
- ✓ **Git-Hash-Stamping**: `doc_logger.save_run_config` schreibt `_git_commit` (short hash +
  `-dirty`) in jede `config.json`. v12/E1/E2/E3 laufen auf ~`97ab30d`.
- ✓ **Feste, disjunkte Eval-Seeds** + standardisiertes Protokoll ([[eval-protokoll]]).
- ✓ **Versionen gepinnt**: `requirements.txt` mit `==` (inkl. der zuvor **fehlenden** sb3-contrib
  und torch!) + `requirements.lock.txt` (pip freeze).
- ✓ **Reproducibility-Anhang** in `Projektdokumentation.tex`: Systemumgebungs-Tabelle +
  Pineau/Henderson-Checkliste.
- ✓ **Setup-Tabelle**: Basiskonfiguration + Iterationen (v12/E1/E2/E3) in der Methodik.

## Projektspezifisch, was Standardchecklisten nicht abdecken

Der **C++-Build ist Teil der Umgebung**. Ein Experiment ist nur reproduzierbar mit Commit-Hash
**und** `game_config.json`-Stand — sonst rekonstruiert man eine andere Welt ([[rebuild-pflicht]],
[[v11-env-bruch]]). Das ist der Punkt, an dem sich diese Arbeit von einem reinen Python-Projekt
unterscheidet, und gehört explizit in den Anhang.

## Rest-Nichtdeterminismus (ehrlich benennen)

torch/numpy/env-Seeds werden gesetzt (`--seed`). Verbleibende Quellen: **DummyVecEnv-Reihenfolge**
und **CPU-Threading**. Bit-genaue Reproduktion ist damit nicht garantiert — statistische schon.

## Offen (optional)

- Bootstrap-95%-CIs in der Endergebnis-Tabelle statt nur ±Std.
- Ein-Kommando-Reproduktion ("so entsteht Tabelle X": Build → PYTHONPATH → train → eval). Die
  Bausteine stehen im `CLAUDE.md`, es fehlt nur die Zusammenfassung.
