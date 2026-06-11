# videos/ — Demo-Videos

Dieser Ordner enthält Demo- und Präsentationsvideos des Stoneforge-RL-Projekts.

## Empfohlene Videos für die Projektabgabe

| Datei | Inhalt | Empf. Länge |
|-------|--------|-------------|
| `demo_agent.mp4` | KI-Agent löst mehrere Karten | 1–2 min |
| `demo_training.mp4` | Training + TensorBoard-Live | 1–2 min |
| `demo_launcher.mp4` | Launcher GUI Walkthrough | 1 min |
| `demo_evaluation.mp4` | 50-Seed-Evaluation im Launcher | 1 min |

## Videos aufnehmen

### macOS — QuickTime

1. QuickTime öffnen → Neue Bildschirmaufnahme
2. Stoneforge starten:
   ```bash
   python scripts/launcher_gui.py
   # oder
   python scripts/watch_agent.py --model models/ppo_phase5/best_model.zip
   ```
3. Aufnahme starten, Demo durchführen, beenden.

### macOS — OBS Studio (empfohlen für bessere Qualität)

```bash
brew install --cask obs
```

### Empfohlene Export-Einstellungen

- Format: MP4 (H.264)
- Auflösung: 1280×720 oder 1920×1080
- FPS: 30
