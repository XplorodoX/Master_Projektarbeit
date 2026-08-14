# Stoneforge RL

Ein RL-Agent navigiert in einer eigenentwickelten, prozedural generierten
2D-Welt (Stoneforge) zu einem Ausgang, den er zu Beginn nicht sieht.

Eine gemeinsame Projektarbeit von **Laurin Rößler** (Spiel und
Weltgenerierung, C++) und **Florian Merlau** (Reinforcement Learning,
Python), Hochschule Aalen. Die vollständige schriftliche Ausarbeitung liegt
unter [`docs/Doku/Projektarbeit Stoneforge RL.pdf`](docs/Doku/Projektarbeit%20Stoneforge%20RL.pdf).

| | |
|---|---|
| Simulationskern | C++20, angebunden über pybind11 |
| Schnittstelle | Gym-/Gymnasium-API |
| Algorithmen | PPO · RecurrentPPO (LSTM) · DQN · A2C, über Stable-Baselines3 / sb3-contrib |
| Hauptergebnis | RecurrentPPO, Curriculum-Training, n = 7 unabhängige Läufe (`models/ppo_lstm_curriculum_v12_s1..s7`) |

---

## Schnellstart

```bash
# 1. Bauen (C++-Simulationskern + Python-Binding + Client)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"

# 2. Python-Umgebung einrichten
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt

# 3. Loslegen
python scripts/launcher_gui.py     # GUI: Training, Play, Eval, Build
./build/stoneforge_client          # oder direkt das Spiel starten
```

**Vollständige Anleitung** (Build-Details, Trainings-/Eval-Kommandos, wie
sich der berichtete v12-Stand reproduzieren lässt, Konfigurationsreferenz,
bekannte Fallstricke) → [`admin/README.md`](admin/README.md).

---

## Berichtete Ergebnisse reproduzieren

Alle in der Arbeit zitierten Zahlen stammen ausschließlich aus dem
kanonischen Eval-Protokoll:

```bash
python scripts/eval_baselines.py --models
```

Das lädt die trainierten v12-Modelle aus `models/` und wertet sie zusammen
mit den ungelernten Referenzverfahren (Random, ε-Kompass-Heuristik,
BFS-Optimum) auf Testset A (Seeds 7000–7049) und Holdout B (Seeds 8000–8049)
aus. Ergebnis: `logs/eval_results/baselines.json`, dieselbe Datei liegt
bereits im Repository. Details zum Protokoll in
[`admin/README.md`](admin/README.md#kanonisches-eval-protokoll).

---

## Ordnerstruktur

```
.
├── CMakeLists.txt          ← Build-System (Wurzel)
├── CHANGELOG.md            ← vollständige Experiment-Historie (Pflichtlektüre für Details)
├── requirements.txt        ← Python-Abhängigkeiten
│
├── src/                    ← C++-Quellcode
│   ├── core/               ← Kernsimulation (Weltgenerierung, PBRS, BFS)
│   ├── client/              ← raylib-Client (grafisch spielbar)
│   ├── apps/                ← Headless-Runner (Benchmarks, keine Grafik)
│   ├── python/               ← pybind11-Binding → stoneforge_sim.so
│   └── include/stoneforge/  ← öffentliche C++-API
│
├── python/                 ← importierbare RL-Bibliothek (PYTHONPATH)
│   └── stoneforge_env.py   ← Gym-Environment + Wrapper
│
├── scripts/                ← ausführbare Skripte
│   ├── launcher_gui.py     ← Haupt-Einstiegspunkt (GUI)
│   ├── train_curriculum.py ← erzeugt die berichtsfähigen v12-Ergebnisse (PPO / RecurrentPPO)
│   ├── train.py            ← Einzelphasen-Training (PPO / DQN / A2C)
│   ├── eval_baselines.py   ← kanonisches Eval-Protokoll (siehe oben)
│   ├── watch_agent.py      ← Agent grafisch beobachten
│   ├── analyze_agent.py    ← Verhaltensanalyse
│   └── probe_world_geometry.py ← Umwegfaktor, Wanddichte, Lösbarkeit
│
├── models/                 ← trainierte Modelle (.zip)
│   └── ppo_lstm_curriculum_v12_s1..s7  ← aktuelles Hauptergebnis (n = 7)
│
├── assets/base/            ← Spielkonfiguration (game_config.json)
│
├── logs/
│   ├── eval_results/       ← baselines.json — Grundlage aller berichteten Zahlen
│   └── tensorboard/        ← Trainingsverläufe (`tensorboard --logdir logs/tensorboard/`)
│
├── docs/
│   ├── Doku/                ← schriftliche Ausarbeitung (.tex/.pdf) + Abbildungen
│   └── papers/               ← Referenz-Forschungsliteratur
│
├── admin/                  ← vollständige Anleitung für Betreuende/Prüfende
├── bin/                    ← portable Kopie der Binaries (nach Build)
├── screenshots/ videos/    ← Bild-/Videomaterial der Anwendung
├── toolgen/                ← Wartungsskripte
│
├── OLD/                    ← archivierter, nicht mehr aktiver Code
└── build/                  ← CMake-Build-Output (nicht getrackt)
```

---

## TensorBoard

```bash
tensorboard --logdir logs/tensorboard/
```

---

## Zielkriterium und Stand

| Testset | Ziel | Erreicht (v12, n = 7, stochastisch) |
|---------|------|---------------------------------------|
| A — Seeds 7000–7049 | ≥ 70 % Success Rate | 64,6 % ± 10,1 |
| B — Holdout (Seeds 8000–8049) | ≥ 60 % Success Rate | 68,8 % ± 15,5 |

Einordnung, Statistik und Diskussion (unter anderem der Vergleich gegen die
ε-Kompass-Heuristik) in Kapitel 5 der schriftlichen Ausarbeitung.

---

## Plattform

Windows · macOS · Linux — C++20, CMake ≥ 3.20, raylib ≥ 5.0, Python ≥ 3.10.
Getestet auf Apple M1 Pro (macOS).
