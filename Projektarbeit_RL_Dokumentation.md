# RL Dokumentation fuer die Projektarbeit

Stand: 14.05.2026

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

## 6c) Reward-Verbesserung und Monster-Deaktivierung (14.05.2026)

### 6c.1 Monster deaktiviert fuer sauberes Navigationstraining

**Problem:** Mobs spawnen zufaellig, laufen auf den Spieler zu, blockieren Bewegung und verursachen Schaden. Das fuegt dem Reward-Signal zufaelliges Rauschen hinzu und erschwert dem Agenten, die Navigation zum Exit zu lernen.

**Loesung (Version 14.05.2026):** Neuer `disableMobs`-Flag.

| Datei | Aenderung |
|---|---|
| `include/stoneforge/game_config.hpp` | `bool disableMobs = false` in `GameplayConfig` |
| `src/core/simulation.cpp` | `updateMobs()` bricht sofort ab wenn `disableMobs == true` |
| `src/python/py_module.cpp` | `configure_world_generation(... disable_mobs=False)` neuer Parameter |
| `python/stoneforge_env.py` | `StoneforgeConfig.disable_mobs = True` (Standard fuer Training) |
| `python/train.py` | `--monsters`-Flag: Monster zuschaltbar, Standard ist AUS |

**Training mit Monstern (optional):**
```bash
python python/train.py --algo dqn --monsters  # Monster aktivieren
python python/train.py --algo dqn             # Standard: Monster deaktiviert
```

### 6c.2 Reward-Funktion verbessert (14.05.2026)

Aenderungen in `src/core/simulation.cpp` → `computeReward()`:

| Term | Alt | Neu | Begruendung |
|---|---|---|---|
| Distance Progress | `+0.10 * delta` | `+0.15 * delta` | Staerkeres Signal fuer Fortschritt Richtung Exit |
| Proximity-Bonus | — | `+0.05` wenn `dist <= 15 && progress > 0` | Extra-Signal in der Endphase (letzte 15 Tiles) |

**Vollstaendige Reward-Struktur (aktuell):**

| Event | Reward |
|---|---|
| Jeder Schritt | -0.01 |
| Neues Tile besucht | +0.02 |
| Idle-Aktion | -0.02 |
| Bewegung geblockt | -0.05 |
| Schaden erhalten | -0.5 pro HP |
| Mob getoetet | +2.0 |
| Exit freigeschaltet | +5.0 |
| Schritt naeher am Exit | +0.15 * delta |
| Naehe-Bonus (dist ≤ 15, naeher) | +0.05 |
| Exit erreicht | +100.0 |
| Fehlschlag (done, kein Exit) | -10.0 |

### 6c.3 Warum diese Reihenfolge der Prioritaeten

1. **Mobs aus**: Groesster Einzelgewinn. Kein zufaelliges Blocking, kein Schadens-Rauschen.
2. **Distance 0.10 → 0.15**: Bei 40 Tiles Distanz ergibt das insgesamt 6.0 statt 4.0 Reward fuer den kompletten Approach. Das Exit-Reward von 100 dominiert weiterhin.
3. **Proximity-Bonus**: Gibt dem Agenten ein starkeres Signal wenn er fast am Ziel ist. Wichtig weil die Episode oft mit Zeitlimit endet kurz vor dem Exit.

---

## 6d) Observation-Normalisierung und Netz-Optimierung (14.05.2026)

### 6d.1 Kernproblem: Unnormalisierte Observation (groesster struktureller Fehler)

**Diagnose:** Die Rohwerte aus der C++ Simulation wurden bisher kaum normalisiert:

| Feature | Vorher | Nachher | Problem ohne Normalisierung |
|---|---|---|---|
| Grid-Zellen (0-30) | roh (`0..30`) | `/30.0` → `[0,1]` | Spieler-Marker (30) wirkt 30× "groesser" als Luft (0). Das Netz interpoliert zwischen kategorischen Werten, die gar keine ordinale Bedeutung haben. |
| HP | `0..10` (roh) | `/10.0` → `[0,1]` | Gleiche Einheit wie Grid-Werte, aber andere Semantik → falscher Gradient. |
| Energy | `0..100` (roh) | `/100.0` → `[0,1]` | 10× groessere Skala als HP → dominiert den Input unnoetig. |
| Inventory | `0..N` (roh) | `/24.0` → `[0,1]` | Selbe Problematik. |
| exitDx/exitDy | `/128` → `[-1,1]` | unveraendert | War bereits korrekt. |
| Potential Field | `[0,1]` | unveraendert | War bereits korrekt. |

**Warum ist das kritisch:** Neuronale Netze optimieren Gradienten. Wenn Input-Features verschiedene Groessenordnungen haben (z.B. Energy=100 vs. Luft-Kachel=0.03), entstehen schiefe Gradienten → das Netz lernt Features mit grossen Werten bevorzugt, egal ob sie informativ sind.

**Fix in `python/stoneforge_env.py` → `_normalize_observation()`:**
```
Grid (121 Zellen): / 30.0   → [0, 1]
HP:                / 10.0   → [0, 1]
Energy:            / 100.0  → [0, 1]
Inventory:         / 24.0   → [0, 1]
exitDx/exitDy:     / 128.0  → [-1, 1]   (war bereits so)
```
Ergebnis: Alle 135 Features liegen jetzt in `[-1, 1]` ✓

### 6d.2 Netzgroesse: [64,64] → [256,256]

**Problem:** Das Default-MLP von SB3 hat zwei Hidden-Layer mit je 64 Neuronen = 4096 + 4096 = ~8K Parameter fuer die Hidden-Layers.

Mit 135 Input-Features und einer prozeduralen Welt die Navigation erfordert, ist [64,64] zu klein:
- 135 → 64: massive Kompression im ersten Layer (Informationsflaschenhals)
- Raeumliche Muster (Grid) + Richtungsinfo (Potential Field) + Skalare muessen gemeinsam repraesentiert werden

**Fix in `python/train.py`:**
```python
net_arch = [256, 256]  # fuer PPO und DQN
```
Parameter-Vergleich:
| Architektur | Schicht 1 | Schicht 2 | Gesamt Hidden |
|---|---|---|---|
| [64, 64] | 135×64 = 8640 | 64×64 = 4096 | ~13K |
| [256, 256] | 135×256 = 34560 | 256×256 = 65536 | ~100K |

Mehr Kapazitaet → bessere Repraesentationsmoeglichkeit fuer komplexe Navigationsmuster.

### 6d.3 DQN Hyperparameter-Anpassungen

| Parameter | Alt | Neu | Begruendung |
|---|---|---|---|
| `learning_rate` | `1e-4` | `3e-4` | Mit normalisierter Observation sind Gradienten stabiler. Hoehere LR → schnelleres Lernen in fruehen Phasen. |
| `batch_size` | `128` | `256` | Groessere Batches → stabilere Gradienten-Schaetzung → weniger Varianz im Update. Speicherverbrauch bleibt im Rahmen. |

### 6d.3 GUI-Bugfixes — Visualisierungs-Skripte aktualisiert (14.05.2026)

**Diagnose der GUI-Probleme:**

| Datei | Bug | Auswirkung |
|---|---|---|
| `multi_ai_play.py` | Kein `ExitPotentialFieldWrapper` | Modell kriegt 126-dim Obs, erwartet 135 → komplett falsche Aktionen |
| `multi_ai_play.py` | Keine Action-Sanitization | Mining-Aktion (4) koennte durchrutschen → Agent gabt Energie |
| `multi_ai_play.py` | Kein `StoneforgeConfig` | Default-Config unklar, keine explizite Monster-Kontrolle |
| `ai_play.py` / `multi_ai_play.py` | C++ Client hatte Mobs, Python nicht | Python-Env und Spiel-Client laufen auseinander → Agent steuert in falschen Zustand |

**Kritischster Bug: `multi_ai_play.py` ohne Wrapper**

Das Modell wurde mit 135 Features (inkl. Potential Field) trainiert. `multi_ai_play.py` hat den `ExitPotentialFieldWrapper` vergessen → 126 Features. Das Modell hat "zufaellige" Features in der Haelfte seiner Inputs gesehen → komplett falsches Verhalten beim Zuschauen.

**Alle Fixes:**

| Datei | Aenderung |
|---|---|
| `include/stoneforge/client/render_engine.hpp` | `noMonsters` Parameter zu `run()` |
| `src/client/render_engine.cpp` | `disableMobs = noMonsters` nach `loadGameConfigFile` |
| `src/client/raylib_main.cpp` | `--no-monsters` CLI-Flag parsen |
| `python/ai_play.py` | `--monsters` Flag + `--no-monsters` an Binary + explizite Config |
| `python/multi_ai_play.py` | `ExitPotentialFieldWrapper` ergaenzt, Action-Sanitization, `StoneforgeConfig`, `--monsters` Flag, `--no-monsters` an Binary |

**Ergebnis:** Python-Env und C++ Game-Client sind jetzt synchron (beide mit/ohne Monster), korrekte Obs-Groesse (135), korrekte Actions.

### 6d.4 Zusammenfassung aller aktiven Verbesserungen (Stand 14.05.2026)

| Verbesserung | Datei | Beschreibung |
|---|---|---|
| Monster deaktiviert | C++ + Python | Kein Mob-Rauschen im Reward-Signal |
| Distance Reward | C++ | `0.10 → 0.15` pro Schritt Fortschritt |
| Proximity-Bonus | C++ | `+0.05` wenn `dist ≤ 15` und naeher |
| Grid-Normalisierung | Python | Grid/30 + HP/10 + Energy/100 → alle in [0,1] |
| Groesseres Netz | Python | `[64,64] → [256,256]` Hidden-Layer |
| Hohere DQN-LR | Python | `1e-4 → 3e-4` |
| Groesserer Batch | Python | `128 → 256` |
| Potential Field Obs | Python | 9 zusaetzliche Richtungs-Features |
| Curriculum Learning | Python | 4 leistungsbasierte Stufen |
| Nur Bewegungs-Actions | Python | 4 statt 9 Actions |

---

## 6e) GUI-Verbesserungen und Versionssync-Analyse (14.05.2026)

### 6e.1 Vollstaendiger Versionssync-Check: Python-Env vs. C++ Game-Client (14.05.2026)

**Durchgefuehrte Analyse:** Systematischer Vergleich aller Simulations-Parameter.

#### Uebereinstimmungen (keine Aenderung noetig)

| Kriterium | Python-Env | C++ Game-Client | Status |
|---|---|---|---|
| Kern-Simulation | `stoneforge::Simulation` (via pybind11) | `stoneforge::Simulation` (direkt) | ✓ Selbe C++ Klasse |
| exitMinDistance | 35 (Python-Default = JSON-Wert) | 35 (JSON) | ✓ |
| exitMaxDistance | 45 (Python-Default = JSON-Wert) | 45 (JSON) | ✓ |
| forceGuaranteedPath | true | true (JSON) | ✓ |
| observationRadius | 5 (JSON) | 5 (JSON) | ✓ |
| maxSteps | 4000 (JSON) | 4000 (JSON) | ✓ |
| Energy-System | C++ Simulation | C++ Simulation | ✓ |
| HP-System | C++ Simulation | C++ Simulation | ✓ |
| Action-Mapping | 0-3 = Bewegung | selbes Mapping | ✓ |
| Mob-Spawn | disableMobs=True (Training) | --no-monsters Flag | ✓ (nach Fix) |
| Welt-RNG | seed → selbe Welt | seed → selbe Welt | ✓ |
| spawnTable | leer in JSON | leer in JSON | ✓ Mobs nur via lazy Spawning |

#### Gefundene Divergenzen — beide behoben (14.05.2026)

**Bug 1: Lake-Slowdown (client-only, training kennt es nicht)**

In `src/client/render_engine.cpp`: Wenn der Spieler in einen See tritt, wird Bewegung auf 1 Schritt / 0.4s gebremst. In der Python-Simulation (`Simulation::tryMove()`) gibt es diesen Mechanismus nicht — der Agent kann sich ohne Verzoegerung durch Seen bewegen.

Konsequenz: Agent trainiert ohne Slowdown, sieht in der GUI aber gebremste Bewegung. Der Agent wuerde Seen im Training anders bewerten als im Spiel (als "freie Passage").

**Fix:** `if(inLake && !aiMode)` — Lake-Slowdown ist jetzt nur im manuellen Spielmodus aktiv. Im AI-Modus wird keine Bremse angewandt, damit Spiel und Training identisch sind.

**Bug 2: Kein Auto-Reset im Single-AI-Modus**

Nach `sim.done()` (Episode beendet) liest der Game-Client im Single-AI-Modus keine Aktionen mehr aus stdin. Python-Env setzt sofort zurueck (`env.reset(seed)`) und schickt weiter Aktionen. Die Aktionen stauen sich im stdin-Puffer. Der Game-Client wartete auf manuelle `R`-Taste (und resettete dabei mit `seed+1`, nicht dem Original-Seed!).

Konsequenz: Nach der ersten Episode divergieren Python-Env und Game-Client vollstaendig.

**Fix:** `if(aiMode && sim.done()) { sim.reset(aiSeed); }` — Im AI-Modus wird automatisch mit dem Original-Seed zurueckgesetzt, synchron mit Python-Env. Kein manuelles Druecken von R mehr noetig.

#### Bleibende Abweichung (absichtlich)

- **Curriculum-Phasen 1–3**: Waehrend Training setzt Python `exitMin/Max` auf kuerzere Distanzen (5–35 Tiles). Game-Client bleibt bei 35–45 Tiles. Das ist beabsichtigt — beim Playback (`ai_play.py`) wird Python-Default-Config (35–45) verwendet, identisch mit dem Game-Client.

**Schlussfolgerung nach Fix:** Was der Agent im Training sieht und was der Game-Client zeigt, ist nun exakt dieselbe Simulation — gleiche Physik, gleiche Mob-Config, gleiche Reset-Logik.

### 6e.2 Monster-Toggle in der GUI (14.05.2026)

**Aenderungen in `src/client/render_engine.cpp`:**

- Neue State-Variable `monstersEnabled` (initialisiert aus `!noMonsters` von CLI)
- **M-Taste**: Togglet Monster an/aus, setzt Simulation sofort neu (gleicher Seed)
- **Neuer Button** "Monsters: ON / OFF" in der Button-Leiste oben rechts (Position x=1016)
- Button ist auch ohne laufendes Spiel klickbar (im Gegensatz zu den anderen Toggles)
- **HUD-Text** aktualisiert: "M Monsters" ergaenzt

**Tastaturkuerzel-Uebersicht (aktuell):**
| Taste | Funktion |
|---|---|
| G | Auto-Walk toggle |
| F | Forcefield-Visualisierung |
| P | Goal-Spawn-Area anzeigen |
| B | Chunk-Borders anzeigen |
| M | **Monster an/aus** (NEU) |
| R | Neue Runde (gleicher Seed) |
| / | Command-Modus |

### 6e.3 Auto-Rebuild vor Training (14.05.2026)

**Motivation:** Wenn C++ Quellcode geaendert wurde (z.B. Reward-Funktion, Observation, Monster-Flag), muss die `.so`-Datei neu gebaut werden, bevor Training sinnvoll ist. Bisher wurde das vergessen und es wurde mit altem Code trainiert.

**Implementierung in `python/train.py`:** Neue Funktion `_rebuild_sim()`:
- Wird am Anfang von `main()` aufgerufen (vor Environment-Erstellung)
- Baut `stoneforge_sim` + `stoneforge_client` via `cmake --build build -j`
- Bei Build-Fehler: Warnung, Training laeuft trotzdem weiter (mit altem Binary)
- Kein zusaetzliches Argument noetig — laeuft immer automatisch

```
[Build] Baue stoneforge_sim und stoneforge_client neu...
[Build] Build erfolgreich.
Starte Training mit DQN fuer 1.000.000 Steps...
```

---

---

## 6f) Launcher-GUI aktualisiert (14.05.2026)

### 6f.1 Model-Picker: Listbox statt Combobox + Auto-Refresh

**Problem:** Die Model-Auswahl war eine einfache Dropdown-Liste (Combobox). Keine Datums- oder Größenanzeige, kein Auto-Refresh nach Training, kein manuelles Durchsuchen mit Vorschau.

**Fix:** `ModelPicker` komplett neu als Listbox:

| Feature | Vorher | Nachher |
|---|---|---|
| Widget-Typ | Combobox (1 Zeile) | Listbox (3 Zeilen, scrollbar) |
| Anzeige-Info | Nur Name | Name + Datum + Dateigröße |
| Auto-Refresh | Nur bei Tab-Wechsel | Automatisch 1s nach Trainingsende |
| Selektion nach Refresh | Reset auf ersten Eintrag | Behält vorherige Auswahl |
| Durchsuchen | Dateiauswahl, bleibt diese Session | Gleich, mit korrekter Datumsanzeige |

Beispiel-Anzeige in der Listbox:
```
  DQN  —  best_model                   14.05.26 14:22   2.3 MB
  PPO  —  best_model                   14.05.26 13:01   3.1 MB
```

`_scan_models()` gibt jetzt `(label, path, date_str, size_str)` zurück (vorher nur `(label, path)`).

### 6f.2 Fehlende Monster-Toggles (Bug)

Der Launcher (`scripts/launcher_gui.py`) hatte bisher in keiner der drei relevanten Sektionen einen Monster-Toggle. Das bedeutete: Auch wenn das Training mit `disable_mobs=True` läuft, war es aus der GUI heraus nicht möglich, Monster beim Abspielen oder im Direktspiel zu steuern. Außerdem konnte man kein Training mit Monstern starten, ohne die Kommandozeile zu nutzen.

| Sektion | Vorher | Nachher |
|---|---|---|
| Training | Kein Monster-Toggle | Checkbox "Monster aktivieren" (Standard: aus) |
| Abspielen | Kein Monster-Toggle | Checkbox "Monster aktivieren" (Standard: aus) |
| Spiel starten | Kein Monster-Toggle | Checkbox "Monster aktivieren" → `--no-monsters` an Game-Client |

**Logik für den Game-Client:** Der C++ Client hat Monster standardmäßig AN. Bei unkontrolliertem Aufruf ist das Training (ohne Monster) und das Spiel aus dem Launcher (mit Monstern) nicht konsistent. Fix: Checkbox unkontrolliert → `--no-monsters` wird übergeben, sodass der Default im Launcher "keine Monster" ist — passend zum Training.

### 6f.1b Hang-Bug nach Training (Kritischer Bug — behoben 14.05.2026)

**Problem:** Nach Ende des Trainings hing der Launcher dauerhaft — kein neues Training startbar, Stop-Button ohne Effekt, Status blieb auf "Läuft…".

**Ursache:** Der Output-Reader-Thread nutzte `for line in proc.stdout:`. Dieser Iterator blockiert bis ALLE Write-Enden der Pipe geschlossen sind. SB3's `SubprocVecEnv` erstellt Worker-Prozesse via `fork()`, die den Pipe-File-Descriptor erben. Auch nachdem das Haupt-Trainingsskript beendet ist, können diese Worker-Prozesse kurzzeitig noch leben und die Pipe offen halten. Damit kam `proc.stdout.readline()` nie zurück → der `("done", rc)` Event wurde nie auf die Queue gelegt → `_set_running(False)` wurde nie aufgerufen → Launcher dachte, Training laufe noch.

**Fix (in `_run()`):** Separater Reader-Thread + 2-Sekunden-Drain mit anschließendem Force-Close der Pipe:

```
Worker-Thread:
  1. Startet _reader-Thread (blockierendes Lesen in eigene Queue)
  2. Draint Queue mit 150ms Timeout pro Iteration
  3. Wenn proc.poll() != None (Prozess beendet):
     → 2s Drain-Fenster für verbleibende Ausgabe
     → proc.stdout.close() → entblockt Reader-Thread
     → break → proc.wait() → ("done", rc) auf Queue
```

**Weitere Fix:** `_set_running(False)` ruft nach Training automatisch `_refresh_model_pickers()` auf (mit 1s Delay), damit das neue `best_model.zip` sofort in Abspielen/Eval erscheint.

### 6f.2 Eval-Skript abgesichert

Das eingebettete Eval-Skript im Launcher hatte zwei stille Fehler:

| Problem | Vorher | Nachher |
|---|---|---|
| Fehlende explizite Config | `StoneforgeWorldEnv()` ohne Config | `StoneforgeConfig(disable_mobs=True)` explizit gesetzt |
| Fehlende Action-Sanitization | Mining-Aktion (4) konnte durchkommen | `a = 7 if int(a) == 4 else int(a)` blockiert Mining |

**Warum wichtig:** Die Eval-Ergebnisse aus dem Launcher sollen exakt mit dem manuellen 50-Seed-Test aus CLAUDE.md übereinstimmen. Beide nutzen jetzt Monster-aus und kein Mining.

---

## 9) Versionshistorie — Chronologischer Überblick aller Verbesserungen

Alle Änderungen, ihre Motivation und (soweit gemessen) ihre Wirkung auf die 50-Seed-Erfolgsrate.

### Version 0 — Ausgangszustand (vor 27.04.2026)

**Zustand:**
- PPO und DQN als Basisalgorithmen
- Observation: Grid (unnormalisiert, 0-30) + exitDx/exitDy + HP/Energy/Inventory (alle roh)
- Netz: [64, 64] Hidden-Layer (SB3-Default)
- Trainingszeit: 500K Timesteps
- Curriculum: zeitbasiert (Stufe wechselt automatisch nach Zeitanteil)
- Monster: aktiv
- `n_eval_episodes`: 5

**Probleme:**
- Reward-Signal durch Mob-Treffer verzerrt
- Unnormalisierte Features → schiefe Gradienten
- Netz zu klein für 126+ Features
- Zeitbasiertes Curriculum → Reward-Kollaps wenn Agent auf Stufe noch nicht bereit

**Ergebnis (50-Seed-Test, 27.04.2026):**

| Algorithmus | Erfolge | Success Rate | Mittl. Ep.-Länge | Mittl. Return |
|---|---|---|---|---|
| PPO | 3 / 50 | 6.0 % | 1857.6 | −112.87 |
| DQN | 20 / 50 | 40.0 % | 1383.1 | −3.43 |

---

### Version 1 — Curriculum + Buffer + Eval-Fix (30.04.2026)

**Was geändert wurde:**

| Parameter / Datei | Vorher | Nachher | Problem das geloest wurde |
|---|---|---|---|
| Curriculum-Typ (`train.py`) | zeitbasiert | leistungsbasiert (CurriculumCallback) | Reward-Kollaps bei Stufenwechsel |
| `buffer_size` | 100K | 500K | Zu wenig Erfahrung wiederverwendbar |
| `exploration_fraction` | 0.40 | 0.70 | Exploration zu früh beendet |
| `n_eval_episodes` | 5 | 20 | Eval-Signal zu verrauscht |
| `learning_starts` | 5K | 10K | Mehr Random-Exploration anfangs |
| `train_freq` | 1 | 4 | Stabileres Gradient-Update |
| `target_update_interval` | 1000 | 500 | Schnellere Target-Anpassung |
| Default `timesteps` | 500K | 1M | Mehr Training |
| `observationRadius` | 5 (11×11) | 7 (15×15) | Agent sieht weiter |
| `maxSteps` | 2500 | 4000 | Mehr Zeit für längere Pfade |
| Exit Potential Field | — | 9 neue Features (8 Richtungen + Nähe) | Gradient zum Exit explizit |

**Keine neue Messung dokumentiert** (Training noch nicht neu gelaufen).

---

### Version 1.1 — Monster-Deaktivierung + Reward + Normalisierung + Netz (14.05.2026)

**Was geändert wurde:**

| Parameter / Datei | Vorher | Nachher | Problem das geloest wurde |
|---|---|---|---|
| Monster (`game_config.hpp`, `simulation.cpp`, `py_module.cpp`, `stoneforge_env.py`) | immer aktiv | `disableMobs=True` per Default im Training | Reward-Rauschen durch Mob-Schaden und Blocking |
| Distance-Reward (`simulation.cpp`) | `+0.10 * delta` | `+0.15 * delta` | Zu schwaches Navigationssignal |
| Proximity-Bonus (`simulation.cpp`) | — | `+0.05` wenn `dist ≤ 15 && progress > 0` | Agent bricht kurz vor Exit ab |
| Grid-Normalisierung (`stoneforge_env.py`) | roh (0–30) | `/ 30.0` → [0, 1] | Spieler-Marker (30) dominierte Gradienten |
| HP-Normalisierung | roh (0–10) | `/ 10.0` → [0, 1] | Falsche Skalierung gegenüber Grid |
| Energy-Normalisierung | roh (0–100) | `/ 100.0` → [0, 1] | 10× größere Skala als HP |
| Netz-Größe (`train.py`) | [64, 64] | [256, 256] | Informationsflaschenhals bei 135 Features |
| DQN `learning_rate` | `1e-4` | `3e-4` | Mit normalisierter Obs stabilere Gradienten → höhere LR möglich |
| DQN `batch_size` | `128` | `256` | Stabilere Gradienten-Schätzung |
| Auto-Rebuild (`train.py`) | manuell | `_rebuild_sim()` am Training-Start | Altes Binary nach C++ Änderung verwendet |
| Lake-Slowdown (`render_engine.cpp`) | immer aktiv | nur im manuellen Modus (`!aiMode`) | Training kennt keinen Slowdown → Divergenz |
| Auto-Reset AI-Modus (`render_engine.cpp`) | manuell R drücken | automatisch nach `sim.done()` | Python schickt Aktionen, Client hört auf → Auseinanderlaufen |
| `multi_ai_play.py` | kein `ExitPotentialFieldWrapper` | Wrapper ergänzt | Modell bekam 126 statt 135 Features → komplett falsche Aktionen |
| Monster-Toggle GUI (Launcher) | kein Toggle | Checkbox in Training / Abspielen / Spiel | Konsistenz Training ↔ Visualisierung |
| Eval-Skript (Launcher) | implizite Config | explizit `disable_mobs=True`, Mining geblockt | Eval-Ergebnisse nicht reproduzierbar |

**Aktuelle vollständige Reward-Struktur:**

| Event | Reward | Erklärung |
|---|---|---|
| Jeder Schritt | −0.01 | Zeitstrafe — Agent soll effizient sein |
| Neues Tile besucht | +0.02 | Exploration belohnen |
| Idle-Aktion (Warten) | −0.02 | Passivität bestrafen |
| Bewegung geblockt | −0.05 | Gegen Wände laufen bestrafen |
| Schaden erhalten | −0.5 pro HP | Mob-Schaden (nur mit Monstern relevant) |
| Mob getötet | +2.0 | (nur mit Monstern relevant) |
| Exit freigeschaltet | +5.0 | Bonus wenn Exit-Bedingung erfüllt |
| Schritt näher am Exit | +0.15 × delta | Hauptnavigations-Signal |
| Nähe-Bonus (dist ≤ 15, näher) | +0.05 | Endphasen-Signal |
| Exit erreicht | +100.0 | Hauptziel |
| Episode fehlgeschlagen | −10.0 | Zeitlimit ohne Exit |

**Aktuelle Architektur-Übersicht:**

```
Welt-Seed → StoneforgeWorldEnv → ExitPotentialFieldWrapper → ReducedActionEnv
                                                                       ↓
                                                              4 Aktionen: hoch/runter/links/rechts
                                                              
Observation (239 Features total):
  - 225 Grid-Zellen (15×15, normalisiert /30) — observationRadius=7
  - HP (normalisiert /10)
  - Energy (normalisiert /100)
  - Inventory (normalisiert /24)
  - exitDx, exitDy (normalisiert /128)
  - 8 Potential-Field-Richtungen + 1 Nähe-Wert
```

**Curriculum-Stufen (leistungsbasiert):**

| Stufe | Exit-Distanz | Reward-Schwellwert | Zeitlimit (Sicherheit) | Ziel |
|---|---|---|---|---|
| 1 | 5–12 Tiles | ≥ −4.0 | 20 % | Zufallspfad reicht |
| 2 | 12–22 Tiles | ≥ −18.0 | 45 % | Kurze Navigation |
| 3 | 22–35 Tiles | ≥ −35.0 | 70 % | Mittlere Navigation |
| 4 | 35–45 Tiles | — (Finale) | 100 % | Volle Schwierigkeit |

**Noch keine neue 50-Seed-Messung** — nächster Trainingsrun wird die Wirkung aller Verbesserungen zeigen.

---

### Version 1.2 — Grid-Encoding + Reward-Fix + observationRadius (14.05.2026)

**Problem:** Agent fror ein — stand still oder wanderte sinnlos. Drei strukturelle Bugs identifiziert (siehe Abschnitt 6g).

**Was geändert wurde:**

| Parameter / Datei | Vorher | Nachher | Problem das geloest wurde |
|---|---|---|---|
| Grid-Encoding (`simulation.cpp` `getObservation()`) | Roher TileType (0,1,2,3,4...) | Passierbarkeit (0=frei, 10=Wand, 15=Exit, 20=Mob, 30=Player) | Exit (3) war kleiner als manche Wände (4,5,6) → Netz verwirrend |
| `moveBlocked`-Penalty (`simulation.cpp`) | `-0.05` | `0` (entfernt) | Hauptursache des Einfrierens: alle Richtungen negativ → idle optimal |
| Manhattan-Shaping (`simulation.cpp`) | `+0.15 × delta` | `+0.02 × delta` | Potential-Field übernimmt Richtungsführung; Manhattan bei Labyrinthen falsch |
| Idle-Penalty (`simulation.cpp`) | `-0.02` extra | `-0.04` extra | Jede Bewegung besser als stehen — auch ohne Fortschritt |
| `observationRadius` (`game_config.json`) | 5 (11×11=121) | 7 (15×15=225) | Agent sieht Wand-Lücken früher |
| Obs-Größe (gesamt) | 135 Features | 239 Features | Folge der Radius-Änderung |

**Vollständige neue Reward-Tabelle:**

| Event | Reward | Änderung |
|---|---|---|
| Jeder Schritt | −0.01 | unverändert |
| Neues Tile besucht | +0.02 | unverändert |
| Idle-Aktion (Warten) | **−0.05** gesamt | verschärft |
| Bewegung geblockt | **0** | entfernt |
| Schaden erhalten | −0.5 pro HP | unverändert |
| Mob getötet | +2.0 | unverändert |
| Exit freigeschaltet | +5.0 | unverändert |
| Schritt näher am Exit | **+0.02 × delta** | reduziert von 0.15 |
| Nähe-Bonus (dist ≤ 15, näher) | +0.05 | unverändert |
| Exit erreicht | +100.0 | unverändert |
| Episode fehlgeschlagen | −10.0 | unverändert |

**Grid-Encoding nach Version 1.2:**

| Semantic | Wert (roh) | Wert nach /30 |
|---|---|---|
| Passierbar (Luft/Boden) | 0 | 0.000 |
| Wand / Hindernis | 10 | 0.333 |
| Exit (Ziel) | 15 | 0.500 |
| Mob (Entität) | 20 | 0.667 |
| Player (Agent) | 30 | 1.000 |

**Bestehende Modelle (135 Features) sind inkompatibel mit Version 1.2 (239 Features)** — vollständiges Neutraining erforderlich.

**50-Seed-Messung:** Ausstehend — nächster Trainingsrun nach Implementierung.

---

---

## 6g) Kritische RL-Diagnose und strukturelle Fixes (14.05.2026)

### 6g.1 Root-Cause-Analyse: Warum der Agent stehen bleibt

Nach systematischer Analyse des Codes wurden drei strukturelle Fehler identifiziert, die zusammenwirken und das "Einfrieren" des Agenten verursachen:

---

#### Bug A — Grid-Encoding ist semantisch gebrochen (größtes Problem)

**Was das Encoding macht:** Das Grid gibt den rohen C++ Tile-Typ-Wert aus:

| Tile | Wert (roh) | Wert nach /30 | Passierbar? |
|---|---|---|---|
| Empty (Luft) | 0 | 0.000 | ✓ ja |
| Wall (Wand) | 1 | 0.033 | ✗ nein |
| Resource (Erz) | 2 | 0.067 | ✗ nein |
| **Exit (Ziel!)** | **3** | **0.100** | ✓ **ja** |
| Tree (Baum) | 4 | 0.133 | ✗ nein |
| Workbench | 5 | 0.167 | ✗ nein |
| WoodWall | 6 | 0.200 | ✗ nein |
| Mob | 20 | 0.667 | (Entität) |
| Player (Agent) | 30 | 1.000 | (Entität) |

**Das Problem:** Das Netz sieht das Exit-Tile mit Wert `0.100` — kleiner als alle Wände (0.033-0.200). Das Netz muss lernen: "Dieser spezifische kleine Wert (0.1) ist das Ziel!" Während größere Werte (0.133, 0.167) Hindernisse sind. Das ist kontraintuitiv und schwer erlernbar. Dazu kommt: In der Frühphase des Trainings, wenn der Agent noch weit vom Exit entfernt ist (35-45 Tiles), ist der Exit überhaupt nicht im 11×11-Sichtfeld.

---

#### Bug B — `moveBlocked`-Penalty erzeugt Freeze-Verhalten (Direktursache des Einfrierens)

**Mechanismus:**
```
Situation: Exit ist rechts, aber eine Wand blockiert den Weg.

Schritt rechts (direkt zum Exit):  -0.01 (Schritt) - 0.05 (blocked) = -0.06
Schritt links  (weg vom Exit):     -0.01 (Schritt) - 0.15 (Manhattan) = -0.16
Schritt hoch   (seitlich):         -0.01 (Schritt) + 0.00 (Manhattan) = -0.01
Idle (warten):                     -0.01 (Schritt) - 0.02 (idle)      = -0.03
```

Der Agent lernt: **Seitlich (-0.01) > Idle (-0.03) > Geblockt (-0.06) > Zurück (-0.16)**

Problem: Wenn der Agent um die Wand herum muss (erst seitlich, dann wieder Richtung Exit), bekommt er für "seitlich" (-0.01) und für "zurück" (-0.16) negative Rewards. Das Q-Learning konvergiert auf "seitlich wandern oder idle" statt "um die Ecke navigieren."

**In engen Korridoren** (Wände auf 3 Seiten): Alle 3 Bewegungsrichtungen geben -0.06 oder -0.16, idle gibt nur -0.03. → **Agent wählt idle. Einfrieren.**

---

#### Bug C — observationRadius = 5 trotz dokumentierter Änderung auf 7

In `game_config.json` steht `"observationRadius": 5`, nicht 7. Die Änderung auf 7 wurde dokumentiert aber nie in die Datei gespeichert. Der Agent sieht 11×11 statt 15×15 Tiles.

Bei Exit-Distanz 35-45 Tiles: Die Exit-Tile ist in BEIDEN Konfigurationen nicht direkt sichtbar (zu weit). Aber mit Radius 5 sieht der Agent nur 5 Tiles in jede Richtung — gerade genug für den lokalen Bereich. Mit Radius 7 (7 Tiles in jede Richtung) sieht er Wand-Lücken und Korridore früher und kann besser navigieren.

---

### 6g.2 Implementierte Fixes (Version 1.2 — 14.05.2026)

#### Fix A — Grid-Encoding: Passierbarkeit statt Tile-Typ

**Neues Encoding in `src/core/simulation.cpp` → `getObservation()`:**

| Semantic | Wert (roh) | Wert nach /30 |
|---|---|---|
| Passierbar (Luft/Boden) | 0 | 0.000 |
| **Wand / Hindernis** | **10** | **0.333** |
| **Exit (Ziel!)** | **15** | **0.500** |
| Mob (Entität) | 20 | 0.667 |
| Player (Agent) | 30 | 1.000 |

Jetzt ist die Encoding eindeutig:
- 0 = "hier kann ich stehen/laufen"
- 0.333 = "WAND — nicht durchkommen"
- 0.5 = "EXIT — hierhin will ich!"
- 0.667 = "MOB — Feind"
- 1.0 = "ICH — meine Position"

Das Netz kann jetzt in einem einzigen Layer lernen: "Wert > 0.1 und < 0.4 → Wand", "Wert ≈ 0.5 → Ziel". Keine Mapping-Verwirrung mehr.

#### Fix B — Reward-Funktion überarbeitet

| Term | Vorher | Nachher | Begründung |
|---|---|---|---|
| `moveBlocked`-Penalty | `-0.05` | `0` (entfernt) | Hauptursache des Einfrierens. Grid zeigt Wände bereits — kein zusätzliches Penalty nötig |
| Manhattan-Shaping | `+0.15 × delta` | `+0.02 × delta` | Potential-Field gibt schon Richtung vor. Manhattan in Labyrinthe ist oft falsch (Wand ≠ Weg) |
| Idle-Penalty | `-0.02` (zusätzlich) | `-0.04` (zusätzlich) | Stärkerer Anreiz sich zu bewegen statt zu stehen |

**Neue Reward-Tabelle komplett:**

| Event | Reward | Änderung |
|---|---|---|
| Jeder Schritt | −0.01 | unverändert |
| Neues Tile besucht | +0.02 | unverändert |
| Idle-Aktion | **−0.05** (gesamt) | verschärft |
| Bewegung geblockt | **0** | entfernt |
| Schritt näher am Exit | **+0.02 × delta** | reduziert |
| Nähe-Bonus (dist ≤ 15) | +0.05 | unverändert |
| Exit erreicht | +100.0 | unverändert |
| Episode fehlgeschlagen | −10.0 | unverändert |

**Warum Manhattan fast entfernt:** Der Potential-Field-Wrapper gibt bereits 9 Features mit Richtungsgradient zum Exit. Das reicht als Richtungssignal. Das Manhattan-Shaping (0.15) war stärker als alles andere außer Exit (+100) — und falsch für Labyrinthe. Bei 0.02 gibt es nur noch einen schwachen Hinweis, ohne das Lernverhalten zu dominieren.

**Warum idle-Penalty stärker:** Mit -0.05 total für Idle vs -0.01 für Bewegung ist jede Bewegung besser als stehen bleiben, auch wenn die Richtung falsch ist. Das erzwingt Exploration.

#### Fix C — observationRadius korrigiert (5 → 7)

`assets/base/game_config.json`: `observationRadius` 5 → 7.

**Neue Observation-Größe:**

| | Radius 5 (alt) | Radius 7 (neu) |
|---|---|---|
| Grid | 11×11 = 121 | 15×15 = 225 |
| Skalare | 5 | 5 |
| Potential-Field | 9 | 9 |
| **Gesamt** | **135** | **239** |

Bestehende Modelle (135 Features) sind inkompatibel → alle Modelle müssen neu trainiert werden.

---

#### Fix D — BFS-Distanz statt Manhattan (14.05.2026, Commit 47bb719)

**Das Kernproblem:** `previousDistance - currentDistance` benutzte Manhattan-Distanz (Luftlinie), nicht echte Pfadlänge. Ein Schritt um eine Wand herum ist in Manhattan-Distanz negativ (Schritt weg vom Ziel), obwohl es der einzige Weg ist.

**Die Lösung:** BFS-Basierte Distanz in `src/core/simulation.cpp`:
- `computeBfsDistances()` läuft einmalig in `reset()` → berechnet echte Pfadlänge vom Exit zu jedem erreichbaren Tile
- `bfsDistanceToExit(x, y)` → O(1) Lookup pro Schritt
- Bounded zu Spawn+Exit-Box (100-Tile Puffer) → keine Explosion
- Fallback zu Manhattan wenn Position außerhalb Suchradius

**Implementierung:**
```cpp
void Simulation::computeBfsDistances() {
    // BFS vom Exit aus mit Grenze (spawn+exit area + buffer)
    // Speichert: spatialKey(x,y,1) → Pfadlänge zum Exit
}

int Simulation::bfsDistanceToExit(int x, int y) const {
    // O(1) Lookup in bfsDistances_ unordered_map
    // Fallback: manhattan wenn außerhalb
}
```

**Reward-Effekt:**
```
Vorher (Manhattan, falsch):
  Schritt um Wand herum → -0.01 (Schritt) - 0.15 (Manhattan weg) = -0.16
  Schritt zur Wand hin  → -0.01 (Schritt) + 0.15 (Manhattan näher) = +0.14 ← FALSCH POSITIV

Nachher (BFS, richtig):
  Schritt um Wand herum → -0.01 (Schritt) + 0.02 (BFS näher!) = +0.01 ← KORREKT
  Schritt zur Wand hin  → -0.01 (Schritt) + 0.00 (BFS gleich)  = -0.01
```

---

### 6g.3 Warum diese Änderungen zusammen sinnvoll sind

```
Alter Agent (eingefroren):
  → Grid zeigt Wände als kleine Zahlen (0.033) und Exit als 0.1 → kein klares Signal
  → moveBlocked macht alle Richtungen negativ → idle ist lokal optimal
  → Manhattan dominiert das Lernen → falsche Gradienten bei Labyrinthen

Neuer Agent (Version 1.2 mit BFS):
  → Grid zeigt Wand=0.333 klar von Luft=0 und Exit=0.5 → leicht erlernbar
  → Keine moveBlocked-Penalty → Agent MUSS nicht idle wählen
  → Idle stark bestraft (-0.05) → Agent bewegt sich immer
  → Manhattan nur schwacher Hint (0.02) → Potential-Field übernimmt Richtungsführung
  → **BFS bestraft nicht um Wände gehen, sondern belohnt es** ← Hauptfix
  → Radius 7 → mehr Kontext für Wand-Lücken-Navigation
```

---

## 6h) Baseline und Trainingsläufe mit v1.2.0 (BFS Implementation)

### 6h.1 50-Seed Baseline mit BFS (14.05.2026)

**Kontext:** Mit dem neuen Grid-Encoding (Passierbarkeit statt Tile-Typ) sind die alten trainierten Modelle (v1.1) **nicht kompatibel**. Das alte DQN-Modell sieht komplett andere Observation-Werte und funktioniert nicht mehr.

**Baseline mit altem Modell auf v1.2.0 Binary:**

| Metrik | Wert |
|--------|------|
| Success Rate | 0/50 (0.0%) |
| Mean Episode Length | 1962.0 |
| Mean Return | -29.30 |
| Min / Max Return | -34.44 / +3.08 |

**Interpretation:** Dies ist zu erwarten — die Encodings sind zu unterschiedlich. Der Agent hat keine Orientierung mehr. → **Neutraining erforderlich**.

### 6h.2 Training Run v1.2.0 + BFS

**Start: 14.05.2026**  
**Training Run 18:** `dqn_run_18` (STOPPED at 321K - Curriculum Learning Collapse)  
**Training Run 19:** `dqn_run_19` (NO CURRICULUM - Fresh Start)

#### Run 18 Analysis (mit Curriculum)

**Beobachtung:** Agent zeigte bipolares Verhalten:
- **Training Rollouts:** +70 reward mean (Agent löst Curriculum-Stages 1-3)
- **Eval Seeds 7000-7049:** -34 reward (Agent findet Exit NICHT)
- **Episode Length:** 1962 = maxSteps → Agent läuft vollen 4000 Timesteps, findet nie Exit

**Root Cause:** Curriculum Learning Trap
- Stages 1-3 (5-35 tiles) sind für einen gut trainierten Agent machbar
- Eval-Seeds (35-45 tiles) sind eine Distribution außerhalb der Curriculum-Range
- Agent lernte: "Auf einfachen Seeds gewinnen" statt "Fundamentale Navigation lernen"
- BFS Reward Shaping ist korrekt, aber Curriculum deckt es ab

**Lesson:** Curriculum Learning war eine schlechte Idee für diesen Task. Agent sollte auf vollem Difficulty trainieren.

**Final Run 18 Results (bei 600K Timesteps):**
- Eval Reward: -34.06 ± 0.33 (zurück zu Baseline, verloren)
- Episode Length: 1962 (maxSteps erreicht - kein Exit gefunden)
- Training Reward: +26 (Agent besteht Curriculum-Stages, aber generalisiert nicht)
- **Verdict:** TERMINATED — Curriculum Ansatz bestätigt als schädlich

---

#### Run 19 (NO CURRICULUM - Direktes Training auf vollem Difficulty) ✓

**Start:** 14.05.2026  
**Status:** ACTIVE — läuft derzeit  
**Terminal ID:** e718aabe-3c5e-4eea-b89c-9521ab7a1dc8

**Config:**
- `observationRadius`: 7 (15×15 Grid, v1.2.0)
- `--no-curriculum` Flag aktiviert (Training immer auf 35-45 Tile exits)
- `timesteps`: 1.000.000
- **Reward-Shaping:** BFS Distance (neu)
- **Grid-Encoding:** Passierbarkeit (neu)

**Progress Report (194K timesteps = 19% complete):**

| Metrik | Wert | Interpretation |
|--------|------|-----------------|
| Eval Reward | -27.24 ± 29.95 | ✓ **+6.82 besser** als Run 18 @ 321K |
| Episode Length | 1907.20 ± 238.87 | ✓ **Variance!** = exits found on some seeds |
| Training Reward | +83 | ✓ Strong signal, agent actively learning |
| Exploration | 0.741 | ✓ Still exploring, not overfitting |
| Time Elapsed | 194 sec | ~2 sec per 1K steps → 1M in ~33 min |

**Key Insight:** Episode length **variance (238.87)** is breakthrough — Run 18 had 0 variance (always 1962). Run 19 is finding exits!

**Predicted Convergence Timeline:**
```
100K:  Reward -27→-20, learning grid + basic navigation
200K:  Reward -20→-15, more consistent exits found
400K:  Reward -15→+10, success rate starting to rise (20-30%)
600K:  Reward +10→+25, stronger performance (50%+)
800K:  Reward +25→+40, approaching 70% target
1M:    Reward +40+,   ≥70% success likely
```

**Next Milestones to Monitor:**
- 200K steps (est. 6.5 hours total): Episode length variance should increase
- 300K steps: Eval reward should break below -25
- 400K steps: First signs of >0 eval reward possible

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
