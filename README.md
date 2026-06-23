# Stoneforge RL — Masterarbeit Projektarbeit

RL-Agent der in prozedural generierten 2D-Welten (Stoneforge) den Exit finden soll.  
Algorithmen: PPO · DQN · A2C · Framework: Stable-Baselines3 · Simulation: C++ / Pybind11

---

## Schnellstart

```bash
# 1. Bauen
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j

# 2. Python-Umgebung einrichten
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"

# 3. Launcher starten
python scripts/launcher_gui.py
```

Vollständige Admin-Anleitung → [admin/README.md](admin/README.md)

---

## Ordnerstruktur

```
.
├── CMakeLists.txt          ← Build-System (Wurzel)
├── CHANGELOG.md            ← Experiment-Changelog (PFLICHTLEKTÜRE)
├── requirements.txt        ← Python-Abhängigkeiten
│
├── src/                    ← C++-Quellcode + Build-Infrastruktur
│   ├── core/               ← Kernsimulation (Weltgenerierung, Physik, BFS)
│   ├── client/             ← Raylib-Client (grafisch spielbar)
│   ├── apps/               ← Headless-Runner (schnell, kein Fenster)
│   ├── python/             ← Pybind11-Binding → stoneforge_sim.so
│   ├── include/stoneforge/ ← C++-Header (öffentliche API)
│   └── cmake/              ← CMake-Hilfsmodule
│
├── python/                 ← Importierbare RL-Bibliothek (PYTHONPATH)
│   └── stoneforge_env.py   ← Gym-Environment + Wrapper
│
├── scripts/                ← Alle ausführbaren Skripte
│   ├── launcher_gui.py     ← HAUPT-EINSTIEGSPUNKT (GUI)
│   ├── train.py            ← Training (PPO / DQN / A2C + Curriculum)
│   ├── watch_agent.py      ← KI-Agent grafisch beobachten
│   ├── analyze_agent.py    ← Verhaltensanalyse
│   ├── eval_hard_world.py  ← Eval auf schwierigen Welten
│   ├── eval_temperature.py ← Temperatur-Sweep Benchmark
│   └── setup_env.sh        ← Umgebung aktivieren
│
├── models/                 ← Trainierte Modelle (.zip)
│   ├── ppo_baseline/       ← Erste Baseline
│   ├── ppo_phase3/         ← Phase-3-Modell
│   ├── ppo_phase5/         ← Aktuell bestes Modell
│   └── ...
│
├── assets/base/            ← Spielkonfiguration (game_config.json)
│
├── logs/                   ← Alle Logs
│   ├── tensorboard/        ← TensorBoard-Trainingsverläufe
│   └── runtime/            ← Laufzeit-Logs (game.log etc.)
│
├── docs/                   ← Projektdokumentation
│   ├── Expose.pdf          ← Projektexposé
│   ├── Projektarbeit_RL_Dokumentation.md
│   ├── report.html
│   └── papers/             ← Referenz-Forschungsliteratur
│
├── admin/                  ← Anleitung für Betreuer/Prüfer
├── bin/                    ← Kompilierte Binaries (nach Build)
├── screenshots/            ← Screenshots der Anwendung
├── videos/                 ← Demo-Videos
├── wireframes/             ← UI-Mockups
├── toolgen/                ← Wartungsskripte (fix_changelog.py)
│
├── OLD/                    ← Archiv veralteter Dateien (nicht löschen!)
│
└── build/                  ← CMake Build-Output (auto-generiert, in .gitignore)
```

---

## Wichtige Dateien

| Datei / Ordner | Beschreibung |
|----------------|-------------|
| `scripts/launcher_gui.py` | GUI-Launcher (Training, Play, Eval, Build, Dashboard) |
| `scripts/train.py` | Training mit PPO / DQN / A2C |
| `python/stoneforge_env.py` | Gym-Environment + Wrapper |
| `scripts/watch_agent.py` | KI-Agent grafisch beobachten (auto-erkennt PPO/A2C/DQN) |
| `assets/base/game_config.json` | Konfiguration (observationRadius, maxSteps, …) |
| `CHANGELOG.md` | Alle Experimente, Ergebnisse, Änderungen |
| `src/core/simulation.cpp` | C++ Kernsimulation |
| `src/python/py_module.cpp` | Pybind11-Binding |

---

## TensorBoard

```bash
tensorboard --logdir logs/tensorboard/
```

---

## Zielkriterium (Projektarbeit)

| Testset | Ziel |
|---------|------|
| A — Seeds 7000–7049 | ≥ 70 % Success Rate |
| B — Holdout (Seeds 8000–8049) | ≥ 60 % Success Rate |

---

## Plattform

Windows · macOS · Linux — C++20, CMake ≥ 3.20, raylib ≥ 5.0, Python ≥ 3.10
