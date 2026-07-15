# CLAUDE.md — Stoneforge RL Projekt

## Projektübersicht

RL-Agent der in prozedural generierten 2D-Welten (Stoneforge) den Exit finden soll.
Ziel: Generalisierung auf unbekannte Seeds (kein Overfitting auf einzelne Karten).
Algorithmen: PPO · DQN · A2C (alle via Stable-Baselines3).
Framework: Stable-Baselines3, Gym-API, C++ Core via Python-Binding (pybind11).

---

## Projektstruktur

```
.
├── admin/                     # Admin-Anleitung (Build, Start, Eval)
├── bin/                       # Kompilierte Binaries (nach Build)
├── docs/                      # Projektdokumentation, Exposé, Paper, Report
│   └── papers/                # Referenz-Forschungsliteratur
├── models/                    # Trainierte Modelle (.zip) — ALLE hier!
│   ├── ppo_baseline/          # Erste PPO-Baseline
│   ├── ppo_bfscompass/        # PPO mit BFS-Compass-Feature
│   ├── ppo_phase3/            # Phase-3-Modell (~56% SR)
│   ├── ppo_phase4/            # Phase-4-Modell (aktuell bestes — 100% SR, Ø 49 Schritte)
│   ├── ppo_phase5/            # Phase-5-Modell (neu trainiert 07.06.2026, 72% SR)
│   └── ...                    # Weitere Experimente
├── OLD/                       # Veralteter Code (nicht gelöscht, nur archiviert)
│   └── scripts/               # Alter CLI-Launcher, Shell-Skripte
├── python/                    # Importierbare RL-Bibliothek (PYTHONPATH)
│   └── stoneforge_env.py      # Gym-Environment + Wrapper (importiert via PYTHONPATH)
├── scripts/                   # Ausführbare Skripte
│   ├── launcher_gui.py        # ← HAUPT-EINSTIEGSPUNKT (GUI)
│   ├── train.py               # Training (PPO / DQN / A2C, CurriculumCallback)
│   ├── watch_agent.py         # Grafische Agent-Visualisierung
│   ├── analyze_agent.py       # Verhaltensanalyse
│   ├── eval_hard_world.py     # Evaluation auf schwierigen Welten
│   ├── eval_temperature.py    # Temperatur-Sweep Benchmark
│   └── setup_env.sh           # Umgebung aktivieren + PYTHONPATH setzen
├── screenshots/               # Screenshots
├── src/                       # C++ Quellcode
│   ├── core/simulation.cpp    # Kernsimulation
│   └── python/py_module.cpp   # Pybind11-Binding
├── toolgen/                   # Wartungsskripte (fix_changelog.py)
├── videos/                    # Demo-Videos
├── wireframes/                # UI-Mockups
├── assets/base/               # Spielkonfiguration (game_config.json)
├── logs/                      # Laufzeit-Logs und TensorBoard
│   └── tensorboard/           # TensorBoard-Runs (ppo_*, dqn_*)
├── requirements.txt           # Python-Abhängigkeiten
├── CHANGELOG.md               # ← PFLICHTLEKTÜRE: alle Versionen, Ergebnisse
└── CMakeLists.txt             # Build-System
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

## Launcher starten (empfohlen)

```bash
python scripts/launcher_gui.py
```

Der Launcher bietet Training, Evaluation, Abspielen und Build in einer GUI.

---

## Training starten (CLI)

```bash
python scripts/train.py --algo ppo --timesteps 1000000
python scripts/train.py --algo dqn --timesteps 1000000
python scripts/train.py --algo a2c --timesteps 1000000
```

Modelle werden gespeichert in `models/{algo}/best_model.zip`.
TensorBoard-Logs landen in `logs/tensorboard/`.

---

## Standardisierter Eval (50-Seed-Test)

Immer Seeds `7000–7049`, deterministisch, Mining geblockt:

```bash
python - <<'PY'
import numpy as np
from stable_baselines3 import PPO, A2C, DQN
from stoneforge_env import StoneforgeWorldEnv

seeds = list(range(7000, 7050))
env = StoneforgeWorldEnv(exit_min=35, exit_max=45)

# Modellpfad anpassen:
path = "models/ppo_phase4/best_model.zip"
model = None
for Cls, name in [(PPO, "PPO"), (A2C, "A2C"), (DQN, "DQN")]:
    try:
        model = Cls.load(path); algo = name; break
    except Exception:
        pass

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
print(f"{algo}: success={succ}/50 ({succ/50:.1%}), mean_len={np.mean(lens):.1f}, mean_return={np.mean(rets):.2f}")
PY
```

---

## Aktuelle Konfiguration

| Parameter | Wert |
|-----------|------|
| `observationRadius` | 7 (→ 15×15 Grid) |
| `maxSteps` | 4.000 |
| `exitMinDistance` | 35 (Eval-Env) |
| `exitMaxDistance` | 45 (Eval-Env) |
| `forceGuaranteedPath` | `false` (redundant seit v11: BFS-Exit-Platzierung garantiert Lösbarkeit) |
| DQN `buffer_size` | 200.000 |
| DQN `exploration_fraction` | 0.50 |
| `n_eval_episodes` | 50 |
| Timesteps | 1.000.000 |

Curriculum: leistungsbasiert (4 Phasen: 5–12 / 12–25 / 25–45 / Greedy Fine-Tune).
Observation (Env v11, seit 06.07.2026): **229 Features** = Grid 15×15 (225) + HP + exitDx/exitDy + step_frac.
Legacy-Modelle (231-dim, vor 06.07.2026): `StoneforgeWorldEnv(..., include_energy_inventory=True)`.
RecurrentPPO: `batch_size=8` (**NICHT 64!** — 64 destabilisiert den Critic: EV bleibt ≈ 0,1,
SR oszilliert chaotisch; nachgewiesen 07.07.2026 mit 4 Läufen + A/B, siehe CHANGELOG v2026-07-07.4),
`ent_coef=0.05`, Device **CPU** (MPS ist langsamer!).

---

## Verfügbare Algorithmen

| Algo | Aktionsraum | Status | Modell-Ordner |
|------|-------------|--------|---------------|
| PPO | Discrete ✓ | **Aktiv** | `models/ppo` |
| DQN | Discrete ✓ | **Aktiv** | `models/dqn` |
| A2C | Discrete ✓ | **Aktiv** | `models/a2c` |
| SAC | nur Box ✗ | nicht nutzbar | — |
| TD3 | nur Box ✗ | nicht nutzbar | — |

---

## Environment

Kein Wrapper-Stack mehr: `StoneforgeWorldEnv` wird direkt verwendet.
Aktionsraum: `Discrete(4)` — 0=hoch, 1=runter, 2=links, 3=rechts.
Mining/Bauen/Kampf sind seit Env v11 **im C++-Binding entfernt** (Aktionen 4–8 → RuntimeError);
sie existieren nur noch im spielbaren Client. Die früheren Wrapper
`ExitPotentialFieldWrapper`/`ReducedActionEnv` liegen archiviert in `OLD/`.

---

## ⚠️ DOKUMENTATIONSPFLICHT — Immer einhalten!

**Nach jeder Änderung und nach jeder Messung:**

1. `CHANGELOG.md` aktualisieren
2. Neue Version anlegen (Schema: `v1.x — Datum — Kurzbeschreibung`)
3. Folgendes dokumentieren:
   - **Was wurde geändert?** (Datei, Parameter, vorher → nachher)
   - **Warum?** (Problem das beobachtet wurde, Hypothese)
   - **Ergebnis?** (50-Seed-Eval: success/50, mean_len, mean_return)

**Format für neue Ergebnisse:**

```markdown
| Algorithmus | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|-------------|---------|--------------|----------------------|---------------|-------|
| PPO         | X / 50  | X.X %        | XXXX.X               | −XX.XX        | TT.MM.JJJJ |
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

- **PBRS läuft auf BFS-Distanz** (`simulation.cpp`, β=2.5, γ=0.999 = RL-γ) — policy-invariant.
  Luftlinien-/Manhattan-Potential wäre irreführend; NIE darauf zurückbauen.
- **Biom-Schwellwerte sind hartkodiert** in `World::sampleBaseTile()` (world.cpp) — NICHT über
  `game_config.json` tunebar (die früheren cold/warm/moss-Keys waren tot und wurden entfernt).
- **WorldGen-Config ist prozess-global** (C++): `StoneforgeWorldEnv` stempelt sie bei jedem
  `reset()` neu. Bei direkter Nutzung des C++-Bindings ohne Env-Wrapper daran denken!
- **Legacy-Obs:** Modelle von vor dem 06.07.2026 (231-dim) brauchen
  `include_energy_inventory=True` beim Env, sonst Shape-Mismatch.
- **MPS/GPU bringt nichts:** CPU ist bei diesem Netz immer schneller (gemessen 06.07.2026).
- **`batch_size=64` destabilisiert den LSTM-Critic** (EV ≈ 0,1, SR oszilliert 10–74 % ohne
  Konvergenz). batch=8 → ~8× mehr Gradientenschritte pro Rollout, EV > 0,85, stabiler Anstieg.
  Die frühere 64er-„Validierung" beruhte auf einem Einzel-Snapshot (07.07.2026 widerlegt).
- **Eval-Episoden-Cap < 4000 verfälscht die SR massiv** (gleiches Modell: 48 % @Cap 600 vs.
  86 % @Cap 4000). Evals IMMER mit Cap 4000 (= Env-maxSteps) fahren; kostet nur ~21 s/Eval.
- **Einzelne Eval-Snapshots sind keine Validierung:** Die Lerndynamik kann chaotisch sein
  (transiente Hochphasen). Hyperparameter-Entscheidungen nur auf Basis ganzer Eval-KURVEN treffen.
- **Zeitbasiertes Curriculum** → Reward-Kollaps. Seit v1.1 leistungsbasiert.
- **Rebuild vergessen** nach Änderungen an C++ oder `game_config.json` → Crash oder falsche Ergebnisse.
- **Modellpfade**: Alle Modelle liegen jetzt in `models/`, nicht mehr in `best_models_*/`.

---

## Zielkriterium (Projektarbeit)

- **Testset A** (Seeds 7000–7049): ≥ 70 % Success Rate
- **Testset B** (Holdout, Seeds 8000–8049): ≥ 60 % Success Rate
- Pro Konfiguration: mind. 3 Trainingsläufe → Mittelwert + Standardabweichung angeben
