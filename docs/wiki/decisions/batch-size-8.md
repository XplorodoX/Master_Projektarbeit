---
id: batch-size-8
title: Entscheidung — batch_size=8, nicht 64
type: decision
tags: [hyperparameter, entscheidung, kritisch]
related: [recurrent-ppo, batch-size-forensik, v7-v9-rootcause, v12-final, eval-protokoll]
status: bestätigt
decided: 2026-07-07
updated: 2026-07-17
---

# batch_size = 8 (NICHT 64)

Der wichtigste einzelne Hyperparameter im Projekt. Zweimal falsch gesetzt, zweimal teuer.

## Entscheidung

`batch_size = 8` bei `n_steps = 256`.

## Begründung

`batch_size=64` **destabilisiert den LSTM-Critic**:

| | batch=64 | batch=8 |
|---|---|---|
| `explained_variance` | ≈ 0.1 (Critic lernt nie) | > 0.85 (v10-Repro: **0.939**) |
| SR-Verlauf | oszilliert 10–74 %, keine Konvergenz | stabiler Anstieg |

Der Mechanismus: Bei `n_steps=256` und `batch=8` gibt es **32 Gradientenschritte pro Rollout**
statt 4 — rund 8× mehr. Der Critic bekommt genug Updates, um dem bewegten Ziel zu folgen. Bei
`batch=256` (v7–v9) war es sogar nur **ein** Batch pro Epoche — Full-Batch-Gradientenabstieg unter
falschem Namen ([[v7-v9-rootcause]]).

## Warum das zweimal passieren konnte

Die frühere Freigabe von 64 beruhte auf einem **Einzel-Snapshot** (52 % SR, EV 0.72), der zufällig
gut aussah. Er war ein Moment in einem chaotischen Prozess, kein Zustand. Nachgewiesen am
07.07.2026 mit 4 Läufen + A/B ([[batch-size-forensik]]), CHANGELOG `v2026-07-07.4`.

> **Die verallgemeinerte Lehre:** Hyperparameter-Entscheidungen nie auf einzelnen Snapshots, immer
> auf ganzen Eval-**Kurven**. Deshalb schreibt jeder Lauf `eval_history.json`.

## Konsequenz

Alle finalen v12-Läufe ([[v12-final]]) laufen mit `batch=8`. Der Wert steht in `RPPO_KWARGS` und im
`CLAUDE.md` mit Ausrufezeichen. Wer ihn ändern will, braucht eine **Kurve**, keinen Screenshot.
