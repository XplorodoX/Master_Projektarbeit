# python/ — Importierbare Bibliothek (RL-Environment)

Dieser Ordner ist Teil des `PYTHONPATH` und enthält **nur importierbare Module** —
keine direkt ausführbaren Skripte. Ausführbare Skripte liegen in `../scripts/`.

```bash
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
```

---

## Inhalt

| Datei | Beschreibung |
|-------|-------------|
| `stoneforge_env.py` | **Gym-Environment** — `StoneforgeWorldEnv`, `SwarmSeedPool` |
| `stream_wrapper.py` | `StreamWrapper` — sendet Agenten-Positionen an die Live-Map (WS) |
| `doc_logger.py` | Automatische Ergebnis-Dokumentation (config/results/eval_history → RESULTS.md) |
| `cnn_extractor.py` | CNN-Feature-Extractor (für Visited-Mask-Experimente) |
| `stoneforge_sim.so` | Kompiliertes Pybind11-Modul (Unix) — auto-generiert, in `.gitignore` |
| `stoneforge_sim.pyd` | Kompiliertes Pybind11-Modul (Windows) — auto-generiert, in `.gitignore` |

> Die früheren Wrapper `ExitPotentialFieldWrapper` und `ReducedActionEnv` wurden entfernt:
> Der Aktionsraum ist seit Env v11 direkt im C++-Binding auf Bewegung (0–3) beschränkt,
> und BFS-Features gehören bewusst NICHT in die Observation (kein Orakel).

---

## Wichtigste Klassen

### `StoneforgeWorldEnv`

Basis-Gym-Environment. Verbindet Python mit der C++-Simulation.
Observation: **229 Features** (Grid 15×15 + HP + exitDx/exitDy + step_frac).
Aktionsraum: `Discrete(4)` — hoch / runter / links / rechts.

```python
from stoneforge_env import StoneforgeWorldEnv

env = StoneforgeWorldEnv(exit_min=35, exit_max=45)
obs, info = env.reset(seed=7000)
obs, reward, terminated, truncated, info = env.step(action)
```

**Legacy-Modelle** (vor 06.07.2026, 231-dim Obs, z. B. `ppo_lstm_curriculum`):

```python
env = StoneforgeWorldEnv(exit_min=35, exit_max=45, include_energy_inventory=True)
```

Hinweis: Die WorldGen-Konfiguration ist im C++-Kern prozess-global; das Env stempelt
seine eigene Konfiguration deshalb bei **jedem** `reset()` neu (mehrere Env-Instanzen
mit unterschiedlichen exit-Ranges sind dadurch sicher).

### `SwarmSeedPool`

Thread-sicherer Seed-Pool für Swarm-Training: erfolgreiche Seeds werden mit
Wahrscheinlichkeit `swarm_prob` beim Reset wiederholt (`plr_mode=True` invertiert
zu knapp-gescheiterten Seeds).

---

## Kompiliertes Binding bauen

Die `.so`/`.pyd`-Dateien werden automatisch in `../build/` erzeugt und
vom Launcher nach `python/` kopiert:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build --target stoneforge_sim -j
```
