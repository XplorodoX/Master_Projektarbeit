# screenshots/ — Screenshots

Dieser Ordner enthält Screenshots der Stoneforge-RL-Anwendung.

## Kategorien

| Unterordner | Inhalt |
|-------------|--------|
| `launcher/` | Screenshots des GUI-Launchers |
| `game/` | Screenshots des laufenden Spiels |
| `training/` | TensorBoard-Kurven, Trainingsverläufe |
| `evaluation/` | Evaluationsergebnisse, Diagramme |

## Screenshots erstellen

```bash
# Launcher starten
python scripts/launcher_gui.py

# Spiel mit KI starten
python scripts/watch_agent.py --model models/ppo_phase5/best_model.zip --seed 7000
```

Screenshots können mit dem System-Screenshot-Tool erstellt werden:
- **macOS**: `Cmd + Shift + 4` (Bereich auswählen)
- **Windows**: `Win + Shift + S`
- **Linux**: `gnome-screenshot` oder `scrot`
