---
id: cpp-core
title: C++-Core und Pybind11-Binding
type: component
tags: [cpp, build, architektur]
path: src/core/simulation.cpp, src/python/py_module.cpp
related: [stoneforge-env, pbrs-reward-shaping, rebuild-pflicht, config-prozessglobal]
updated: 2026-07-17
---

# C++-Core und Binding

Die Simulation läuft in C++ (Performance), Python spricht über pybind11 mit ihr.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
```

## Zwei getrennte Binaries — der wichtigste Architekturfakt

| Binary | Besteht aus |
|--------|-------------|
| `stoneforge_sim.so` | `py_module.cpp` + `stoneforge_core` (game_config, item, object, recipe, world, simulation) |
| `stoneforge_client` | zusätzlich `render_engine.cpp`, `render_ui.cpp` |

**Folge: Änderungen am Spiel-Client berühren das RL-Env nicht.** Nachgewiesen über den Build-Graph
am 15.07.2026 — die Spiel-Commits (Tiles, Debug-Menü, Inventartext, "structures") haben
ausschließlich Client-Dateien angefasst. `44936aa "structures"` ist trotz des Namens reines
Rendering, keine Weltgenerierung. Wer bei Client-Commits Angst um die Reproduzierbarkeit der
RL-Ergebnisse hat, kann sie hier ablegen.

## Was im Core steckt

- **Weltgenerierung** (prozedural, seed-basiert) → [[weltgenerierung]]
- **BFS** — Grundlage für [[pbrs-reward-shaping]] und die [[exit-platzierung]]
- **Biom-Schwellwerte**: hartkodiert, **nicht** über `game_config.json` tunebar → [[biome]]
- **Das Spiel selbst** (Tiles, Mining, Inventar) → [[stoneforge-spiel]], [[tile-typen]]

## Fallstricke

- Nach jeder Änderung an C++ **oder** an `game_config.json` ist ein Rebuild nötig →
  [[rebuild-pflicht]].
- Die WorldGen-Config ist prozessglobal → [[config-prozessglobal]].
