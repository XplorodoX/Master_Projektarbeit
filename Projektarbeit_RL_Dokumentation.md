# RL Dokumentation fuer die Projektarbeit

Stand: 27.04.2026

## 1) Ziel

Ziel der RL-Experimente ist ein Agent, der in prozedural generierten Welten den Exit robust findet (Generalisierung auf unbekannte Seeds).

## 2) Technischer Rahmen

- Environment: `python/stoneforge_env.py`
- Core-Simulation: `src/core/simulation.cpp`
- Training: `python/train.py`
- Python-Binding: `src/python/py_module.cpp`
- Visualisierung: `python/ai_play.py`

## 3) Relevante Implementierungsentscheidungen

### 3.1 Seed-Randomisierung pro Episode

In `python/stoneforge_env.py` wird bei `reset(seed=None)` ein neuer Welt-Seed aus Gym RNG gezogen. Damit trainiert der Agent nicht auf einer einzelnen Karte.

### 3.2 Zielrichtung in der Observation

In `src/core/simulation.cpp` werden `exitDx` und `exitDy` in die Observation aufgenommen. Dadurch hat der Agent eine direkte Zielrichtungsinformation (zusatzlich zum lokalen Grid).

### 3.3 Aktionsraum reduziert

In `python/train.py` wird via `ReducedActionEnv` auf Bewegungsaktionen beschraenkt:

- 0: hoch
- 1: runter
- 2: links
- 3: rechts

Mining wird fuer Training ausgeschlossen, um Lernrauschen zu reduzieren.

### 3.4 Mining im Playback gesperrt

In `python/ai_play.py` wird Mining bei der Visualisierung abgefangen (`sanitize_action`), damit beide Agenten im Vergleich nicht abbauen.

### 3.5 Curriculum-Einstellungen in der Welt

In `assets/base/game_config.json` wurde ein leichteres Curriculum fuer Navigation gesetzt:

- `exitMinDistance = 35`
- `exitMaxDistance = 45`
- `forceGuaranteedPath = true`

## 4) Reproduzierbarer Ablauf

### 4.1 Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
```

### 4.2 Training

```bash
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
python python/train.py --algo ppo --timesteps 500000
python python/train.py --algo dqn --timesteps 500000
```

### 4.3 Harte 50-Seed-Evaluierung

Verwendet wurden feste Seeds `7000..7049` (nicht zufaellig zur Laufzeit).

```bash
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
python - <<'PY'
import numpy as np
from stable_baselines3 import PPO, DQN
from stoneforge_env import StoneforgeWorldEnv

seeds = list(range(7000, 7050))

def evaluate(model, name):
    env = StoneforgeWorldEnv()
    succ, lens, rets = 0, [], []
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        done = False
        ep_ret = 0.0
        steps = 0
        reached = False
        while not done and steps < 4000:
            action, _ = model.predict(obs, deterministic=True)
            a = int(action)
            if a == 4:
                a = 7
            obs, r, term, trunc, info = env.step(a)
            ep_ret += float(r)
            steps += 1
            if info.get("reached_exit", False):
                reached = True
            done = term or trunc
        succ += int(reached)
        lens.append(steps)
        rets.append(ep_ret)
    print(f"{name}: success={succ}/50 ({succ/50:.1%}), mean_len={np.mean(lens):.1f}, mean_return={np.mean(rets):.2f}")

ppo = PPO.load("best_models_ppo/best_model.zip")
dqn = DQN.load("best_models_dqn/best_model.zip")
evaluate(ppo, "PPO")
evaluate(dqn, "DQN")
PY
```

## 5) Aktuelle Ergebnisse (50-Seed-Test)

Messung vom 27.04.2026:

- PPO: `3/50` Erfolg (`6.0%`), `mean_len = 1857.6`, `mean_return = -112.87`
- DQN: `20/50` Erfolg (`40.0%`), `mean_len = 1383.1`, `mean_return = -3.43`

Interpretation:

- DQN ist im aktuellen Setup deutlich robuster als PPO.
- Das Ziel ist erreichbar und lernbar, aber noch nicht auf dem gewuenschten Zuverlaessigkeitsniveau.

### 5.1 Direkter Vergleich PPO vs DQN (gleiche Bedingungen)

Vergleichsbasis:

- gleiches Environment
- gleiche Trainingsdauer (`500000` Timesteps)
- gleiches Eval-Seedset (`7000..7049`)
- Mining im Eval geblockt

| Algorithmus | Success Rate | Mittlere Episodenlaenge | Mittlerer Return |
|---|---:|---:|---:|
| PPO | 6.0% (3/50) | 1857.6 | -112.87 |
| DQN | 40.0% (20/50) | 1383.1 | -3.43 |

Fazit aus der Messung: DQN ist in diesem Projektstand klar die bessere Baseline.

### 5.2 Warum ist DQN hier besser als PPO?

Die folgende Einordnung beschreibt die wahrscheinlichsten Gruende im aktuellen Setup:

1. Diskreter, kleiner Aktionsraum passt gut zu DQN.
    Der Agent nutzt nur 4 Bewegungsaktionen. Q-Learning in kleinem diskretem Raum ist oft sehr effizient.

2. Wiederverwendung von Erfahrung (Replay Buffer) hilft bei seltenen Erfolgspfaden.
    DQN lernt off-policy und kann erfolgreiche Trajektorien mehrfach verwenden.

3. PPO reagiert empfindlicher auf stark schwankende Episoden.
    In den Trainingslogs waren bei PPO deutliche Eval-Schwankungen sichtbar, waehrend DQN stabilere Verbesserungen zeigte.

4. Das Reward-Signal bleibt trotz Verbesserungen relativ anspruchsvoll.
    In diesem Bereich profitiert DQN oft von konservativerem TD-Update-Verhalten.

Wichtig: Das ist keine allgemeingueltige Aussage "DQN ist immer besser", sondern eine Aussage fuer genau dieses Environment und die aktuelle Konfiguration.

### 5.3 Dokumentationsrelevante Kernaussage fuer die Projektarbeit

Die Experimente zeigen belastbar:

- Das Ziel-Finden ist lernbar (nicht mehr 0% Erfolg).
- Die Algorithmuswahl hat grossen Einfluss auf Zuverlaessigkeit.
- Unter identischen Bedingungen ist DQN aktuell dem PPO deutlich ueberlegen.

Das begruendet fuer den weiteren Projektverlauf eine DQN-first Strategie als primare Baseline.

## 6) Risiken und Validitaet

- Eval waehrend Training nutzt aktuell nur wenige Episoden (`n_eval_episodes=5`), daher starkes Rauschen.
- Einzelne Runs koennen stark schwanken (stochastische RL-Optimierung).
- Ein fixes 50-Seed-Testset ist gut fuer Vergleichbarkeit, sollte aber durch ein weiteres Holdout-Set ergaenzt werden.

## 6b) Optimierungen fuer prozedural generierte Welten (30.04.2026)

### 6b.1 Das Manhattan-Distanz-Problem (Kernproblem)

Der aktuelle Reward-Shaping-Term `0.10 * (distanceBefore - distanceAfter)` verwendet Manhattan-Distanz.
In einer Welt mit Waenden ist das IRREFUEHREND:

```
    [Exit]
     |||||  ← Wand trennt direkten Weg
     
[Agent]→ Richtung Luecke (Manhattan-Distanz steigt → negativer Reward!)
```

Der Agent bekommt Minusreward fuer den RICHTIGEN Schritt (zur Oeffnung laufen).
Das ist das haeufigste Versagen bei Navigation in prozeduralen Welten.

**Loesungsansaetze:**
1. Curriculum Learning: Fange so nah an dass Waende egal sind, Netz generalisiert dann.
2. Reward-Shaping nur im Sichtradius des Agenten aktiv lassen.
3. BFS-Distanz statt Manhattan (erfordert C++ Aenderung, aufwendig).

### 6b.2 Curriculum Learning — leistungsbasiert (30.04.2026)

In `python/train.py` implementiert: `CurriculumCallback` mit **leistungsbasiertem** Stufenwechsel.

**Problem des zeitbasierten Curriculums (beobachtet 30.04.2026):**
Der Agent wurde bei 45% des Trainings auf Stufe 3 (22–35 Tiles) gezwungen, obwohl Stufe 2 nicht gemeistert war. Reward kollabierte von -3 auf -50. Ursachen:
1. Replay Buffer voll mit Misserfolgen → erfolgreiche Episoden aus Stufe 2 verloren
2. Exploration bereits bei 5% (exploration_fraction=0.4 = nach 40% fertig) → kein Anpassen moeglich

**Fix: Stufe wechselt erst wenn Reward-Schwellwert erreicht:**

| Stufe | Exit-Distanz | Reward-Schwellwert | Zeitlimit (Sicherheit) |
|---|---|---|---|
| 1 | 5–12 Tiles | > -4.0 | 20% |
| 2 | 12–22 Tiles | > -18.0 | 45% |
| 3 | 22–35 Tiles | > -35.0 | 70% |
| 4 | 35–45 Tiles | — (Finale) | 100% |

Gemessen ueber gleitenden Mittelwert der letzten 50 Episoden.

**Weitere Fixes (30.04.2026):**
- `buffer_size`: 200K → 500K (erfolgreiche Episoden laenger im Speicher)
- `exploration_fraction`: 0.40 → 0.70 (Exploration bleibt hoch bis 70% des Trainings)

**Eval-Env bleibt immer auf 35-45 Tiles** → misst echten Fortschritt.

### 6b.3 Observation-Radius erhöht (30.04.2026)

`assets/base/game_config.json`:
- `observationRadius`: 5 → 7 (Agent sieht 11×11 → 15×15 Tiles; **rebuild noetig**)
- `maxSteps`: 2500 → 4000 (mehr Zeit fuer laengere Pfade)

### 6b.4 DQN-Hyperparameter verbessert (30.04.2026)

| Parameter | Alt | Neu | Grund |
|---|---|---|---|
| `buffer_size` | 100K | 200K | Mehr Erfahrung wiederverwendbar |
| `learning_starts` | 5K | 10K | Mehr Random-Exploration |
| `train_freq` | 1 | 4 | Stabileres Update |
| `target_update_interval` | 1000 | 500 | Schnellere Target-Anpassung |
| `n_eval_episodes` | 5 | 20 | Zuverlaessigeres Eval-Signal |
| Default `timesteps` | 500K | 1M | Mehr Training |

### 6b.5 Exit-Potential-Field Observation

### Motivation

Der Agent hatte exitDx/exitDy als rohe normalisierte Vektoren in der Observation. Das NN musste daraus selbst Richtung UND Distanz ableiten, was bei weit entferntem Exit einen flachen Gradienten erzeugt und das Lernen verlangsamt.

### Implementierung

In `python/stoneforge_env.py` wurde die Klasse `ExitPotentialFieldWrapper` ergaenzt. Sie haengt 9 neue Features an die Observation:

- **8 Richtungs-Samples** (Kompassrose): Fuer jede der 8 Himmelsrichtungen wird ein Samplepunkt im Abstand `SAMPLE_RADIUS=10` Tiles berechnet. Die Feldstaerke an diesem Punkt ist `1 / (1 + dist_zum_exit / FIELD_SCALE)`, mit `FIELD_SCALE=40` (ca. mittlere Exit-Distanz).
- **1 Proximity-Wert**: Feldstaerke direkt am Agenten.

Alle Werte liegen in `[0, 1]`.

**Mathematik** (kein C++ noetig, rein aus exitDx/exitDy berechenbar):
```
Samplepunkt in Richtung θ mit Radius R:
  sx = exitDx - cos(θ)*R
  sy = exitDy - sin(θ)*R
  dist = sqrt(sx² + sy²)
  staerke = 1 / (1 + dist / SCALE)
```

Der **Gradient** ueber die 8 Werte zeigt direkt in Richtung Exit. Das NN muss die nicht-lineare Transformation nicht mehr selbst erlernen.

### Wrapper-Reihenfolge

```
StoneforgeWorldEnv → ExitPotentialFieldWrapper → ReducedActionEnv
```

### Observation-Groesse

Vorher: `(2*r+1)² + 5` Features  
Nachher: `(2*r+1)² + 5 + 9` Features (+ 8 Richtungs-Samples + 1 Proximity)

---

## 7) Naechste Schritte fuer die Projektarbeit

1. DQN als Hauptbaseline weiterfuehren (z. B. 1M bis 2M Timesteps).
2. Eval waehrend Training auf 20 bis 50 Episoden erhoehen.
3. Drei Trainingslaeufe pro Konfiguration reporten (Mittelwert und Streuung).
4. Zweites Holdout-Seedset dokumentieren (Generalisation Test B).
5. Zielkriterium fuer Abgabe festlegen, z. B. `>= 70%` auf Testset A und `>= 60%` auf Testset B.

## 8) Assets fuer den Bericht

- TensorBoard Runs: `tensorboard_logs/ppo_run_11`, `tensorboard_logs/dqn_run_7`
- Modelle: `best_models_ppo/best_model.zip`, `best_models_dqn/best_model.zip`
- Live-Vergleich: `python/ai_play.py --model ... --model2 ...`
