# logs/ — Alle Laufzeit- und Trainings-Logs

Enthält zwei Kategorien von Logs, die während des Experiments anfallen:

---

## Unterordner

### `tensorboard/`

TensorBoard-Trainingsverläufe. Jeder Trainingsrun erzeugt einen Unterordner
(`ppo_run_*/`, `dqn_run_*/`, `a2c_run_*/`).

```bash
tensorboard --logdir logs/tensorboard/
```

Der Ordner wird im Repo getrackt (`logs/tensorboard/` ist NICHT in `.gitignore`).

### `runtime/`

Laufzeit-Logs des Spielclients (`game.log`, `game.pid`).  
Wird von `.gitignore` ausgeschlossen — ändert sich bei jedem Spielstart.

---

## Was wird woher geschrieben?

| Quelle | Ziel |
|--------|------|
| `scripts/train.py` → Stable-Baselines3 | `logs/tensorboard/` |
| Spielclient (`stoneforge_client`) | `logs/runtime/game.log` |
| Launcher-GUI | nur Terminal-Ausgabe, kein File |
