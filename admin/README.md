# Stoneforge RL — Admin-Anleitung

Diese Anleitung richtet sich an Betreuende und Prüfende, die das Projekt selbst
bauen, den Client spielen oder die berichteten RL-Ergebnisse nachvollziehen
möchten. Sie ergänzt die [Haupt-README](../README.md) um die vollständigen
Details.

Die schriftliche Ausarbeitung liegt unter
[`docs/Doku/Projektarbeit Stoneforge RL.pdf`](../docs/Doku/Projektarbeit%20Stoneforge%20RL.pdf),
die vollständige Experiment-Historie in [`CHANGELOG.md`](../CHANGELOG.md).

---

## Inhalt

1. [Build](#build)
2. [Einstiegspunkte](#einstiegspunkte)
3. [Berichtsfähiger Stand nachvollziehen](#berichtsfähiger-stand-nachvollziehen)
4. [Kanonisches Eval-Protokoll](#kanonisches-eval-protokoll)
5. [Konfiguration](#konfiguration)
6. [Algorithmen](#algorithmen)
7. [Environment](#environment)
8. [Projektstruktur](#projektstruktur)
9. [Bekannte Fallstricke](#bekannte-fallstricke)
10. [Offene Punkte](#offene-punkte)

---

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"

python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
```

Voraussetzungen: C++20-Compiler, CMake ≥ 3.20, Python ≥ 3.10. raylib und
nlohmann/json werden über CMake eingebunden, siehe `CMakeLists.txt`.

**Rebuild nötig:** nach jeder Änderung am C++-Code (`src/`). Wird er
vergessen, folgen Crash oder falsche Ergebnisse.

**Rebuild nicht nötig:** nach Änderungen an `assets/base/game_config.json`.
Die Datei wird zur Laufzeit gelesen. Ein Patch wirkt sofort für jedes danach
**neu konstruierte** Environment; bereits bestehende Instanzen behalten ihre
Werte.

Nach dem Build liegen die Binaries in `build/` (`stoneforge_client`,
`stoneforge_headless`, `stoneforge_sim*.so`/`.pyd`). Für eine portable Kopie
siehe [`bin/README.md`](../bin/README.md).

---

## Einstiegspunkte

### Launcher (empfohlen für einen ersten Überblick)

```bash
python scripts/launcher_gui.py
```

Bietet Training, Evaluation, Abspielen und Build in einer Oberfläche.

### Spiel spielen

```bash
./build/stoneforge_client
```

### Training

```bash
# Berichtsfähige Läufe (Curriculum, liefert den v12-Stand aus der Arbeit)
python scripts/train_curriculum.py --algo rppo    # RecurrentPPO (LSTM) — Hauptergebnis
python scripts/train_curriculum.py --algo ppo     # MLP-Kontrollgruppe

# Einzelphasen-Training (nicht Quelle der berichteten Zahlen)
python scripts/train.py --algo ppo --timesteps 1000000
python scripts/train.py --algo dqn --timesteps 1000000
python scripts/train.py --algo a2c --timesteps 1000000
```

`train_curriculum.py` speichert die Modelle nach `models/`, TensorBoard-Logs
nach `logs/tensorboard/`. Ein LSTM-Lauf über das volle Curriculum dauert auf
Grundlage der in der Arbeit gemessenen Durchsatzwerte (rund 104 Steps/s)
mehrere Stunden; die MLP-Kontrollgruppe (rund 5.989 Steps/s) ist deutlich
schneller.

### Evaluation

```bash
python scripts/eval_baselines.py --models     # Baselines + trainierte Modelle
python scripts/eval_baselines.py              # nur ungelernte Referenzen
```

Siehe [Kanonisches Eval-Protokoll](#kanonisches-eval-protokoll) für Details
und Begründung, warum kein eigenes Eval-Snippet verwendet werden sollte.

### Weitere Werkzeuge

| Skript | Zweck |
|---|---|
| `scripts/watch_agent.py` | Grafische Agent-Visualisierung |
| `scripts/analyze_agent.py` | Verhaltensanalyse (Besuchsmuster, Trajektorien) |
| `scripts/probe_world_geometry.py` | Umwegfaktor, Wanddichte, Lösbarkeit der Weltgenerierung |
| `scripts/smoke_test_algo_switch.py` | Schnelltest des `--algo`-Pfads |
| `scripts/plot_*.py` | Erzeugen die Abbildungen der schriftlichen Ausarbeitung aus den Rohdaten |

---

## Berichtsfähiger Stand nachvollziehen

Die für die Arbeit zitierten Zahlen (Kapitel 5) stammen ausschließlich aus
`logs/eval_results/baselines.json`. Diese Datei ist Teil des Repositories und
lässt sich mit den trainierten Modellen unter `models/ppo_lstm_curriculum_v12_s1..s7`
und `models/ppo_mlp_curriculum_v12_s1..s7` reproduzieren (Seed 8 der
MLP-Reihe ist nicht Teil der berichteten sieben Läufe, siehe CHANGELOG):

```bash
python scripts/eval_baselines.py --models
```

Die dabei erzeugte `logs/eval_results/baselines.json` sollte mit der im
Repository liegenden Version übereinstimmen (Env-Seeds sind deterministisch,
siehe [Bekannte Fallstricke](#bekannte-fallstricke)). Kleinere Abweichungen
in den letzten Nachkommastellen sind durch den fünffachen Wiederholungs-Mittelwert
über unabhängige Politik-RNG-Seeds möglich.

Die Abbildungen in `docs/Doku/Bilder/eval_*.png` und `v12_zielkriterium.png`
werden aus derselben Datei erzeugt:

```bash
python scripts/plot_eval_results.py
python scripts/plot_v12_endergebnis.py
```

---

## Kanonisches Eval-Protokoll

**Es gibt genau ein gültiges Protokoll. Nicht selbst nachbauen.**

| Parameter | Wert |
|-----------|------|
| Testset A | Seeds 7000–7049 |
| Holdout B | Seeds 8000–8049 |
| Exit-Distanz | 35–45 |
| Episoden-Cap | **4000** (= Env-`maxSteps`) |
| Wiederholungen | 5 über unabhängige Politik-RNG-Seeds |
| Ausgabe | `logs/eval_results/baselines.json` |

Ein früheres Inline-Eval-Snippet war defekt: Es deckte `RecurrentPPO` nicht
ab und rief `model.predict()` ohne LSTM-Zustand auf, was ein Gedächtnismodell
ohne Gedächtnis misst und systematisch zu niedrige Werte liefert.
`eval_baselines.py` führt den LSTM-Zustand in `ModelPolicy` korrekt mit.

Ein Eval-Cap unter 4000 verfälscht die Success Rate massiv (gleiches Modell:
48 % bei Cap 600 gegenüber 86 % bei Cap 4000). Immer mit Cap 4000 fahren.

---

## Konfiguration

### Environment

| Parameter | Wert |
|-----------|------|
| `observationRadius` | 7 (→ 15×15 Grid) |
| `maxSteps` | 4.000 |
| `exitMinDistance` / `exitMaxDistance` (Eval) | 35 / 45 |
| `forceGuaranteedPath` | `false` — die BFS-Exit-Platzierung garantiert Lösbarkeit unabhängig davon |

Observation (Env v11): 229 Features = Grid 15×15 (225) + HP + exitDx +
exitDy + step_frac. Legacy-Modelle von vor der v11-Umstellung haben 231
Dimensionen und benötigen `StoneforgeWorldEnv(..., include_energy_inventory=True)`.

### Algorithmenspezifisch

| Algo | Parameter | Wert | Begründung |
|------|-----------|------|------------|
| RecurrentPPO | `batch_size` | **8** (nicht 64) | 64 destabilisiert den Critic — siehe Kapitel 4.2 der Arbeit |
| RecurrentPPO | `ent_coef` | 0.05 | |
| RecurrentPPO | Device | **CPU** | MPS ist bei diesem Netz langsamer |
| DQN | `buffer_size` | 200.000 | |
| DQN | `exploration_fraction` | 0.50 | |

---

## Algorithmen

| Algo | Aktionsraum | Status | Ausgabeordner |
|------|-------------|--------|---------------|
| PPO | Discrete ✓ | aktiv | `models/ppo` |
| RecurrentPPO | Discrete ✓ | aktiv, liefert v12-Hauptergebnis | `models/ppo_lstm_curriculum_v12_s*` |
| DQN | Discrete ✓ | aktiv, für Hauptvergleich verworfen (Begründung: Kapitel 3.2 der Arbeit) | `models/dqn` |
| A2C | Discrete ✓ | aktiv, nicht praktisch getestet | `models/a2c` |
| SAC / TD3 | nur Box ✗ | nicht nutzbar (Aktionsraum ist `Discrete(4)`) | — |

---

## Environment

`StoneforgeWorldEnv` wird direkt verwendet, kein Wrapper-Stack.

Aktionsraum `Discrete(4)` — 0 = hoch, 1 = runter, 2 = links, 3 = rechts.

Mining, Bauen und Kampf sind seit Env v11 im C++-Binding entfernt; die
entsprechenden Aktionen existieren nur noch im spielbaren Client.

---

## Projektstruktur

```
.
├── admin/                        # diese Anleitung
├── assets/base/                  # game_config.json (Weltgenerierung, Env-Parameter)
├── bin/                          # portable Kopie der Binaries (nach Build)
├── build/                        # CMake-Build-Output (nicht getrackt)
├── docs/
│   ├── Doku/                     # schriftliche Ausarbeitung (.tex/.pdf) + Abbildungen
│   └── papers/                   # Referenz-Forschungsliteratur
├── logs/
│   ├── eval_results/             # baselines.json — Grundlage aller berichteten Zahlen
│   └── tensorboard/               # Trainingsverläufe
├── models/                       # trainierte Modelle (.zip), inkl. v12-Endergebnis
├── OLD/                          # archivierter, nicht mehr aktiver Code
├── python/
│   └── stoneforge_env.py         # Gym-Environment + Wrapper
├── scripts/                      # ausführbare Skripte (Training, Eval, Plots)
├── src/
│   ├── core/                     # C++-Simulationskern (Weltgenerierung, PBRS)
│   ├── client/                   # raylib-Client
│   ├── apps/                     # Headless-Runner
│   ├── python/                   # pybind11-Binding
│   └── include/stoneforge/       # öffentliche C++-API
├── toolgen/                      # Wartungsskripte
├── CHANGELOG.md                  # vollständige Experiment-Historie
├── CMakeLists.txt
└── requirements.txt
```

---

## Bekannte Fallstricke

- **Einzelne Eval-Snapshots sind keine Validierung.** Die Lerndynamik kann
  chaotisch sein und transiente Hochphasen zeigen. Hyperparameter-Entscheidungen
  fallen ausschließlich auf Basis ganzer Eval-Kurven (`logs/tensorboard/`).
- **`batch_size=64` destabilisiert den LSTM-Critic** — siehe Konfigurationstabelle
  oben und Kapitel 4.2 der Arbeit.
- **MPS/GPU bringt bei diesem Netz nichts**, CPU ist durchgehend schneller.
- **Zelluläre Glättung härtet die Welt nicht** — sie senkt Wanddichte und
  Umwegfaktor gleichzeitig; siehe Kapitel 2.1.3 und Anhang A der Arbeit.
- **WorldGen-Config ist prozess-global** (C++). `StoneforgeWorldEnv` stempelt
  sie bei jedem `reset()` neu. Bei direkter Nutzung des C++-Bindings ohne
  Env-Wrapper daran denken.

---

## Offene Punkte

- `scripts/eval_hard_world.py` setzt tote Config-Keys und macht die Welt
  dadurch leerer statt härter — vor Weiterverwendung reparieren.
- Die quantitative Härteanalyse der Weltgenerierung (Umwegfaktor, Wanddichte
  über Regelvarianten, siehe `scripts/probe_world_geometry.py --sweep`) ist
  gemessen (`logs/eval_results/world_geometry.json`), aber noch nicht als
  eigener Abschnitt in die schriftliche Ausarbeitung eingeflossen.
