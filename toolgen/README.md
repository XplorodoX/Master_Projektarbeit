# toolgen/ — Wartungsskripte

Hilfsskripte zur Pflege und Aufbereitung des Projekts.  
Trainings- und Evaluationsskripte liegen in `../scripts/`.

---

## Enthaltene Skripte

| Datei | Beschreibung |
|-------|-------------|
| `fix_changelog.py` | Bereinigt und reformatiert den CHANGELOG.md |

### `fix_changelog.py` — Verwendung

```bash
python toolgen/fix_changelog.py
```

Bereinigt `CHANGELOG.md` — doppelte Einträge, Formatierungsfehler.

---

## Training & Evaluation (→ `../scripts/`)

```bash
# Training starten
python scripts/train.py --algo ppo --timesteps 1000000

# Modell beobachten
python scripts/watch_agent.py --model models/ppo_phase5/best_model.zip

# Verhaltensanalyse
python scripts/analyze_agent.py --model models/ppo_phase5/best_model.zip

# Hard-World-Eval
python scripts/eval_hard_world.py --model models/ppo_phase5/best_model.zip

# Temperatur-Benchmark
python scripts/eval_temperature.py --model models/ppo_phase5/best_model.zip
```

## Grafischer Launcher (→ `../scripts/`)

```bash
python scripts/launcher_gui.py
```
