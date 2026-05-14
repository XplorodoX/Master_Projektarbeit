# CLAUDE.md — Stoneforge RL Projekt

## Projektübersicht

RL-Agent der in prozedural generierten 2D-Welten (Stoneforge) den Exit finden soll.
Ziel: Generalisierung auf unbekannte Seeds (kein Overfitting auf einzelne Karten).
Algorithmen: DQN (primäre Baseline), PPO (Vergleich).
Framework: Stable-Baselines3, Gym-API, C++ Core via Python-Binding.

---

## Projektstruktur

```
.
├── python/
│   ├── train.py               # Training-Einstiegspunkt (PPO + DQN, CurriculumCallback)
│   ├── stoneforge_env.py      # Gym-Environment + ExitPotentialFieldWrapper + ReducedActionEnv
│   └── ai_play.py             # Visualisierung / Live-Vergleich zweier Agenten
├── src/
│   ├── core/simulation.cpp    # C++ Kernsimulation (Weltgenerierung, Physik, Observation)
│   └── python/py_module.cpp   # Pybind11-Binding für Python
├── assets/base/
│   └── game_config.json       # Observation-Radius, maxSteps, Exit-Distanz, Curriculum-Einstellungen
├── best_models_ppo/           # Gespeicherte PPO-Modelle (best_model.zip)
├── best_models_dqn/           # Gespeicherte DQN-Modelle (best_model.zip)
├── tensorboard_logs/          # TensorBoard-Runs (ppo_run_*, dqn_run_*)
└── RL_Experiment_Changelog.md # ← PFLICHTLEKTÜRE: alle Versionen, Ergebnisse, Änderungen
```

---

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
```

> ⚠️ Nach jeder Änderung an `game_config.json` (z. B. `observationRadius`) ist ein Rebuild nötig.

---

## Training starten

```bash
python python/train.py --algo dqn --timesteps 1000000
python python/train.py --algo ppo --timesteps 1000000
```

---

## Standardisierter Eval (50-Seed-Test)

Immer Seeds `7000–7049`, deterministisch, Mining geblockt:

```bash
python - <<'PY'
import numpy as np
from stable_baselines3 import DQN
from stoneforge_env import StoneforgeWorldEnv

seeds = list(range(7000, 7050))
env = StoneforgeWorldEnv()
model = DQN.load("best_models_dqn/best_model.zip")
succ, lens, rets = 0, [], []
for seed in seeds:
    obs, _ = env.reset(seed=seed)
    done, ep_ret, steps, reached = False, 0.0, 0, False
    while not done and steps < 4000:
        action, _ = model.predict(obs, deterministic=True)
        a = int(action); a = 7 if a == 4 else a
        obs, r, term, trunc, info = env.step(a)
        ep_ret += float(r); steps += 1
        if info.get("reached_exit", False): reached = True
        done = term or trunc
    succ += int(reached); lens.append(steps); rets.append(ep_ret)
print(f"DQN: success={succ}/50 ({succ/50:.1%}), mean_len={np.mean(lens):.1f}, mean_return={np.mean(rets):.2f}")
PY
```

---

## Aktuelle Konfiguration (v1.1)

| Parameter | Wert |
|-----------|------|
| `observationRadius` | 7 (→ 15×15 Grid) |
| `maxSteps` | 4.000 |
| `exitMinDistance` | 35 (Eval-Env) |
| `exitMaxDistance` | 45 (Eval-Env) |
| `forceGuaranteedPath` | `true` |
| `buffer_size` | 500.000 |
| `exploration_fraction` | 0.70 |
| `n_eval_episodes` | 20 |
| Timesteps | 1.000.000 |

Curriculum: leistungsbasiert (4 Stufen: 5–12 / 12–22 / 22–35 / 35–45 Tiles).
Observation: Grid + exitDx/exitDy + ExitPotentialField (9 Features).

---

## Wrapper-Reihenfolge

```
StoneforgeWorldEnv → ExitPotentialFieldWrapper → ReducedActionEnv
```

Aktionsraum (ReducedActionEnv): 0=hoch, 1=runter, 2=links, 3=rechts. Mining ausgeschlossen.

---

## ⚠️ DOKUMENTATIONSPFLICHT — Immer einhalten!

**Nach jeder Änderung und nach jeder Messung:**

1. `RL_Experiment_Changelog.md` aktualisieren
2. Neue Version anlegen (Schema: `v1.x — Datum — Kurzbeschreibung`)
3. Folgendes dokumentieren:
   - **Was wurde geändert?** (Datei, Parameter, vorher → nachher)
   - **Warum?** (Problem das beobachtet wurde, Hypothese)
   - **Ergebnis?** (50-Seed-Eval: success/50, mean_len, mean_return)

**Format für neue Ergebnisse:**

```markdown
| Algorithmus | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|-------------|---------|--------------|----------------------|---------------|-------|
| DQN         | X / 50  | X.X %        | XXXX.X               | −XX.XX        | TT.MM.JJJJ |
```

**Format für neue Änderungen:**

```markdown
#### Änderung X — Kurztitel
**Datei:** `pfad/zur/datei.py`
**Problem:** Was war falsch / was hat nicht funktioniert?
**Lösung:** Was wurde konkret geändert?

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `param`   | alt    | neu     | Warum      |
```

> Diese Dokumentation ist Grundlage der Projektarbeit. Kein Experiment ohne Eintrag im Changelog.

---

## Bekannte Fallstricke

- **Manhattan-Distanz im Reward-Shaping ist irreführend** bei Wänden → Agent bekommt Minusreward für richtige Schritte zur Wandlücke. Aktuell kein Fix implementiert; BFS-Distanz wäre die saubere Lösung (C++ Änderung nötig).
- **Zeitbasiertes Curriculum** lässt den Agent auf neue Stufen wechseln bevor er die aktuelle meistert → Reward-Kollaps. Seit v1.1 leistungsbasiert.
- **`n_eval_episodes=5`** reicht nicht — starkes Rauschen. Seit v1.1 auf 20 gesetzt.
- **Rebuild vergessen** nach `game_config.json`-Änderung → altes Observation-Format → Crash oder falsche Ergebnisse.

---

## Zielkriterium (Projektarbeit)

- **Testset A** (Seeds 7000–7049): ≥ 70 % Success Rate
- **Testset B** (Holdout, noch zu definieren): ≥ 60 % Success Rate
- Pro Konfiguration: mind. 3 Trainingsläufe → Mittelwert + Standardabweichung angeben