# assets/ — Spielkonfiguration

Enthält Konfigurationsdaten die vom Spiel zur Laufzeit geladen werden.

> **Wichtig:** Dieser Ordner muss am Projektwurzel liegen.  
> Der C++-Code liest `assets/base/game_config.json` als relativen Pfad  
> vom aktuellen Arbeitsverzeichnis. Nicht verschieben.

---

## Inhalt

### `base/game_config.json`

Zentrale Konfigurationsdatei für Simulation und RL-Training.

| Parameter | Wert (v1.1) | Beschreibung |
|-----------|-------------|--------------|
| `observationRadius` | 7 | → 15×15 Sichtfeld des Agenten |
| `maxSteps` | 4000 | Maximale Schritte pro Episode |
| `exitMinDistance` | 5–35 | Mindestdistanz Exit (Curriculum-abhängig) |
| `exitMaxDistance` | 12–45 | Maximaldistanz Exit (Curriculum-abhängig) |
| `forceGuaranteedPath` | `true` | Garantiert lösbaren Pfad zum Exit |

> **Rebuild nötig** nach Änderung von `observationRadius` oder anderen  
> strukturellen Parametern! Die Observation-Shape ist zur Compile-Zeit fest.

```bash
cmake --build build -j
```

---

## Wer liest diese Datei?

| Komponente | Wie |
|-----------|-----|
| `src/python/py_module.cpp` | Pybind11-Binding beim Modul-Import |
| `src/apps/headless_main.cpp` | Headless-Runner beim Start |
| `src/client/render_engine.cpp` | Grafischer Client beim Start |
| `python/stoneforge_env.py` | indirekt (über Pybind11-Binding) |
