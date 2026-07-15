# RL–Spielwelt-Integration: Wie hängt das zusammen?

## Kurze Antwort

**Ja — der RL-Agent spielt in exakt derselben Simulation wie du**, wenn du das Spiel spielst.
Beide nutzen denselben C++-Code (`stoneforge::Simulation`). Der einzige Unterschied:
beim Training sind Mobs und Energie deaktiviert, um das Lernproblem zu vereinfachen.

---

## Architektur-Überblick

```
┌─────────────────────────────────────────────────────────────────┐
│                        C++ Kern (src/)                          │
│                                                                 │
│   stoneforge::Simulation  ←──── stoneforge::World              │
│   (simulation.cpp)               (world.cpp)                   │
│                                                                 │
│   • Spielzustand (HP, Pos, Mobs, ...)                          │
│   • Weltgenerierung (prozedural, seed-basiert)                 │
│   • BFS-Distanzfeld vom Exit                                    │
│   • Reward-Berechnung (computeReward)                          │
│   • Observation (15×15 lokales Grid)                            │
└──────────────────┬──────────────────────────────────────────────┘
                   │  pybind11 (src/python/py_module.cpp)
                   │  → kompiliert zu: build/stoneforge_sim.so
                   ▼
┌─────────────────────────────────────────────────────────────────┐
│              Python-Binding (stoneforge_sim.StoneforgeCoreEnv)  │
│                                                                 │
│   .reset(seed)         → gibt flachen int-Vektor zurück        │
│   .step(action)        → (obs, reward, done, truncated, info)  │
│   .configure_world_generation(exit_min, exit_max, ...)          │
│   .bfs_distance_at_offset(dx, dy)                              │
└──────────────────┬──────────────────────────────────────────────┘
                   │  Python-Wrapper
                   ▼
┌─────────────────────────────────────────────────────────────────┐
│           StoneforgeWorldEnv  (python/stoneforge_env.py)        │
│           implements gymnasium.Env                              │
│                                                                 │
│   • Normalisierung der Observation (÷30, ÷10, ÷100 ...)        │
│   • Reward-Shaping (Stuck-Penalty, Visit-Count-Penalty)        │
│   • Action-Buffer, Step-Fraction                               │
│   • SwarmSeedPool (Curriculum)                                  │
│   • Early-Stopping (256 Schritte ohne pos. Reward)             │
└──────────────────┬──────────────────────────────────────────────┘
                   │  Stable-Baselines3 / sb3-contrib
                   ▼
┌─────────────────────────────────────────────────────────────────┐
│                 RecurrentPPO / PPO / DQN / A2C                  │
│                 (scripts/train.py)                              │
│                                                                 │
│   • Neuronales Netz (MLP + LSTM)                               │
│   • Lernt Policy: Observation → Action                         │
│   • Läuft auf CPU / MPS / CUDA (device-Flag)                   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Was der Agent sieht (Observation)

Der Agent sieht **nicht** die ganze Welt — nur einen 15×15 Ausschnitt
um sich herum (`observationRadius = 7`).

```
Observation-Vektor (231 Werte, float32):

 [0 ... 224]  → 15×15 lokales Grid (225 Werte)
               Encoding: 0=Boden/Luft  10=Wand  15=Exit  20=Mob  30=Agent
               Normalisiert: ÷30 → [0.0, 0.33, 0.5, 0.67, 1.0]

 [225]        → HP (÷10, range 0–1)
 [226]        → Energie (÷100, range 0–1)
 [227]        → Inventory (÷64, range 0–1)
 [228]        → exitDx (÷64) — horizontale Richtung zum Exit
 [229]        → exitDy (÷64) — vertikale Richtung zum Exit
 [230]        → step_frac — wie viele Schritte schon verbraucht (0–1)
```

**Wichtig**: exitDx/exitDy ist eine Gerade-Linie-Richtung, kein Pfad.
Der Agent weiß nicht ob Wände dazwischen liegen — das ist das eigentliche Lernproblem.

---

## Was der Agent tun kann (Actions)

`ReducedActionEnv` begrenzt auf 4 Aktionen (kein Mining):

| Index | Aktion      |
|-------|-------------|
| 0     | Hoch        |
| 1     | Runter      |
| 2     | Links       |
| 3     | Rechts      |

---

## Reward-Funktion (in C++ — simulation.cpp:1459)

```
Schritt-Penalty (immer):       -0.01   (motiviert kurze Wege)
Neue Tile betreten:            +0.02   (Exploration-Bonus)
Idle/Warten:                   -0.04
Schaden durch Mob:             -0.5 × damage
Wand-Anrennen (1×):            -0.05
Wand-Anrennen (≥2× konsek.):   -0.25
2-Schritt-Loop (↑↓↑↓):         -0.15
Mob getötet:                   +2.0 × kills
Exit erreicht:                 +100.0
Episode verloren (kein Exit):  -10.0

Potential-Based Reward Shaping (PBRS):
  Φ(s) = -BFS_Distanz(s) / 128.0
  F(s,s') = γ·Φ(s') − Φ(s)    mit γ=0.999
  Bonus = 2.5 × F(s,s')
  → Pro Tile näher zum Exit:   netto ca. +0.01
  → Pro Tile weiter weg:       netto ca. -0.01

Zusatz in Python (StoneforgeWorldEnv.step()):
  Tile oft besucht (>25×):     -0.03 × (count/25)  (Stuck-Penalty)
```

---

## Unterschied Training vs. spielbares Spiel

| Feature           | Training (RL)          | Spielbares Spiel      |
|-------------------|------------------------|-----------------------|
| Mobs              | deaktiviert            | aktiv                 |
| Energie           | deaktiviert            | aktiv                 |
| Worldgen (Seed)   | identisch              | identisch             |
| Observation       | 15×15 Grid + Kompass   | vollständige Karte    |
| Aktionen          | 4 (nur Bewegung)       | alle (Mining, Bauen …)|
| Exit-Distanz      | konfigurierbar (5–45)  | 35–45 (Standard)      |

---

## Weltgenerierung — deterministisch und reproduzierbar

Dieselbe Seed → dieselbe Welt, immer.

```python
env.reset(seed=42)   # → immer exakt dieselbe Welt
```

Das ermöglicht den standardisierten Eval-Test (Seeds 7000–7049):
Der Agent wurde auf diesen Seeds **nie trainiert** — sie sind reines Testset.

---

## Datenfluss pro Trainingsschritt

```
1. SB3 fragt Policy: "Welche Aktion für diese Observation?"
         ↓
2. StoneforgeWorldEnv.step(action)
         ↓
3. stoneforge_sim.StoneforgeCoreEnv.step(action)  [C++ via pybind11]
         ↓
4. C++: tryMove() / mineForward() / etc.
   C++: updateMobs()
   C++: computeReward(BFS_vor, BFS_nach, ...)
   C++: getObservation() → 225 Grid-Werte + 5 Extras
         ↓
5. Python: Normalisierung, Stuck-Penalty, Truncation, Swarm-Pool
         ↓
6. SB3: speichert (obs, action, reward, done) im Rollout-Buffer
   SB3: nach n_steps×n_envs Schritten → Netz-Update (PPO/LSTM)
```

---

## Warum pybind11?

Die Simulation ist in C++ geschrieben weil:
- Weltgenerierung (Perlin-Noise, BFS, prozedurale Karten) ist CPU-intensiv
- 8–16 parallele Environments müssen tausende Steps/Sekunde leisten
- Python allein wäre ~10–50× langsamer

pybind11 kompiliert die Simulation zu einer `.so`-Datei (`stoneforge_sim.so`)
die Python wie ein normales Modul importiert — ohne Overhead durch Sockets oder Prozesse.

---

## Relevante Dateien

| Datei | Rolle |
|-------|-------|
| [src/core/simulation.cpp](../src/core/simulation.cpp) | Simulationslogik, Reward, BFS, Observation |
| [src/python/py_module.cpp](../src/python/py_module.cpp) | pybind11-Binding: C++ → Python-Objekt |
| [python/stoneforge_env.py](../python/stoneforge_env.py) | Gym-Wrapper: Normalisierung, Penalties, Curriculum |
| [scripts/train.py](../scripts/train.py) | SB3-Training: RecurrentPPO/PPO/DQN/A2C |
| [assets/base/game_config.json](../assets/base/game_config.json) | Konfiguration: observationRadius, maxSteps, … |
