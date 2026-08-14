# CLAUDE.md — Stoneforge RL Projekt

## Leseregeln für Agents

1. `CHANGELOG.md` ist Pflichtlektüre, bevor irgendetwas geändert oder gemessen wird.
2. Zahlen für Bericht und Projektarbeit stammen ausschließlich aus dem Abschnitt
   [Berichtsfähiger Stand](#berichtsfähiger-stand). Werte aus dem Changelog oder aus
   älteren Modellordnern sind historisch und nicht vergleichbar.
3. Evaluiert wird ausschließlich mit `scripts/eval_baselines.py`. Kein eigenes
   Eval-Snippet, unter keinen Umständen.
4. Kein Experiment ohne Changelog-Eintrag (siehe [Dokumentationspflicht](#dokumentationspflicht)).
5. Vor jeder Aussage über Modellqualität den Abschnitt
   [Bekannte Fallstricke](#bekannte-fallstricke) lesen — die meisten naheliegenden
   Schlüsse sind in diesem Projekt bereits einmal widerlegt worden.
6. Dies ist eine **Gemeinschaftsarbeit zweier Autoren**. Vor Änderungen an `src/`
   den Abschnitt [Gemeinschaftsarbeit](#gemeinschaftsarbeit) lesen, dort liegt fremde
   Zuständigkeit.

---

## Projektübersicht

RL-Agent, der in prozedural generierten 2D-Welten (Stoneforge) den Exit finden soll.
Ziel ist die Generalisierung auf unbekannte Seeds, nicht das Overfitting auf einzelne
Karten.

| | |
|---|---|
| Algorithmen | PPO · DQN · A2C · RecurrentPPO (alle via Stable-Baselines3) |
| Schnittstelle | Gym-API |
| Kern | C++, angebunden über pybind11 |
| Abgabeform | eine gemeinsame Arbeit von Laurin und Florian, siehe unten |

---

## Gemeinschaftsarbeit

Die Arbeit wird als **ein Dokument** abgegeben und besteht aus zwei aufeinander
aufbauenden Teilen. Laurin entwickelt zuerst das Spiel samt Weltgenerierung, Florian
trainiert darauf anschließend die RL-Algorithmen. Der zweite Teil setzt den ersten
voraus; ohne die fertige Umgebung existiert keine Trainingsgrundlage.

| | Teil I | Teil II |
|---|---|---|
| Autor | **Laurin** | **Florian** |
| Gegenstand | Spiel, Weltgenerierung, Umgebungshärte | Reinforcement Learning auf dieser Umgebung |
| Kernfragen | Wanddichte, Umwegfaktor, Lösbarkeit, Biome, prozedurale Erzeugung | Algorithmenauswahl, Curriculum, Gedächtnis (LSTM gegen MLP), Generalisierung |
| Repository | `src/core/`, `src/python/`, `assets/base/game_config.json`, `scripts/probe_world_geometry.py`, `scripts/eval_hard_world.py` | `python/`, `scripts/train*.py`, `scripts/eval_baselines.py`, `scripts/analyze_agent.py`, `models/`, `logs/` |
| Abhängigkeit | eigenständig | baut vollständig auf Teil I auf |

### Regeln für die Zusammenarbeit

- **Änderungen an `src/` sind Eingriffe in Laurins Teil.** Sie werden nicht ohne
  Absprache vorgenommen. Das gilt besonders für `world.cpp` und `simulation.cpp`.
- **Jede Änderung an der Weltgenerierung erzeugt eine neue Env-Version** und entwertet
  sämtliche zuvor gemessenen Modellwerte. Der Zusammenhang ist im Abschnitt
  [Berichtsfähiger Stand](#berichtsfähiger-stand) bereits dokumentiert und ist der
  teuerste Fehler, den dieses Projekt machen kann: Teil II verliert dabei rückwirkend
  seine Datenbasis.
- **Die Umgebung ist die Schnittstelle zwischen beiden Teilen.** Env-Version,
  Observationsformat (229 Features, Env v11) und Aktionsraum (`Discrete(4)`) sind
  Vertragsgegenstand und werden nur gemeinsam geändert.
- **Kennzahlen der Welt gehören Teil I.** Umwegfaktor 1,12, Wanddichte 0,238 und die
  Befunde zur zellulären Glättung stammen aus Laurins Untersuchung. Teil II zitiert sie,
  misst sie nicht neu.
- **Ergebnisse der Modelle gehören Teil II.** Teil I zitiert sie, interpretiert sie aber
  nicht eigenständig.

### Kennzeichnung im Dokument

Die Autorschaft muss pro Kapitel eindeutig erkennbar sein.

- **Übersichtstabelle** im Vorspann der Arbeit, die jedes Kapitel einer Person oder
  „gemeinsam" zuordnet.
- **Marker in der Kapitelüberschrift**, Schema: `## 4 Weltgenerierung (Laurin)`.
- **Gemeinsam verfasste Kapitel** werden ebenso gekennzeichnet: `(gemeinsam)`.
- Bei geteilten Kapiteln wird auf Abschnittsebene weitergekennzeichnet, nicht feiner.
  Absatzweise Zuordnung ist unlesbar.

Vorschlag für den Dokumentaufbau, vor der Abgabe mit der betreuenden Person abzustimmen:

```
Titelblatt · Eidesstattliche Erklärungen (getrennt, je eine pro Person)
Autorschaftsübersicht (Tabelle Kapitel → Autor)
1  Einleitung                                    (gemeinsam)
2  Grundlagen                                    (gemeinsam)
3  Stoneforge: Spiel und Architektur             (Laurin)
4  Prozedurale Weltgenerierung                   (Laurin)
5  Analyse der Umgebungshärte                    (Laurin)
6  Methodik des Reinforcement Learning           (Florian)
7  Training und Curriculum                       (Florian)
8  Evaluierung der Modelle                       (Florian)
9  Zusammenfassung und Ausblick                  (gemeinsam)
Literatur · Anhänge (je Teil getrennt nummeriert)
```

---

## Berichtsfähiger Stand

**Einzige zitierfähige Quelle für Ergebniszahlen.** Modellwerte aus verschiedenen
Env-Versionen sind nicht vergleichbar. Berichtsfähig ist ausschließlich der v12-Stand
(n = 7) unter dem Standard-Eval.

### Trainierte Modelle (v12, `models/ppo_lstm_curriculum_v12_s1..s7`)

Quelle: `logs/eval_results/baselines.json` (5-Wiederholungs-Protokoll) — dieselbe
Datei und Messkampagne wie MLP-Kontrollgruppe und ungelernte Referenzen.

| Modell | Testset | Success Rate |
|--------|---------|--------------|
| LSTM (RecurrentPPO) | A (Seeds 7000–7049) | 64,6 % ± 10,1 (stochastisch) |
| LSTM (RecurrentPPO) | B (Holdout, 8000–8049) | 68,8 % ± 15,5 (stochastisch) |
| MLP (PPO) | A (Seeds 7000–7049) | 33,5 % ± 25,5 (stochastisch) |
| MLP (PPO) | B (Holdout, 8000–8049) | 35,8 % ± 30,5 (stochastisch) |

Die früher kanonischen LSTM-Werte 65,7 % ± 12,4 / 66,9 % ± 12,8 stammten aus dem
Einzelmessungs-Lauf vom 17.07.2026 (kein 5-Wiederholungs-Protokoll) und mischten sich
in der Arbeit mit MLP-Werten aus der späteren Kampagne. Vereinheitlicht am 14.08.2026,
siehe CHANGELOG v2026-08-14.2. Die deterministische LSTM-Zusatzmessung
(29,1 % ± 8,0 A / 32,6 % ± 12,7 B, Einzelmessung auf den Testset-Seeds) bleibt
unverändert gültig und wird als solche gekennzeichnet.

### Ungelernte Referenzen

Quelle: `logs/eval_results/baselines.json`, Testset A.

| Referenz | Success Rate | Pfadeffizienz |
|----------|--------------|---------------|
| Random | 5,2 % ± 2,7 | 0,021 |
| Kompass-Zufallslauf ε = 0,9 | 92,0 % ± 5,1 | 0,047 |

Die früher notierten Werte „8 % / 89 %" waren veraltet und widersprachen der
Projektdokumentation. Korrigiert am 05.08.2026 gegen die Rohdaten.

### Die Pfadeffizienz rettet das Ergebnis nicht

v12 (LSTM) liegt im Mittel bei 0,053 (Einzelläufe 0,033–0,060) und damit nur knapp über
dem ε=0,9-Zufallslauf (0,047). Der ε=0,3-Kompass erreicht 0,177 und ist rund 3,3-mal
wegeffizienter als das trainierte Modell; das MLP kommt auf 0,067, was wegen des
Selektionseffekts bei niedriger SR nicht als bessere Wegfindung zu lesen ist. Das LSTM
erkauft seine höhere Erfolgsquote durch mehr Herumlaufen, nicht durch bessere Wege.
Quelle: `logs/eval_results/baselines.json` (die ältere
`baselines_and_models.json` ist eine Einzelmessungs-Vorstufe, nicht zitieren).

### Historische Stände — nicht zitieren

| Ordner | Warum nicht vergleichbar |
|--------|--------------------------|
| `ppo_phase4` | MLP mit BFS-Orakel, Env vor v11. 100 % deterministisch nur unter dem damaligen Kurzdistanz-Protokoll; unter Standard-Eval (exit 35–45) 32 % stochastisch / 0 % deterministisch |
| `ppo_phase5` | 72 % unter altem Protokoll |
| `ppo_phase3` | ~56 % SR, Env vor v11 |
| `ppo_baseline`, `ppo_bfscompass` | frühe Vorstufen |

---

## Projektstruktur

```
.
├── admin/                       # Admin-Anleitung (Build, Start, Eval)
├── bin/                         # Kompilierte Binaries (nach Build)
├── docs/                        # Projektdokumentation, Exposé, Paper, Report
│   └── papers/                  # Referenz-Forschungsliteratur
├── models/                      # Trainierte Modelle (.zip) — ALLE hier, nicht in best_models_*/
│   ├── ppo_baseline/            # historisch
│   ├── ppo_bfscompass/          # historisch, BFS-Compass-Feature
│   ├── ppo_phase3/              # historisch
│   ├── ppo_phase4/              # historisch
│   ├── ppo_phase5/              # historisch
│   ├── ppo_lstm_curriculum_v12_s1..s7/   # AKTUELLE ENDERGEBNISSE (n=7)
│   └── ...                      # weitere Experimente
├── OLD/                         # Veralteter Code, archiviert statt gelöscht
│   └── scripts/                 # Alter CLI-Launcher, Shell-Skripte
├── python/                      # Importierbare RL-Bibliothek (via PYTHONPATH)
│   └── stoneforge_env.py        # Gym-Environment + Wrapper
├── scripts/                     # Ausführbare Skripte
│   ├── launcher_gui.py          # HAUPT-EINSTIEGSPUNKT (GUI)
│   ├── train_curriculum.py      # ERZEUGT DIE BERICHTSFÄHIGEN ERGEBNISSE (v12, n=7)
│   ├── train.py                 # Einzelphasen-Training, NICHT Quelle der v12-Zahlen
│   ├── eval_baselines.py        # KANONISCHES EVAL-PROTOKOLL (SR + Pfadeffizienz)
│   ├── probe_world_geometry.py  # Umwegfaktor, Wanddichte, Lösbarkeit
│   ├── smoke_test_algo_switch.py# Schnelltest des --algo-Pfads (rppo/ppo)
│   ├── watch_agent.py           # Grafische Agent-Visualisierung
│   ├── analyze_agent.py         # Verhaltensanalyse
│   ├── eval_hard_world.py       # DEFEKT, siehe Offene Punkte
│   ├── eval_temperature.py      # Temperatur-Sweep Benchmark
│   └── setup_env.sh             # Umgebung aktivieren + PYTHONPATH setzen
├── src/                         # C++ Quellcode
│   ├── core/simulation.cpp      # Kernsimulation (PBRS liegt hier)
│   └── python/py_module.cpp     # Pybind11-Binding
├── assets/base/                 # Spielkonfiguration (game_config.json)
├── logs/                        # Laufzeit-Logs
│   ├── eval_results/            # baselines.json, baselines_and_models.json
│   └── tensorboard/             # TensorBoard-Runs (ppo_*, dqn_*)
├── toolgen/                     # Wartungsskripte (fix_changelog.py)
├── screenshots/  videos/  wireframes/
├── requirements.txt
├── CHANGELOG.md                 # PFLICHTLEKTÜRE: alle Versionen, Ergebnisse
└── CMakeLists.txt
```

---

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
```

**Rebuild nötig:** nach jeder Änderung am C++-Code. Wird er vergessen, folgen Crash oder
falsche Ergebnisse.

**Rebuild nicht nötig:** nach Änderungen an `game_config.json`. Die Datei wird zur
Laufzeit gelesen (`StoneforgeCoreEnv`-Konstruktor, `src/python/py_module.cpp:44-48`; im
Client und headless über `loadGameConfigFile()`). CMake kopiert keine Assets. Ein
JSON-Patch wirkt sofort für jedes danach **neu konstruierte** Env; bereits bestehende
Env-Instanzen behalten ihre Werte. Nachgemessen am 05.08.2026, siehe CHANGELOG
v2026-08-05.1.

---

## Einstiegspunkte

### Launcher (empfohlen)

```bash
python scripts/launcher_gui.py
```

Bietet Training, Evaluation, Abspielen und Build in einer Oberfläche.

### Training

```bash
# Berichtsfähige Läufe (Curriculum, v12)
python scripts/train_curriculum.py --algo rppo    # RecurrentPPO (LSTM)
python scripts/train_curriculum.py --algo ppo     # MLP-Kontrollgruppe für F2

# Einzelphasen-Training
python scripts/train.py --algo ppo --timesteps 1000000
python scripts/train.py --algo dqn --timesteps 1000000
python scripts/train.py --algo a2c --timesteps 1000000
```

`train.py` speichert nach `models/{algo}/best_model.zip`, TensorBoard-Logs nach
`logs/tensorboard/`.

Curriculum: leistungsbasiert in vier Phasen (5–12 / 12–25 / 25–45 / Greedy Fine-Tune).
Seit dem 05.08.2026 laufen `rppo` und `ppo` auf **identischem** Curriculum; Unterschiede
bestehen nur architekturspezifisch bei `policy`, `batch_size` und `net_arch`, alles
Übrige ist geteilt.

Gemessener Durchsatz: MLP 5.989 Steps/s, LSTM 104 Steps/s. Die im Anhang genannten
„8 Stunden pro Lauf" gelten **nur** für das LSTM.

---

## Kanonisches Eval-Protokoll

**Es gibt genau ein gültiges Protokoll. Nicht selbst nachbauen.**

```bash
python scripts/eval_baselines.py --models     # Baselines + v12-Modelle
python scripts/eval_baselines.py              # nur ungelernte Referenzen
```

| Parameter | Wert |
|-----------|------|
| Testset A | Seeds 7000–7049 |
| Holdout B | Seeds 8000–8049 |
| Exit-Distanz | 35–45 |
| Episoden-Cap | **4000** (= Env-`maxSteps`) |
| Wiederholungen | 5 über unabhängige Politik-RNG-Seeds |
| Ausgabe | `logs/eval_results/baselines.json` |

### Warum kein eigenes Snippet

Das frühere Inline-Snippet an dieser Stelle war defekt und wurde am 05.08.2026 entfernt.
Es probierte nur `PPO`, `A2C` und `DQN` durch — `RecurrentPPO` fehlte, die v12-Modelle
hätten sich gar nicht laden lassen. Zudem rief es `model.predict()` ohne LSTM-Zustand
auf. Wer es zum Laufen bringt, misst ein Gedächtnismodell ohne Gedächtnis und erhält
systematisch zu niedrige Werte. `eval_baselines.py` führt den Zustand in `ModelPolicy`
korrekt mit.

### Immer beide Metriken berichten

Die Success Rate allein ist wertlos. Bei Cap 4000 sättigt sie: ein Zufallslauf mit
leichtem Kompassdrift (ε = 0,9) erreicht 92 %, ohne irgendetwas zu können. Die
Pfadeffizienz (BFS-Optimum / tatsächliche Schritte, gemittelt über die **erfolgreichen**
Episoden) trennt dort, wo die SR das nicht mehr tut.

Vorsicht bei der Interpretation: Die Effizienz wird nur über Erfolge gemittelt. Eine
Politik mit niedriger SR schafft bevorzugt die leichten Seeds und sieht dadurch
effizienter aus, als sie ist.

---

## Konfiguration

### Environment

| Parameter | Wert |
|-----------|------|
| `observationRadius` | 7 (→ 15×15 Grid) |
| `maxSteps` | 4.000 |
| `exitMinDistance` | 35 (Eval-Env) |
| `exitMaxDistance` | 45 (Eval-Env) |
| `forceGuaranteedPath` | `false` — redundant seit v11, die BFS-Exit-Platzierung garantiert Lösbarkeit |
| `n_eval_episodes` | 50 |
| Timesteps | 1.000.000 |

**Observation (Env v11, seit 06.07.2026): 229 Features** = Grid 15×15 (225) + HP +
exitDx + exitDy + step_frac.
Legacy-Modelle von vor dem 06.07.2026 haben 231 Dimensionen und brauchen
`StoneforgeWorldEnv(..., include_energy_inventory=True)`, sonst Shape-Mismatch.

### Algorithmenspezifisch

| Algo | Parameter | Wert | Begründung |
|------|-----------|------|------------|
| RecurrentPPO | `batch_size` | **8** (nicht 64) | 64 destabilisiert den Critic, siehe Fallstricke |
| RecurrentPPO | `ent_coef` | 0.05 | |
| RecurrentPPO | Device | **CPU** | MPS ist bei diesem Netz langsamer |
| DQN | `buffer_size` | 200.000 | |
| DQN | `exploration_fraction` | 0.50 | |

---

## Algorithmen

| Algo | Aktionsraum | Status | Ausgabeordner |
|------|-------------|--------|---------------|
| PPO | Discrete ✓ | aktiv | `models/ppo` |
| RecurrentPPO | Discrete ✓ | aktiv, liefert v12 | `models/ppo_lstm_curriculum_v12_s*` |
| DQN | Discrete ✓ | aktiv | `models/dqn` |
| A2C | Discrete ✓ | aktiv | `models/a2c` |
| SAC | nur Box ✗ | nicht nutzbar | — |
| TD3 | nur Box ✗ | nicht nutzbar | — |

---

## Environment

Kein Wrapper-Stack mehr: `StoneforgeWorldEnv` wird direkt verwendet.

Aktionsraum `Discrete(4)` — 0 = hoch, 1 = runter, 2 = links, 3 = rechts.

Mining, Bauen und Kampf sind seit Env v11 im C++-Binding entfernt; die Aktionen 4–8
werfen einen `RuntimeError`. Sie existieren nur noch im spielbaren Client. Die früheren
Wrapper `ExitPotentialFieldWrapper` und `ReducedActionEnv` liegen archiviert in `OLD/`.

---

## Dokumentationspflicht

**Nach jeder Änderung und nach jeder Messung** wird `CHANGELOG.md` aktualisiert. Kein
Experiment ohne Eintrag.

Neue Version anlegen nach dem Schema `v1.x — Datum — Kurzbeschreibung — Autor` und
dokumentieren:

- **Wer hat geändert?** Laurin oder Florian. Bei einer Gemeinschaftsarbeit muss die
  Zuordnung auch im Nachhinein rekonstruierbar sein.
- **Was wurde geändert?** Datei, Parameter, vorher → nachher
- **Warum?** beobachtetes Problem, Hypothese
- **Ergebnis?** 50-Seed-Eval mit success/50, mean_len, mean_return

Betrifft eine Änderung `src/` oder `game_config.json`, wird zusätzlich vermerkt, ob sich
die Env-Version ändert und welche Modellstände dadurch ungültig werden.

### Format für neue Ergebnisse

```markdown
| Algorithmus | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|-------------|---------|--------------|----------------------|---------------|-------|
| PPO         | X / 50  | X.X %        | XXXX.X               | −XX.XX        | TT.MM.JJJJ |
```

### Format für neue Änderungen

```markdown
#### Änderung X — Kurztitel
**Datei:** `pfad/zur/datei.py`
**Problem:** Was war falsch / was hat nicht funktioniert?
**Lösung:** Was wurde konkret geändert?

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `param`   | alt    | neu     | Warum      |
```

Diese Dokumentation ist Grundlage der Projektarbeit.

---

## Bekannte Fallstricke

### Messung und Auswertung

- **Eval-Cap unter 4000 verfälscht die SR massiv.** Gleiches Modell: 48 % bei Cap 600
  gegenüber 86 % bei Cap 4000. Immer mit Cap 4000 fahren, es kostet nur rund 21 s pro Eval.
- **Einzelne Eval-Snapshots sind keine Validierung.** Die Lerndynamik kann chaotisch sein
  und transiente Hochphasen zeigen. Hyperparameter-Entscheidungen fallen ausschließlich
  auf Basis ganzer Eval-Kurven.
- **Die Welt ist leichter, als die SR suggeriert.** Der Umwegfaktor beträgt 1,12
  (BFS-Distanz / Manhattan-Distanz, 100 Seeds, 05.08.2026). Der kürzeste Weg ist nur 12 %
  länger als die Luftlinie, es gibt fast nichts zu umrunden. Das ist die quantitative
  Ursache dafür, dass ein Vierzeilen-Kompass 92 % erreicht und Gedächtnis kaum Vorteil
  bringt. Bei jeder Aussage über die Schwierigkeit der Welt zählt diese Zahl, nicht die SR.

### Training

- **`batch_size=64` destabilisiert den LSTM-Critic.** EV bleibt bei etwa 0,1, die SR
  oszilliert zwischen 10 und 74 % ohne Konvergenz. Mit `batch_size=8` gibt es rund
  achtmal mehr Gradientenschritte pro Rollout, EV > 0,85 und einen stabilen Anstieg.
  Die frühere 64er-Validierung beruhte auf einem Einzel-Snapshot und wurde am 07.07.2026
  widerlegt (CHANGELOG v2026-07-07.4).
- **Zeitbasiertes Curriculum führt zum Reward-Kollaps.** Seit v1.1 leistungsbasiert.
- **MPS/GPU bringt nichts.** CPU ist bei diesem Netz immer schneller, gemessen am 06.07.2026.

### Environment und Weltgenerierung

- **PBRS läuft auf BFS-Distanz** (`simulation.cpp`, β = 2,5, γ = 0,999 = RL-γ) und ist
  damit policy-invariant. Ein Luftlinien- oder Manhattan-Potential wäre irreführend.
  Niemals darauf zurückbauen.
- **Zelluläre Glättung härtet die Welt nicht.** `enableCellularSmoothing: true` mit den
  hinterlegten Regeln (b = 5 / s = 4) senkt die Wanddichte von 0,238 auf 0,037 und den
  Umwegfaktor auf 1,001 — die Welt wird zur leeren Ebene. Jede Regelvariante, die den
  Umwegfaktor hebt, zerstört die Lösbarkeit (b = 2 / s = 1 → 3 von 50 lösbar). Ursache:
  `enableFloodFillValidation` und `enableMacroGraphPrecheck` sind aus, der Automat kennt
  keine Erreichbarkeit. Prüfen mit `scripts/probe_world_geometry.py --sweep`.
- **Biom-Schwellwerte sind hartkodiert** in `World::sampleBaseTile()` (`world.cpp`) und
  nicht über `game_config.json` tunebar. Die früheren cold/warm/moss-Keys waren tot und
  wurden entfernt.
- **WorldGen-Config ist prozess-global** (C++). `StoneforgeWorldEnv` stempelt sie bei
  jedem `reset()` neu. Bei direkter Nutzung des C++-Bindings ohne Env-Wrapper daran denken.

---

## Offene Punkte

- **`scripts/eval_hard_world.py` ist defekt.** Das Skript setzt tote Config-Keys, die
  Welt wird dadurch leerer statt härter. Vor jeder Weiterverwendung reparieren.
- **Ordnerkonvention prüfen.** Die Algorithmentabelle nennt `models/ppo`, `models/dqn`
  und `models/a2c` als Ausgabeziele von `train.py`; im Verzeichnisbaum tauchen diese
  Ordner nicht auf. Vor einer Berufung darauf im Repository verifizieren.
- **Zielkriterium noch nicht erreicht.** Siehe nächster Abschnitt.

---

## Zielkriterium (Projektarbeit)

| Kriterium | Ziel | Stand v12 | Status |
|-----------|------|-----------|--------|
| Testset A (7000–7049) | ≥ 70 % SR | 64,6 % ± 10,1 | nicht erreicht |
| Testset B (8000–8049) | ≥ 60 % SR | 68,8 % ± 15,5 | erreicht |
| Läufe pro Konfiguration | ≥ 3 | 7 | erfüllt |

Pro Konfiguration sind Mittelwert und Standardabweichung anzugeben.

---

## Schreibstil

Gilt für alle Texte der Projektarbeit — Exposé, Report, Paper, Changelog-Fließtext.
Zielbild ist eine deutsche technische Abschlussarbeit im Informatikkontext, die
erkennbar von einem Menschen stammt und ihre Ergebnisse schonungslos benennt.

**Der Stil gilt für beide Teile identisch.** Eine gemeinsam abgegebene Arbeit, in der
zwei Handschriften erkennbar auseinanderlaufen, wird als solche gelesen und bewertet.

### Zwei Autoren, eine Stimme

Die unpersönliche Grundhaltung ist hier kein Selbstzweck, sondern der Mechanismus, der
beide Teile zusammenhält: Wo kein „ich" auftritt, entsteht auch kein Bruch zwischen den
Autoren.

- **Keine Ich-Formen, auch nicht implizit.** Kein „in meinem Teil", kein „wie mein
  Projektpartner zeigt". Stattdessen der Verweis auf das Kapitel: „wie in Kapitel 5
  gezeigt wird".
- **Begriffe werden einmal festgelegt und durchgehalten.** Umwegfaktor, Wanddichte,
  Lösbarkeit, Success Rate, Pfadeffizienz, Env-Version — beide Teile verwenden dieselben
  Bezeichnungen und dieselben Einheiten.
- **Abkürzungen werden nur einmal eingeführt**, bei der ersten Nennung im gesamten
  Dokument, nicht erneut zu Beginn des zweiten Teils.
- **Keine Doppelerklärungen an der Schnittstelle.** Der RL-Teil erklärt die
  Weltgenerierung nicht noch einmal, sondern verweist auf sie. Umgekehrt genauso.
- **Vor der Abgabe liest jede Person den Teil der anderen** und gleicht Terminologie,
  Satzrhythmus und Verweisformat ab.

### Grundhaltung: unpersönlich

Niemals „ich", „wir", „man" oder „unser". Handelndes Subjekt ist die Arbeit, das
Verfahren, das Modell oder das Skript. Passiv und Modalverb sind der Normalfall:

- Vorgehen → „Dazu wird … verwendet.", „Die Ergebnisse werden … bewertet."
- Absicht, Abgrenzung → „… soll in dieser Arbeit ausgeklammert werden."
- Randbedingung → „Dabei muss berücksichtigt werden, dass …"
- Ausblick und Spekulation → ausschließlich Konjunktiv II: „könnte", „könnten", „müsste"

Diese Regel ersetzt die frühere Vorgabe „Aktiv statt Passiv". Sie war mit dem Stil der
Bachelorarbeit unvereinbar und wird nicht mehr angewendet.

### Rhythmus: Satzlängen variieren

Der Durchschnitt liegt bei 20 bis 22 Wörtern pro Satz, ein Hauptsatz plus höchstens ein
Nebensatz. Gegen die Monotonie hilft ein kurzer, harter Satz als Abschluss eines
Absatzes:

> Das LSTM erkauft seine höhere Erfolgsquote durch mehr Herumlaufen, nicht durch bessere
> Wege. Die Pfadeffizienz rettet das Ergebnis nicht.

Absätze umfassen zwei bis fünf Sätze und behandeln genau einen Gedanken.

### Ehrlichkeit: Ecken und Kanten stehen lassen

Negative Ergebnisse, widerlegte Annahmen und versagende Metriken werden hart und
unmissverständlich benannt, ohne Weichzeichner. Kein „konnte tendenziell nicht
vollständig bestätigt werden", sondern:

> Der ε=0,3-Kompass ist rund dreimal wegeffizienter als das trainierte Modell.

Zurückhaltung gilt für Lob, nicht für Kritik. Erfolge werden gedämpft formuliert
(„liefert brauchbare Grundlagen"), Misserfolge klar.

### Begründungspflicht

Jede Entscheidung, jeder Ausschluss und jede Parameterwahl wird begründet — mit „weil",
„da", „daher" oder „dadurch". Eine Auswahl ohne Grund existiert im Text nicht.

### Querverweise

Jeder Sachabsatz wird an eine andere Stelle angebunden. Abwechselnd verwenden:
`(siehe Kapitel 4.2)`, `(siehe Anhang C)`, `(siehe Abbildung 3.1)`, `(siehe Tabelle F.1)`
sowie inline „Wie in der Problemanalyse 3.3.2 beschrieben, …".

### Keine Datei- und Ordnerpfade im Fließtext

Verwiesen wird ausschließlich auf Bestandteile der Arbeit, niemals auf das Repository.
Ordnernamen, Dateipfade, Skriptnamen und Logdateien haben im Text der Projektarbeit
nichts zu suchen — auch nicht als Quellenangabe in Klammern und auch nicht in
Abbildungsunterschriften.

| Falsch | Richtig |
|--------|---------|
| „Quelle: `logs/eval_results/baselines.json`" | „(siehe Anhang D)" |
| „gemessen mit `scripts/probe_world_geometry.py`" | „ermittelt über die Analyse der Weltgeometrie (siehe Kapitel 5.2)" |
| „hartkodiert in `world.cpp`" | „im Quellcode der Weltgenerierung festgelegt" |
| „`train_curriculum.py` erzeugt die Ergebnisse" | „Die Ergebnisse entstehen im leistungsbasierten Curriculum (siehe Kapitel 7.1)" |

Der Grund ist inhaltlich, nicht kosmetisch: Ein Pfad belegt nichts. Er ist für Lesende
ohne Repository-Zugriff nicht nachvollziehbar und veraltet mit dem nächsten Refactoring.
Nachvollziehbarkeit entsteht über Tabellen, Abbildungen und Anhänge im Dokument selbst.

Rohdaten, Konfigurationen und Quellcodeauszüge gehören daher in den Anhang und werden von
dort aus zitiert. Wo ein Werkzeug benannt werden muss, wird es fachlich beschrieben, nicht
über seinen Dateinamen.

Ausgenommen sind ausschließlich Code-Listings und der Anhang selbst, in denen ein
Dateiname als Überschrift des Listings zulässig ist.

Diese Regel gilt nur für die Texte der Projektarbeit. Innerhalb dieser Datei und im
Changelog sind Pfade weiterhin erwünscht, dort sind sie der Zweck.

### Abgrenzungsfigur

Angrenzende Themen werden benannt, ausgeschlossen, begründet und vertagt:

> Die Untersuchung alternativer Weltgeneratoren soll in dieser Arbeit ausgeklammert
> werden, weil der damit verbundene Aufwand eine eigene Arbeit rechtfertigt.

### Signposting

Jeder Abschnitt wird eingeleitet und der nächste angekündigt: „Im Folgenden wird das
Eval-Protokoll näher beschrieben:", „Nun folgt die Betrachtung der Ergebnisse.",
„Dieses Skript soll im folgenden Abschnitt vorgestellt werden."

### Abbildungen — Pflicht, nicht Kür

Reiner Fließtext über Seiten hinweg ist ein Mangel. Wo etwas erklärt, verglichen oder
gezeigt werden soll, wird eine Abbildung eingesetzt. Existiert keine passende Vorlage,
wird selbst eine erstellt.

Faustregel: **kein Kapitel ohne mindestens eine Abbildung**, kein Ergebnisabschnitt ohne
Diagramm.

**Wann eine Abbildung entsteht:**

- **Ergebnisse** — immer. Zahlenreihen im Fließtext sind unlesbar. Balken für Vergleiche,
  Linien für Verläufe über die Trainingszeit, Fehlerbalken für Streuung über die Läufe.
- **Verfahren und Abläufe** — Überblicksdiagramm zu Beginn der Methodik, das die Schritte
  zeigt, bevor sie einzeln beschrieben werden.
- **Architektur** — Aufbau der Umgebung, Datenfluss zwischen C++-Kern und Python-Seite,
  Aufbau des Beobachtungsvektors.
- **Beispiele** — ein Weltausschnitt sagt mehr über die Weltgenerierung als ein Absatz.
  Gelaufene Agentenpfade neben dem BFS-Optimum machen den Effizienzbegriff sofort
  verständlich.
- **Gegenüberstellungen** — zwei Zustände nebeneinander, etwa die Welt mit und ohne
  zelluläre Glättung.

**Naheliegende Abbildungen in diesem Projekt:**

| Inhalt | Form |
|--------|------|
| Erfolgsquote der Modelle gegen die ungelernten Referenzen | Balkendiagramm mit Standardabweichung |
| Pfadeffizienz derselben Verfahren | Balkendiagramm, gleiche Achsenordnung wie oben |
| Verlauf der Erfolgsquote über die Trainingsschritte | Liniendiagramm, alle sieben Läufe plus Mittelwert |
| Vier Curriculum-Phasen mit ihren Distanzbereichen | Ablaufdiagramm |
| Wanddichte und Umwegfaktor über die Regelvarianten | Liniendiagramm |
| Welt mit und ohne zelluläre Glättung | zwei Kartenausschnitte nebeneinander |
| Gelaufener Pfad gegen kürzesten Weg | annotierter Kartenausschnitt |
| Aufbau des 15×15-Beobachtungsfensters | Schemazeichnung |

**Regeln für den Umgang:**

- Jede Abbildung wird im Text **angekündigt und ausgewertet**. Eine Abbildung, die
  unkommentiert dasteht, ist eine verpasste Aussage.
- Diagramme entstehen aus den tatsächlichen Messdaten, niemals aus geschätzten oder
  nachgezeichneten Werten.
- Über alle Diagramme hinweg gelten dieselben Farben, Achsenbeschriftungen und
  Einheiten. Ein Verfahren behält seine Farbe im gesamten Dokument.
- Achsen tragen Beschriftung und Einheit, Skalen beginnen bei null, sofern das nicht
  ausdrücklich anders begründet wird.
- Beide Teile der Arbeit verwenden denselben Abbildungsstil, siehe
  [Zwei Autoren, eine Stimme](#zwei-autoren-eine-stimme).

**Bildunterschriften: maximal ein Satz.**

Die Unterschrift benennt, was zu sehen ist, und nennt die Herkunft. Kein zweiter Satz,
keine Interpretation, keine Wiederholung des Fließtextes, kein Dateipfad. Die Auswertung
gehört in den Text, nicht unter das Bild.

```
Abbildung 8.3: Evaluierung - Erfolgsquote der drei Verfahren auf Testset A, eigene Darstellung
Abbildung 2.1: Grundlagen - Aufbau eines Transformers, Darstellung aus „Attention Is All You Need" [38]
```

Falsch wäre: *„Abbildung 8.3: Erfolgsquote der drei Verfahren. Deutlich zu erkennen ist,
dass das LSTM besser abschneidet als die Kontrollgruppe. Die Streuung ist erheblich."*
Der zweite und dritte Satz gehören in den Fließtext.

### Sprachliche Konventionen

- „beziehungsweise" statt „bzw.", „zum Beispiel" statt „z. B."
- Abkürzungen bei Erstnennung als Vollform mit Klammerakronym einführen, danach nur das
  Akronym: „Success Rate (SR)", „Proximal Policy Optimization (PPO)"
- Gendergerechte Partizipien: „die Programmierenden", nicht „die Programmierer"
- Deutsches Dezimalkomma (0,8 · 65,7 %), englische Fachtermini unübersetzt (Policy,
  Rollout, Curriculum, Baseline)
- Ordnende Adverbien: „Zunächst", „Im nächsten Schritt", „Anschließend", „Zudem",
  „Des Weiteren", „Außerdem", „Daher", „Ferner", „Allerdings", „Jedoch", „Zum Schluss"

### Verboten

- „somit", „folglich", „mithin"
- KI-Floskeln: „Es ist wichtig zu beachten, dass", „Zusammenfassend lässt sich sagen",
  „darüber hinaus", „grundlegend", „umfassend", „signifikant", „entscheidend".
  Ausnahme: „signifikant" ist als statistischer Fachterm zulässig, wenn ein
  Ergebnis tatsächlich auf einen Signifikanztest (p-Wert, Konfidenzintervall)
  gestützt wird, etwa „der Unterschied ist signifikant (p = 0,016)". Als
  bloßes Verstärkungswort („ein signifikanter Fortschritt") bleibt es verboten.
- Gedankenstriche als Einschub; stattdessen Komma, Klammer oder ein zweiter Satz
- Rhetorische Fragen, Leseransprache, Emojis, Superlative, Marketingsprache
- Absätze mit mehr als fünf Sätzen
- Zahlenangaben ohne Beleg im Dokument
- **Datei- und Ordnerpfade als Quellenangabe.** Kein `models/`, kein `scripts/train.py`,
  kein `logs/eval_results/baselines.json` im Fließtext. Stattdessen auf Kapitel, Tabelle,
  Abbildung oder Anhang verweisen

### Selbstprüfung vor der Abgabe

```
[ ] Kein „ich", „wir", „man"
[ ] Passiv plus Modalverb überwiegt, Ausblick im Konjunktiv II
[ ] Satzlängen variieren, mindestens ein kurzer Fazitsatz pro Abschnitt
[ ] Jeder Sachabsatz trägt einen Verweis auf Kapitel, Tabelle, Abbildung oder Anhang
[ ] Kein einziger Datei- oder Ordnerpfad im Fließtext
[ ] Jedes Kapitel enthält mindestens eine Abbildung, jeder Ergebnisabschnitt ein Diagramm
[ ] Jede Abbildung ist im Text angekündigt und ausgewertet
[ ] Jede Bildunterschrift ist genau ein Satz lang
[ ] Jede Entscheidung ist begründet
[ ] Negative Befunde sind hart benannt, nicht geglättet
[ ] Kein Absatz länger als fünf Sätze
[ ] Keine Gedankenstriche, keine KI-Floskeln
```