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

Der Mechanismus: Ein kleinerer `batch_size` zerlegt denselben Rollout-Puffer in mehr Minibatches →
**~8× mehr Gradientenschritte pro Rollout** (64 → 8). Der Critic bekommt genug Updates, um dem
bewegten Ziel zu folgen.

<details>
<summary>Nachrechnung (korrigiert 17.07.2026)</summary>

Der Puffer ist `n_steps × n_envs = 256 × 16 = 4096` Transitionen (Default `--n-envs 16`, verifiziert
in `train_curriculum.py:272`). Daraus folgt pro Epoche:

| `batch_size` | Minibatches/Epoche |
|---|---|
| 8 | 512 |
| 64 | 64 |
| 256 (v7–v9) | 16 |

Die frühere Fassung schrieb "32 statt 4" bzw. "ein Batch pro Epoche" — das rechnete mit
`n_steps=256` **ohne** `n_envs` und war damit um Faktor 16 daneben. Das **Verhältnis** (8×) stimmte
und trägt die Entscheidung; die Absolutzahlen waren falsch.
</details>

## Warum das zweimal passieren konnte

Die frühere Freigabe von 64 beruhte auf einem **Einzel-Snapshot** (52 % SR, EV 0.72), der zufällig
gut aussah. Er war ein Moment in einem chaotischen Prozess, kein Zustand. Nachgewiesen am
07.07.2026 mit 4 Läufen + A/B ([[batch-size-forensik]]), CHANGELOG `v2026-07-07.4`.

> **Die verallgemeinerte Lehre:** Hyperparameter-Entscheidungen nie auf einzelnen Snapshots, immer
> auf ganzen Eval-**Kurven**. Deshalb schreibt jeder Lauf `eval_history.json`.

## Konsequenz

Alle finalen v12-Läufe ([[v12-final]]) laufen mit `batch=8`. Der Wert steht in `RPPO_KWARGS` und im
`CLAUDE.md` mit Ausrufezeichen. Wer ihn ändern will, braucht eine **Kurve**, keinen Screenshot.
