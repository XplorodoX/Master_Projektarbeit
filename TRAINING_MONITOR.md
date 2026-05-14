# BFS Training Monitor - Quick Start

## Aktueller Training Status

**Training Run:** `dqn_run_18` (gestartet 14.05.2026)  
**Start Reward:** -27.4  
**Config:** v1.2.0 + BFS Distance + observationRadius=7  

## Live Monitoring Options

### Option 1: TensorBoard (grafisch)
```bash
tensorboard --logdir tensorboard_logs/dqn_run_18
# → http://localhost:6006
```
Zeigt: Rewards, Episode Lengths, Learning Rate, Loss.

### Option 2: Python Monitor Script (automatische Eval)
```bash
cd /Users/merluee/Master_Projektarbeit
source .venv/bin/activate
export PYTHONPATH=$PWD/python
python monitor_training_bfs.py
```
Wertet alle 5 Minuten das aktuelle `best_model.zip` auf 50 Seeds aus.  
Speichert Ergebnisse in `dqn_run_18_eval_log.json`.

### Option 3: Manuelle Eval (jederzeit)
```bash
python eval_baseline_bfs.py
```
Schnelle 50-Seed-Eval des besten Modells.

## Zielkriterium

- **Success Rate ≥ 70%** auf Seeds 7000-7049 → Training erfolgreich
- **Erwartet:** Nach 500K-1M Steps sollte das Ziel erreicht sein

## Expected Training Curve

```
Phase 1 (0-100K):   Reward -27 → -15 (Exploration, Grid Learning)
Phase 2 (100K-500K): Reward -15 → +10 (BFS Navigation, Success Rate 0→30%)
Phase 3 (500K-1M):  Reward +10 → +40 (Feintuning, Success Rate 30→70%+)
```

Falls Training nicht konvergiert nach 1M Steps: Siehe Troubleshooting in `Projektarbeit_RL_Dokumentation.md` Sektion 6g.

## Training abbrechen

```bash
# Finde Terminal ID
jobs

# Breche ab
kill %1
```

Der beste Checkpoint wird in `best_models_dqn/best_model.zip` automatisch gespeichert.
