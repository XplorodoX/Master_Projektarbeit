# models/ — Trainierte RL-Modelle

Alle trainierten Modelle des Projekts. Jeder Unterordner ist ein Experiment.

## Modell-Übersicht

| Ordner | Algorithmus | Beschreibung | Beste SR |
|--------|-------------|-------------|----------|
| `ppo_baseline/` | PPO | Erste Baseline, Phase 1 | ~56% |
| `ppo_bfscompass/` | PPO | Mit BFS-Compass-Feature (Delta-Encoding) | — |
| `ppo_bfscompass_1m/` | PPO | BFS-Compass, 1M Timesteps | — |
| `ppo_delta_v1/` | PPO | Delta-BFS-Encoding v1 | — |
| `ppo_delta_v2/` | PPO | Delta-BFS-Encoding v2 | — |
| `ppo_phase3/` | PPO | Phase-3-Training (3 Runs) | — |
| `ppo_phase3_run2/` | PPO | Phase-3, Run 2 | — |
| `ppo_phase3_run3/` | PPO | Phase-3, Run 3 | — |
| `ppo_phase4/` | PPO | Phase-4 (Hard-World-kompatibel) | — |
| `ppo_phase5/` | PPO | Phase-5 (aktuell bestes Modell) | — |

## Neue Modelle (nach Training)

Neue Trainingsläufe speichern automatisch in:
```
models/ppo/        ← python train.py --algo ppo
models/dqn/        ← python train.py --algo dqn
models/a2c/        ← python train.py --algo a2c
```

Eigener Ordner per `--save-dir`:
```bash
python scripts/train.py --algo ppo --save-dir models/mein_experiment
```

## Inhalt pro Ordner

```
models/<name>/
├── best_model.zip    ← Bestes Modell (nach Success Rate während Training)
└── final_model.zip   ← Finales Modell (letzter Checkpoint nach Training)
```

## Evaluation

```bash
# Schnelltest eines Modells (via Launcher)
python scripts/launcher_gui.py  # → Evaluation → Modell wählen

# CLI
python scripts/watch_agent.py --model models/ppo_phase5/best_model.zip --seed 7000
```

## TensorBoard

```bash
tensorboard --logdir logs/tensorboard/
```
