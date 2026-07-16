---
id: recurrent-ppo
title: RecurrentPPO (LSTM) — Architektur und ihre Hyperparameter
type: concept
tags: [training, architektur, lstm, ppo]
related: [batch-size-8, e3-lstm512, pomdp-charakter, v7-v9-rootcause, pleines-2022]
updated: 2026-07-17
---

# RecurrentPPO (LSTM)

Aus `sb3-contrib`. Die Architekturentscheidung folgt direkt aus [[pomdp-charakter]]: Bei
partieller Beobachtbarkeit braucht der Agent ein Gedächtnis, sonst kann er den Belief State nicht
approximieren. Ein MLP ohne BFS-Orakel kollabiert auf 0 % ([[ablation-abc]]).

## Kernkonfiguration (die Zahlen, die funktionieren)

| Parameter | Wert | Warum |
|-----------|------|-------|
| `n_steps` | 256 | aus der v2-Konfiguration reproduziert |
| `batch_size` | **8** | kritisch — siehe [[batch-size-8]] |
| `n_epochs` | 10 | Default |
| `ent_coef` | 0.05 | 5× über dem SB3-Default; weniger → zu wenig Exploration |
| `lstm_hidden_size` | 256 | 512 ist schlechter, siehe [[e3-lstm512]] |
| `vf_coef` | 0.5 | 1.0 bringt netto nichts, siehe [[e1-critic]] |
| Device | **CPU** | MPS/GPU ist bei diesem Netz langsamer (gemessen 06.07.2026) |

## Gesunde Trainings-Signatur

Woran man erkennt, dass ein Lauf lernt (statt nur Zeit zu verbrennen):

- `approx_kl` ≈ 0.029 — die Policy verändert sich tatsächlich
- `clip_fraction` ≈ 0.25–0.31 — sie wird aktiv geclippt
- `explained_variance` > 0.85 — der Critic hat verstanden

Gegenbeispiel v7d: `approx_kl = 4.9e-07`. Die Policy hat sich schlicht **nicht bewegt**, der Lauf
war von Anfang an tot. Die Signatur ist ein billiger Rauchtest — sie nach ~20k Steps zu prüfen
spart acht Stunden Rechenzeit.

## Warum CPU

Das Netz ist klein, die LSTM-Sequenzen sind kurz. Der Overhead der Datenübertragung zur GPU frisst
den Rechenvorteil vollständig auf. Gemessen, nicht vermutet.

## Bekannte Grenzen

[[pleines-2022]] dokumentiert für Recurrent PPO in prozeduraler Navigation genau die
Symptome, die hier auftreten: Generalisierung erst mit vielen Trainings-Seeds, Scheitern jenseits
der Gedächtnis-Kapazität, Trainingsinstabilität. Das Projekt reproduziert diese Grenzen — es
scheitert nicht an ihnen aus Versehen.
