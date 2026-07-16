---
id: v7-v9-rootcause
title: v7–v9 — vier gescheiterte Läufe und die Root-Cause-Analyse
type: experiment
tags: [negativergebnis, rootcause, lehre]
related: [batch-size-8, recurrent-ppo, observation-space, v12-final]
status: widerlegt
updated: 2026-07-17
---

# v7–v9: Die teuerste Lehre des Projekts

Sechs Läufe (v7, v7b, v7c, v7d, v8, v9) sind gescheitert. Alle aus demselben Grund — und der
Grund war nicht der, den wir gesucht haben.

## Was wir dachten

Der [[det-stoch-gap]] sei ein **Feature-Problem**. Also haben wir Features hinzugefügt:
`visit_count`, `action_buffer_len=4`, `last_reward` → Observation auf 249 Dimensionen aufgeblasen.
Jeder "Fix" machte die Observation größer. Keiner half.

## Was tatsächlich los war

Root Cause gefunden am 15.06.2026, indem wir das **funktionierende** v2-Modell geladen und per
`inspect` seine echte Konfiguration ausgelesen haben:

```python
RecurrentPPO.load("models/ppo_lstm_curriculum_v2/phase1_best_model.zip")
```

| Parameter | v7–v9 (kaputt) | v2 (86 %) |
|-----------|----------------|-----------|
| `n_steps` | 512 / 128 | **256** |
| `batch_size` | 256 | **8** |
| `ent_coef` | 0.01 | **0.05** |
| Obs | 249 | **231** |

`batch_size=256` zerlegt den Rollout-Puffer (`n_steps × n_envs = 4096`) in nur **16 Minibatches pro
Epoche**; mit `batch_size=8` sind es **512** — 32× mehr Gradientenschritte. Der Critic bekam schlicht
zu wenige Updates. Und `ent_coef=0.01` statt 0.05 nahm dem Agenten die Exploration.
(Zahlen korrigiert 17.07.2026 — die frühere Fassung rechnete ohne `n_envs=16`, siehe
[[batch-size-8]].)

## Die Diagnose war im Log sichtbar

v7d: `approx_kl = 4.9e-07`. Die Policy hat sich über den gesamten Lauf **nicht verändert**.
Zum Vergleich die v10-Reproduktion mit v2-Hyperparametern: `approx_kl = 0.029`,
`clip_fraction = 0.31`, `explained_variance = 0.231 @ 20k` — wo v7–v9 maximal **0.024** erreichten.
Der Critic hat nie etwas verstanden.

## Die Lehre

**Wir haben die Symptome behandelt, statt die Konfiguration zu prüfen.** Sechs Läufe à mehrere
Stunden. Der Fix war ein Vergleich mit der letzten funktionierenden Konfiguration — 20 Minuten
Arbeit, wenn man auf die Idee kommt.

Daraus folgen zwei Routinen, die heute gelten:
1. **Rauchtest nach 20k Steps** (`approx_kl`, `clip_fraction`, `explained_variance`) statt acht
   Stunden blind zu warten.
2. **Bei Regression zuerst gegen den letzten funktionierenden Lauf diffen**, nicht Hypothesen bauen.

Und noch eine, die später doppelt schmerzte: `batch_size` blieb trotzdem strittig →
[[batch-size-8]].
