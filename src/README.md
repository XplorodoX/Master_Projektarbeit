# src/ — Quellcode (C++ Simulation & Client)

Der C++-Quellcode des Projekts. Parallel dazu liegt der Python-RL-Code in `../python/`.

---

## Verzeichnisstruktur

| Ordner | Beschreibung |
|--------|-------------|
| `core/` | Kernsimulation (Weltgenerierung, Physik, Observation, BFS) |
| `client/` | Raylib-Client (grafische Darstellung, Rendering, Input) |
| `apps/` | Headless-Runner (schnelle Simulation ohne GUI, für RL) |
| `python/` | Pybind11-Binding — verbindet C++-Sim mit Python/Gym |
| `cmake/` | CMake-Hilfsmodule (CompilerWarnings) |
| `include/stoneforge/` | Öffentliche C++-Header |

---

## Wichtige Dateien

| Datei | Beschreibung |
|-------|-------------|
| `core/simulation.cpp` | Haupt-Simulation (Weltschritt, Reward, Observation) |
| `core/world.cpp` | Prozedurale Weltgenerierung (Seeds, Biome, Exit) |
| `python/py_module.cpp` | Pybind11-Modul → `stoneforge_sim.so` |
| `client/raylib_main.cpp` | Spielbarer Client (raylib) |
| `apps/headless_main.cpp` | Schneller Headless-Simulator |

---

## Header (`include/stoneforge/`)

| Datei | Beschreibung |
|-------|-------------|
| `simulation.hpp` | Haupt-Simulations-Interface |
| `world.hpp` | Weltgenerierung |
| `game_config.hpp` | Konfigurationsstrukturen |
| `types.hpp` | Gemeinsame Datentypen |
| `client/render_engine.hpp` | Render-Engine-Interface |

---

## Build

```bash
# Aus dem Projektwurzel:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j

# Nur Python-Binding:
cmake --build build --target stoneforge_sim -j

# Nur Spiel-Client:
cmake --build build --target stoneforge_client -j
```

Build-Output landet in `../build/`.
Vollständige Anleitung → `../admin/README.md`

---

## Python-RL-Code

Der Python-Code (Training, Environment, Eval) liegt in `../python/` — nicht hier.
Quellcode-Übersicht: dieses README.
