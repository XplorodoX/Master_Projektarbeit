# Admin-Anleitung — Stoneforge RL

## Voraussetzungen

| Tool | Version | Hinweis |
|------|---------|---------|
| CMake | ≥ 3.20 | `brew install cmake` |
| Python | ≥ 3.10 | mit `pip` |
| raylib | ≥ 5.0 | `brew install raylib` |
| C++ Compiler | C++20 | Xcode CLT auf macOS |

---

## 1 · Projekt bauen

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
```

> Nach jeder Änderung an `assets/base/game_config.json` muss neu gebaut werden.

---

## 2 · Python-Umgebung einrichten

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

---

## 3 · Launcher starten (empfohlen)

```bash
python scripts/launcher_gui.py
```

Der Launcher bietet eine grafische Oberfläche für Training, Evaluation und das Abspielen von Modellen.

---

## 4 · Training manuell starten

```bash
python scripts/train.py --algo ppo --timesteps 1000000
python scripts/train.py --algo dqn --timesteps 1000000
python scripts/train.py --algo a2c --timesteps 1000000
```

Trainierte Modelle werden automatisch in `models/{algo}/` gespeichert.

---

## 5 · Modell beobachten (grafisch)

```bash
python scripts/watch_agent.py --model models/ppo_phase5/best_model.zip --seed 7000
```

---

## 6 · 50-Seed-Evaluation

```bash
python - <<'PY'
import numpy as np
from stable_baselines3 import PPO
from stoneforge_env import StoneforgeWorldEnv

seeds = list(range(7000, 7050))
env = StoneforgeWorldEnv()
model = PPO.load("models/ppo_phase5/best_model.zip")
succ, lens, rets = 0, [], []
for seed in seeds:
    obs, _ = env.reset(seed=seed)
    done, ep_ret, steps, reached = False, 0.0, 0, False
    while not done and steps < 4000:
        action, _ = model.predict(obs, deterministic=True)
        obs, r, term, trunc, info = env.step(int(action))
        ep_ret += float(r); steps += 1
        if info.get("reached_exit", False): reached = True
        done = term or trunc
    succ += int(reached); lens.append(steps); rets.append(ep_ret)
print(f"PPO: success={succ}/50 ({succ/50:.1%}), mean_len={np.mean(lens):.1f}, mean_return={np.mean(rets):.2f}")
PY
```

---

## 7 · Kompilierte Binaries

Nach dem Build liegen die Binaries in `build/`:

| Binary | Beschreibung |
|--------|-------------|
| `build/stoneforge_client` | Spielbarer Client (raylib) |
| `build/stoneforge_headless` | Headless-Simulator |
| `build/stoneforge_sim*.so` | Python-Binding |

---

## 8 · Ordnerstruktur (Kurzübersicht)

```
src/            ← C++-Quellcode (Core, Client, Binding)
python/         ← Importierbare RL-Bibliothek (PYTHONPATH)
scripts/        ← Ausführbare Skripte (Training, Eval, Watch, Launcher)

models/         ← Trainierte Modelle (.zip)
logs/           ← TensorBoard + Laufzeit-Logs
assets/         ← Spielkonfiguration (game_config.json)
build/          ← CMake Build-Output (auto-generiert)

docs/           ← Projektdokumentation, Exposé, Papers
admin/          ← Diese Datei (Anleitung für Betreuer/Prüfer)
bin/            ← Kompilierte Binaries (nach Build)
screenshots/    ← Screenshots der Applikation
videos/         ← Demo-Videos
wireframes/     ← UI-Mockups
toolgen/        ← Wartungsskripte (fix_changelog.py)
```
