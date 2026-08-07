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
│   ├── ppo_phase4/            # HISTORISCH: MLP mit BFS-Orakel, Env vor v11.
│   │                          #   100% det NUR unter dem damaligen Kurzdistanz-Protokoll;
│   │                          #   unter dem Standard-Eval (exit 35-45) 32% stoch / 0% det.
│   ├── ppo_phase5/            # HISTORISCH: 72% unter altem Protokoll, nicht vergleichbar
│   ├── ppo_lstm_curriculum_v12_s1..s7/  # ← AKTUELLE ENDERGEBNISSE (n=7)
│   │                          #   A 65,7% ± 12,4 stoch / B 66,9% ± 12,8 stoch
│   └── ...                    # Weitere Experimente

> ⚠️ Modellwerte aus verschiedenen Env-Versionen sind NICHT vergleichbar.
> Berichtsfähig ist ausschließlich der v12-Stand (n=7) unter dem Standard-Eval.
> Ungelernte Referenz (Quelle: `logs/eval_results/baselines.json`, Testset A):
> Random **5,2 % ± 2,7** (Effizienz 0,021) · Kompass-Zufallslauf ε=0,9 **92,0 % ± 5,1**
> (Effizienz 0,047). Die früher hier notierten „8 % / 89 %" waren veraltet und
> widersprachen der Projektdokumentation — korrigiert 05.08.2026 gegen die Rohdaten.
>
> ⚠️ **Die Pfadeffizienz rettet das Ergebnis NICHT.** v12 liegt bei 0,039–0,061 und damit
> auf dem Niveau des ε=0,9-Zufallslaufs (0,040); der ε=0,3-Kompass erreicht 0,158 und ist
> damit rund dreimal wegeffizienter als das trainierte Modell. Das LSTM erkauft seine
> höhere SR durch mehr Herumlaufen, nicht durch bessere Wege.
> (Quelle: `logs/eval_results/baselines_and_models.json`.)
├── OLD/                       # Veralteter Code (nicht gelöscht, nur archiviert)
│   └── scripts/               # Alter CLI-Launcher, Shell-Skripte
├── python/                    # Importierbare RL-Bibliothek (PYTHONPATH)
│   └── stoneforge_env.py      # Gym-Environment + Wrapper (importiert via PYTHONPATH)
├── scripts/                   # Ausführbare Skripte
│   ├── launcher_gui.py        # ← HAUPT-EINSTIEGSPUNKT (GUI)
│   ├── train_curriculum.py    # ← ERZEUGT DIE BERICHTSFÄHIGEN ERGEBNISSE (v12, n=7)
│   │                          #   --algo {rppo,ppo}: LSTM bzw. MLP-Kontrollgruppe (F2)
│   ├── train.py               # Einzelphasen-Training (PPO / DQN / A2C) — NICHT die
│   │                          #   Quelle der v12-Zahlen, dafür train_curriculum.py
│   ├── eval_baselines.py      # ← KANONISCHES EVAL-PROTOKOLL (SR + Pfadeffizienz)
│   ├── probe_world_geometry.py # Weltgeometrie: Umwegfaktor, Wanddichte, Lösbarkeit
│   ├── smoke_test_algo_switch.py # Schnelltest des --algo-Pfads (rppo/ppo)
│   ├── watch_agent.py         # Grafische Agent-Visualisierung
│   ├── analyze_agent.py       # Verhaltensanalyse
│   ├── eval_hard_world.py     # ⚠️ DEFEKT: setzt tote Config-Keys, Welt wird leerer
│   │                          #   statt härter. Vor Weiterverwendung reparieren.
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

> ⚠️ Nach jeder Änderung am **C++-Code** ist ein Rebuild nötig.
> **Nicht** aber nach Änderungen an `game_config.json`: Die Datei wird zur Laufzeit gelesen
> (`StoneforgeCoreEnv`-Konstruktor, `src/python/py_module.cpp:44-48`; im Client/Headless über
> `loadGameConfigFile()`). CMake kopiert keine Assets. Ein JSON-Patch wirkt für jedes danach
> **neu konstruierte** Env sofort — bestehende Env-Instanzen behalten ihre Werte.
> (Nachgemessen 05.08.2026, siehe CHANGELOG v2026-08-05.1.)

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

## Standardisierter Eval — kanonisch: `scripts/eval_baselines.py`

**Es gibt genau EIN gültiges Protokoll. Nicht selbst nachbauen.**

```bash
python scripts/eval_baselines.py --models     # Baselines + v12-Modelle
python scripts/eval_baselines.py              # nur ungelernte Referenzen
```

Protokoll: Testset A `7000–7049`, Holdout B `8000–8049`, `exit 35–45`, **Cap 4000**,
5 Wiederholungen über unabhängige Politik-RNG-Seeds. Ergebnis landet in
`logs/eval_results/baselines.json`.

> ⚠️ **Das frühere Inline-Snippet an dieser Stelle war defekt** (entfernt 05.08.2026):
> es probierte nur `PPO`/`A2C`/`DQN` durch — `RecurrentPPO` fehlte, eure v12-Modelle
> hätten sich gar nicht laden lassen. Zudem rief es `model.predict()` **ohne LSTM-Zustand**
> auf; wer es zum Laufen bringt, misst ein Gedächtnismodell ohne Gedächtnis und erhält
> systematisch zu niedrige Werte. `eval_baselines.py` führt den Zustand in `ModelPolicy`
> korrekt mit.

**Immer beide Metriken berichten, SR allein ist wertlos.** Bei Cap 4000 sättigt die
Erfolgsquote: ein Zufallslauf mit leichtem Kompassdrift (ε=0,9) erreicht 92 % — ohne
irgendetwas zu können. Die Pfadeffizienz (BFS-Optimum / tatsächliche Schritte, gemittelt
über die *erfolgreichen* Episoden) trennt dort, wo die SR das nicht mehr tut.
Vorsicht bei der Interpretation: Effizienz wird nur über Erfolge gemittelt, eine Politik
mit niedriger SR schafft bevorzugt die leichten Seeds und sieht dadurch effizienter aus.

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

Curriculum: leistungsbasiert (4 Phasen: 5–12 / 12–25 / 25–45 / Greedy Fine-Tune) über
`scripts/train_curriculum.py`. Seit 05.08.2026 mit `--algo {rppo,ppo}`:
`rppo` = RecurrentPPO (LSTM), `ppo` = gedächtnislose MLP-Kontrollgruppe für F2 auf
**identischem** Curriculum. Unterschiede nur architekturspezifisch (`policy`, `batch_size`,
`net_arch`), alles übrige geteilt. Durchsatz gemessen: MLP 5.989 Steps/s, LSTM 104 Steps/s —
die „8 Stunden pro Lauf" aus dem Anhang gelten **nur** für das LSTM.
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
- **Rebuild vergessen** nach Änderungen an C++ → Crash oder falsche Ergebnisse.
  (`game_config.json` braucht **keinen** Rebuild, siehe Build-Abschnitt.)
- **Die Umgebung hat Umwegfaktor 1,12** (BFS-Distanz / Manhattan-Distanz, 100 Seeds,
  05.08.2026). Der kürzeste Weg ist nur 12 % länger als die Luftlinie — es gibt fast nichts
  zu umrunden. Das ist die quantitative Ursache dafür, dass ein Vierzeilen-Kompass 92 %
  erreicht und Gedächtnis kaum Vorteil bringt. Bei jeder Aussage über „Schwierigkeit" der
  Welt diese Zahl mitdenken, nicht die SR.
- **Zelluläre Glättung härtet die Welt NICHT.** `enableCellularSmoothing: true` mit den
  hinterlegten Regeln (b=5/s=4) senkt die Wanddichte von 0,238 auf 0,037 und den Umwegfaktor
  auf 1,001 — die Welt wird zur leeren Ebene. Jede Regelvariante, die den Umwegfaktor hebt,
  zerstört die Lösbarkeit (b=2/s=1 → 3/50 lösbar). Ursache: `enableFloodFillValidation` und
  `enableMacroGraphPrecheck` sind aus, der Automat kennt keine Erreichbarkeit.
  Prüfen mit `scripts/probe_world_geometry.py --sweep`.
- **Modellpfade**: Alle Modelle liegen jetzt in `models/`, nicht mehr in `best_models_*/`.

---

## Zielkriterium (Projektarbeit)

- **Testset A** (Seeds 7000–7049): ≥ 70 % Success Rate
- **Testset B** (Holdout, Seeds 8000–8049): ≥ 60 % Success Rate
- Pro Konfiguration: mind. 3 Trainingsläufe → Mittelwert + Standardabweichung angeben

## Schreibstil
Du bist ein erfahrener Lektor und wissenschaftlicher Betreuer für Informatik-Masterarbeiten. Deine Aufgabe ist es, den folgenden Textabschnitt meiner Projektarbeit so zu überarbeiten, dass er natürlicher, menschlicher und weniger nach einem typischen, generierten KI-Text klingt.

Bitte wende bei der Überarbeitung die folgenden Regeln strikt an:
Satzlängen variieren (Burstiness): Brich die typische KI-Monotonie auf. Wechsle gezielt zwischen sehr kurzen, prägnanten Sätzen (gerne auch mal als hartes Fazit eines Absatzes) und längeren, detaillierten Erklärungen.
Aktiv statt Passiv: Ersetze unpersönliche, bürokratische Passivkonstruktionen ("Es wurde trainiert...", "Es konnte gezeigt werden...") durch aktive Formulierungen ("Wir trainieren...", "Die Daten belegen...", "Die Auswertung deckt auf...").
KI-Floskeln restlos streichen: Entferne typische Füllwörter und Phrasen wie "Es ist wichtig zu beachten, dass...", "Zusammenfassend lässt sich sagen...", "darüber hinaus", "grundlegend", "umfassend", "signifikant" oder "entscheidend". Sei direkter.
Wissenschaftliche Ecken und Kanten: Der Text soll ehrlich, analytisch und auf den Punkt klingen. Wenn ein Ergebnis negativ ist, eine Annahme falsch war oder eine Metrik versagt hat, benenne das hart und klar. Keine künstliche Glättung von Problemen.
Tonalität: Der Text muss weiterhin das formale Niveau einer Informatik-Masterarbeit erfüllen, soll aber die Handschrift eines menschlichen Autors tragen, der seine Ergebnisse selbstbewusst und messerscharf argumentiert.
Vermeide folgendes in den Texten: Bindestriche "-"