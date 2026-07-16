---
id: train-curriculum
title: train_curriculum.py — das Trainingsskript
type: component
tags: [training, code, betrieb]
path: scripts/train_curriculum.py
related: [curriculum-learning, recurrent-ppo, nohup-training, swarm-seed-pool, reproduzierbarkeit]
updated: 2026-07-17
---

# train_curriculum.py

Der Weg, auf dem alle finalen Modelle entstanden sind.

```bash
nohup .venv/bin/python scripts/train_curriculum.py \
  --save-dir models/ppo_lstm_curriculum_v12_s1 \
  --seed 1 --n-envs 16 --no-live-map > logs/train_v12_seed1.log 2>&1 &
```

## Pflicht-Flags

- **`--save-dir` immer setzen.** Der Default ist `models/ppo_lstm_curriculum` — das ist das
  historische Referenzmodell. Ohne eigenes `--save-dir` überschreibt man es.
- **`nohup`** verwenden, sonst sterben die Läufe beim Schließen des Terminals →
  [[nohup-training]].
- **`--no-live-map --no-heatmap`** bei mehreren parallelen Läufen (Port 8766 kollidiert sonst).
- `OMP_NUM_THREADS=2` bei Parallelläufen — je Lauf ~120 % CPU / 432 MB, so passen 4 Läufe
  nebeneinander auf einen M1 Pro ohne Ressourcenkonkurrenz.

## Was das Skript schreibt

| Datei | Inhalt |
|-------|--------|
| `phase{1,2,3}_best_model.zip`, `best_model.zip` | Modelle je Phase |
| `config.json` | exakte Hyperparameter **+ `_git_commit`** (short hash, `-dirty`-Marker) |
| `results.json` | Trainingszeit |
| `eval_history.json` | Eval-Kurve über den Lauf (det + stoch je Punkt) |

Das Git-Hash-Stamping (`doc_logger.save_run_config`) bindet jeden Lauf an einen Code-Stand — eine
Anforderung aus [[pineau-2021]]. Die v12/E1/E2/E3-Läufe entstanden auf ~`97ab30d`.

## Laufzeit

Ein voller 4-Phasen-Lauf: **7,7–8,5 h** solo auf M1 Pro. Vier parallel: 8–12 h erwartet
(in der Praxis mehr, siehe [[projekt-status]]).

## Rauchtest nach ~20k Steps

`approx_kl ≈ 0.029`, `clip_fraction ≈ 0.25` → gesunde v2/v10-Signatur, der Lauf lernt.
Fehlt die, lieber sofort abbrechen als acht Stunden zu verbrennen. Details: [[recurrent-ppo]].
