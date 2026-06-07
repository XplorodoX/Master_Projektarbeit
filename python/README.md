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
| `stoneforge_env.py` | **Gym-Environment** — `StoneforgeWorldEnv`, `ExitPotentialFieldWrapper`, `ReducedActionEnv` |
| `stoneforge_sim.so` | Kompiliertes Pybind11-Modul (Unix) — auto-generiert, in `.gitignore` |
| `stoneforge_sim.pyd` | Kompiliertes Pybind11-Modul (Windows) — auto-generiert, in `.gitignore` |

---

## Wichtigste Klassen

### `StoneforgeWorldEnv`

Basis-Gym-Environment. Verbindet Python mit der C++-Simulation.

```python
from stoneforge_env import StoneforgeWorldEnv

env = StoneforgeWorldEnv(exit_min=35, exit_max=45)
obs, info = env.reset(seed=7000)
obs, reward, terminated, truncated, info = env.step(action)
```

### `ExitPotentialFieldWrapper`

Ergänzt die Observation um 9 BFS-Distanz-Features (Potentialfeld zum Exit).

### `ReducedActionEnv`

Reduziert den Aktionsraum auf 4 Richtungen (Mining ausgeschlossen).

---

## Wrapper-Reihenfolge (Training)

```
StoneforgeWorldEnv → ExitPotentialFieldWrapper → ReducedActionEnv
```

---

## Kompiliertes Binding bauen

Die `.so`/`.pyd`-Dateien werden automatisch in `../build/` erzeugt und
vom Launcher nach `python/` kopiert:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build --target stoneforge_sim -j
```
