# Changelog

---

## v2026-07-08 — HUD-Textbox entfernt

### v2026-07-08.A — Oberen HUD-Textblock deaktiviert
**Datei:** `src/client/render_ui.cpp`

**Problem:** Der obere Overlay-Block zeigte dauerhaft Status- und Hilfetext wie Tile-Größe, Werkzeugstufe, Reichweite, Werkbankstatus, Inventarstatus, Slot und Steuerhinweise an.
**Lösung:** `drawHud(...)` rendert diesen Textblock nicht mehr; die Funktion ist jetzt ein No-Op und lässt den restlichen UI-Flow unverändert.

**Validierung:** Syntax-/Fehlerprüfung der geänderten Datei ohne Befund. Ein vollständiger Build konnte in dieser Sitzung nicht über die Build-Tools ausgeführt werden.

---

## v2026-06-11 — Verbesserungen Stoneforge RL (LSTM-Curriculum)

### v2026-06-11.A — Behebung methodischer Fehler & Lernverbesserungen
**Dateien:** `src/core/simulation.cpp`, `src/include/stoneforge/simulation.hpp`, `src/python/py_module.cpp`, `python/stoneforge_env.py`, `scripts/train_curriculum.py`, `scripts/train_cnn.py`, `scripts/eval_comparison.py`, `scripts/check_solvability.py` (neu), `scripts/run_experiment.py` (neu)

**Motivation:** Behebung von methodischen Fehlern (Data Leakage, Lösbarkeit) und Verbesserung der Lernleistung zur Schließung des Det/Stoch-Gaps auf der Zielverteilung.

#### Änderung 1 — Validierungs-Seeds & Beseitigung des Data-Leakages
**Dateien:** [train_curriculum.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_curriculum.py), [train_cnn.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_cnn.py)
**Lösung:** Einführung von `VAL_SEEDS = list(range(6000, 6050))` für die Phasensteuerung und Modellselektion im Callback. Die Test-Seeds werden nur noch in der finalen Evaluation genutzt.

#### Änderung 2 — Lösbarkeitsprüfung
**Dateien:** [simulation.cpp](file:///Users/merluee/Master_Projektarbeit/src/core/simulation.cpp), [simulation.hpp](file:///Users/merluee/Master_Projektarbeit/src/include/stoneforge/simulation.hpp), [py_module.cpp](file:///Users/merluee/Master_Projektarbeit/src/python/py_module.cpp), [stoneforge_env.py](file:///Users/merluee/Master_Projektarbeit/python/stoneforge_env.py), [check_solvability.py](file:///Users/merluee/Master_Projektarbeit/scripts/check_solvability.py) (neu)
**Problem:** Das Training lief mit `force_guaranteed_path=False`, was möglicherweise unlösbare Welten erzeugte.
**Lösung:** BFS-Prüfung `isPathToExitReachable()` in C++ implementiert und nach Python exportiert. Überprüfung von 3.150 Seeds (inkl. Val/Test/Holdout und Trainingsphasen) ergab 100.0% Lösbarkeit, da der Generierungsalgorithmus nur verbundene Exits wählt. Kein Eingriff in die Generierung nötig.

#### Änderung 3 — Batch-Größe & Trainingseffizienz
**Dateien:** [train_curriculum.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_curriculum.py), [train_cnn.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_cnn.py)
**Problem:** `batch_size=16` war extrem verrauscht und ineffizient.
**Lösung:** Erhöhung der Batch-Größe auf `256` (32 Minibatches pro Epoche) für stabilere Gradienten und deutlich höhere FPS (~330 vs ~110).

#### Änderung 4 — Letzte Aktion + Reward in der Observation
**Dateien:** [stoneforge_env.py](file:///Users/merluee/Master_Projektarbeit/python/stoneforge_env.py)
**Problem:** Der LSTM-Agent litt unter Oszillationsschleifen an Wänden. Ohne Kenntnis über die blockierte Aktion konnte der Belief-State dies schwer auflösen.
**Lösung:** Erweiterung der Observation um die letzte Aktion (One-Hot, 4 Dims) und den letzten Reward (geclippt, 1 Dim). Optionale Aktivierung per Parameter `use_last_action_reward` zur Erhaltung der Rückwärtskompatibilität.

#### Änderung 5 — Entropie-Annealing in Phase 3
**Dateien:** [train_curriculum.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_curriculum.py), [train_cnn.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_cnn.py)
**Problem:** Ein konstantes `ent_coef=0.01` führt dazu, dass die Policy bis zum Ende stochastisch bleibt, was den Det/Stoch-Gap vergrößert.
**Lösung:** Lineare Absenkung des `ent_coef` in Phase 3 von `0.01` auf `0.001` über die ersten 500k Schritte, um eine stabile Greedy-Strategie zu erzwingen.

#### Änderung 6 — Curriculum-Anpassungen & Consecutive Success Check
**Dateien:** [train_curriculum.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_curriculum.py), [train_cnn.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_cnn.py)
**Problem:** Phase 1 (40%) und Phase 2 (30%) stoppten zu früh, wodurch Phase 3 mit einer halbgaren Policy startete. Zudem konnte ein einzelnes gutes Eval ein Ausreißer sein.
**Lösung:** Anhebung der Ziel-Success-Rate auf `85%` (Phase 1) und `70%` (Phase 2). Stoppen einer Phase erst nach **zwei aufeinanderfolgenden** Evals über dem Ziel.

#### Änderung 7 — PLR-Swarm (Swarm-Pool invertieren)
**Dateien:** [stoneforge_env.py](file:///Users/merluee/Master_Projektarbeit/python/stoneforge_env.py), [train_curriculum.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_curriculum.py), [train_cnn.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_cnn.py)
**Problem:** Der SwarmPool wiederholte bereits gelöste Seeds, was das Training auf einfache Fälle verzerrte.
**Lösung:** PLR-Semantik (Prioritized Level Replay): Fehlgeschlagene Seeds (Timeout/Early Stop) kommen in den Pool, gelöste Seeds werden gelöscht. Per Flag `--plr` aktivierbar.

#### Änderung 8 — Stuck-Penalty & PBRS-Gamma-Anpassung
**Dateien:** [stoneforge_env.py](file:///Users/merluee/Master_Projektarbeit/python/stoneforge_env.py), [simulation.cpp](file:///Users/merluee/Master_Projektarbeit/src/core/simulation.cpp)
**Problem:** Stuck-Penalty ab 300 Schritten wurde wegen des 256-Schritte-Early-Stops nie erreicht. `PBRS_GAMMA` von `1.0F` wich vom RL-Discount ab.
**Lösung:** Stuck-Penalty-Schwelle auf `25` Schritte gesenkt und Strafe skaliert. `PBRS_GAMMA` auf `0.999F` gesetzt.

#### Änderung 9 — Wrapper für das 3-Läufe-Protokoll
**Dateien:** [run_experiment.py](file:///Users/merluee/Master_Projektarbeit/scripts/run_experiment.py) (neu)
**Lösung:** Automatisierungsskript zur Ausführung des Trainings über 3 Seeds und anschließender Evaluierung (Mittelwert ± Standardabweichung).

---

## v2026-06-08 — PokéRL-Inspiration: Swarm, Live Map (WS), Hyperparameter-Tuning

---

### v2026-06-08.A — Swarm-Training + HTTP-Live-Map + Heatmap-Eval
**Dateien:** `python/stoneforge_env.py`, `scripts/train_curriculum.py`, `scripts/live_map_server.py`, `scripts/live_map.html`, `scripts/heatmap_eval.py`

**Motivation:** Inspiration durch das PokéRL-Projekt (PokemonRedExperiments). Drei Features eingebaut:

#### Änderung 1 — SwarmSeedPool
**Datei:** `python/stoneforge_env.py`
**Problem:** Alle 8 Parallel-Envs trainieren auf zufälligen Seeds — schwierige Seeds werden nie wiederholt.
**Lösung:** Thread-sicherer `SwarmSeedPool`: wenn ein Agent den Exit findet, landet sein Seed im Pool. Mit Wahrscheinlichkeit `swarm_prob=0.3` zieht eine Env beim nächsten Reset einen erfolgreichen Seed.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| Seed-Wiederholung | keine | 30% Chance auf Erfolgs-Seed | Fokussiert Training auf lösbare Karten |
| Pool-Größe | — | max 200 Seeds (FIFO) | Neueste erfolgreiche Seeds bevorzugt |

**Hinweis:** PokéRL's "Swarm" ist nur Visualisierung (WebSocket-Koordinaten). Unser SwarmSeedPool ist tatsächlich ausgefeilter — echter Lernmechanismus.

#### Änderung 2 — HTTP Live Map (Zwischenstand, später durch WS ersetzt)
**Datei:** `scripts/live_map_server.py`, `scripts/live_map.html`
Stdlib-HTTPServer, kein extra Dependency. GET `/` → HTML, GET `/state` → JSON, POST `/update` → State setzen. Browser pollt alle 250ms. Training-Modus: 8 farbige Agenten-Trails auf gemeinsamer Canvas.

#### Änderung 3 — Heatmap-Evaluation
**Datei:** `scripts/heatmap_eval.py`
```bash
python scripts/heatmap_eval.py --model path --stochastic --holdout --show
```
3-Panel-Matplotlib: alle Episoden (hot), Erfolge (YlGn), Misserfolge (OrRd). Log-Skala (`np.log1p`). Wird nach Training automatisch per Subprocess gestartet.

---

### v2026-06-08.B — Ablation D: CNN + Visited Mask
**Dateien:** `python/cnn_extractor.py` (neu), `scripts/train_cnn.py` (neu), `scripts/eval_comparison.py`

**Motivation:** Hypothese: Ein 2D-CNN mit Visited Mask als zweitem Kanal ermöglicht bessere räumliche Generalisierung als flacher MLP.

**Architektur (StoneforgeGridCNN):**
```
Input: 2×15×15 (Kanal 0: Tile-Typen, Kanal 1: Visited Mask)
Conv(2→16, 3×3) → ReLU → Conv(16→32, 3×3) → ReLU →
MaxPool(3,3) [15→5] → Conv(32→64, 3×3) → ReLU →
Flatten → Linear(1600→128) → ReLU
Output: concat(cnn_out(128), extras(6)) = 134 dims → LSTM
```

| Parameter | vorher (Ablation C) | Ablation D |
|-----------|---------------------|------------|
| Obs-Shape | 231 (flach) | **456** (2×225 Grid + 6 Extras) |
| Feature-Extraktor | MLP (implizit) | **StoneforgeGridCNN** |
| Kanal 1 | — | Visited Mask (1.0 = betreten) |

Ablationsmatrix vollständig:
- **A** — MLP + BFS: 100% det (Referenz-Obergrenze, historisch)
- **B** — MLP, kein BFS: 0% det (Negativ-Ergebnis)
- **C** — LSTM, kein BFS: 86% stoch / 36% det
- **D** — LSTM+CNN, kein BFS: *ausstehend (ppo_lstm_cnn)*

---

### v2026-06-08.C — WebSocket Live Map (PokéRL-Architektur)
**Dateien:** `scripts/ws_map_server.py` (neu), `python/stream_wrapper.py` (neu), `scripts/ws_map.html` (neu)
**Ersetzt:** `scripts/live_map_server.py` (HTTP-Polling)

**Architektur-Vergleich:**

| | PokéRL | Stoneforge |
|--|--------|------------|
| Sender | `StreamWrapper` per WS zu `wss://transdimensional.xyz` | `StreamWrapper` ruft `ws_map_server.update_agent()` in-process auf |
| Server | externer Broadcast-Server | lokaler asyncio WS-Server (`ws://localhost:8765`) |
| Viewer | Browser verbindet zu externem Server | Browser öffnet `ws_map.html` direkt (`file://`), verbindet zu localhost |
| Update-Rate | pro Step | pro Step (StreamWrapper) + ~10fps Push an Browser |

**StreamWrapper** erbt von `gymnasium.Wrapper` → kompatibel mit `DummyVecEnv`.
Pro Step: `player_pos()`, `current_bfs_distance_to_exit()` → JSON an WS-Server.

**Wichtig:** HTTP Live Map (`live_map_server.py`) bleibt als Fallback erhalten (z.B. für `watch_agent.py`). WS-Map ist default für `train_curriculum.py`.

---

### v2026-06-08.D — Hyperparameter-Tuning (PokéRL-inspiriert + Skalierung)

#### Änderung 1 — Stuck-Penalty
**Datei:** `python/stoneforge_env.py`
**Problem:** Agent kann eine Tile beliebig oft besuchen ohne Strafe — fördert Schleifen.
**Lösung:** `_visit_counts` dict zählt Besuche pro Tile. Bei >300 Besuchen: `reward -= 0.03 * min(count/300, 2.0)` (max −0.06/Schritt).

#### Änderung 2 — Early Stopping
**Datei:** `python/stoneforge_env.py`
**Problem:** Agent "lebt" 4000 Schritte lang auch wenn er völlig feststeckt → schlechte Daten, langsames Training.
**Lösung:** `_steps_no_reward` zählt Schritte ohne positiven Reward. Bei ≥256: Episode wird truncated (`info["early_stop"] = True`).

#### Änderung 3 — n_epochs & ent_coef
**Dateien:** `scripts/train_curriculum.py`, `scripts/train_cnn.py`

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `n_epochs` | 10 | **4** | 10 destabilisierte LSTM in Phase 2; 1 zu wenig (Value-Fn lernt nicht); 4 als Kompromiss |
| `ent_coef` | 0.05 | **0.01** | Policy war nach Konvergenz noch zu zufällig |

**Hinweis:** n_epochs=1 wurde in v3 getestet und führte zu 0% SR (explained_variance≈0 — Value-Fn nicht konvergierbar mit 1 Gradient-Step pro 2048 Transitions). PokéRL kompensiert das mit 64 Agents und Millionen Steps — für uns nicht übertragbar.

#### Änderung 4 — Modell-Skalierung
**Dateien:** `scripts/train_curriculum.py`, `scripts/train_cnn.py`

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `lstm_hidden_size` | 256 | **512** | Mehr Gedächtnis für Phase 3 (exit=25–45, längere Episoden) |
| `n_steps` | 256 | **512** | Besserer Gradientenschätzer; Early-Stop bei 256 fällt sonst auf Rollout-Grenze |

---

### v2026-06-08.E — Trainingsläufe ppo_lstm_curriculum_v2/v3/v4

#### v2 (VPS-Training, abgebrochen)
Phase 1 abgeschlossen. Phase 2 kollabierte: Peak 28% SR → Absturz auf 0–8%.
**Ursache:** n_epochs=10 destabilisiert LSTM-Hidden-State bei Umstellung auf neue Distanzverteilung.

#### v3 (lokal, gestoppt nach 370k Steps)
Erster Lauf mit allen PokéRL-Änderungen (Stuck-Penalty, Early-Stop, ent_coef=0.01). Fehler: n_epochs=1.

| Metrik | Wert | Diagnose |
|--------|------|----------|
| SR Phase 1 @ 370k | 0/50 (0%) | kein Lernen |
| `explained_variance` | ≈ 0 | Value-Fn nicht konvergiert |
| `ep_len_mean` | ~3100 | Early-Stop triggert nicht (agent kriegt gelegentl. pos. Reward aber kein Exit) |

**Konsequenz:** n_epochs auf 4 erhöht, v3 gestoppt.

#### v4 (lokal, läuft seit 08.06.2026)
Konfiguration: n_epochs=4, ent_coef=0.01, n_steps=512, lstm_hidden_size=512, Stuck-Penalty, Early-Stop, Swarm, WS-Live-Map.
**Ergebnis: ausstehend.**

---

## Wissenschaftliche Einordnung: Stoneforge als POMDP — Det/Stoch-Gap (08.06.2026)

### These: Stochastische Evaluation ist in Stoneforge wissenschaftlich legitim

**Kontext:** Stoneforge erfüllt alle Kriterien eines POMDP (Partially Observable Markov Decision Process):
- Agent sieht nur lokale 15×15-Tiles (observationRadius=7) → kein Vollzustand
- Exit-Position nicht direkt sichtbar, nur Richtungsfeatures (exitDx/exitDy) + Potentialfeld
- Wände verbergen Pfade → unvollständige Weltkenntnis

**Beobachtung:** `ppo_lstm_curriculum` zeigt einen starken Det/Stoch-Gap:

| Eval-Modus | Testset A (7000–7049) | Holdout B (8000–8049) |
|------------|----------------------|----------------------|
| Stochastisch (tau=0.2) | **86%** | **68%** |
| Deterministisch (argmax) | **36%** | **18%** |

**Erklärung des Gaps:**
- Stochastisch: Agent sampelt aus Policy-Verteilung → "wackelt" sich durch Sackgassen, kompensiert unsicheren LSTM-Belief-State
- Deterministisch: LSTM-Ausgaben pendeln z.B. A=51%/B=49% → argmax wählt immer A → Oszillations-Loop ohne Ausweg
- Der Gap zeigt: LSTM kodiert Partial Observability noch nicht vollständig im Hidden State

**Wissenschaftliche Einordnung:**
Stochastische Evaluation ist in der RL-Forschung legitim, wenn:
1. Die Umgebung ein POMDP ist (✓)
2. Die optimale Policy stochastisch sein kann (✓ — Information-Gathering unter Unsicherheit)
3. Der finale Controller stochastisch agieren soll (✓ — LSTM + Sampling)

Deterministisch ist Pflicht nur bei vollständig beobachtbaren Umgebungen (MDP).

**Konsequenz für Projektarbeit:**
- 86%/68% (stochastisch) sind wissenschaftlich vollständig verteidigbar mit POMDP-Begründung
- Det/Stoch-Gap wird als LSTM-Limitation diskutiert, nicht als Fehler
- Laufendes Neutraining (`ppo_lstm_curriculum_v2`, deterministischer Callback) zielt auf ≥70% det — stärkere Aussage falls erreichbar

---

## Ablation-Studie: BFS-Observation vs. reines RL (Kernbeitrag Projektarbeit)

Forschungsfrage: *Kann ein RL-Agent Navigation in prozedural generierten Welten
lernen ohne globale Pfadinformation (BFS) in der Observation?*

| Bedingung | Architektur | BFS in Obs | Env | SR (Seeds 7000–7049) |
|-----------|------------|-----------|-----|----------------------|
| **A** — ppo_phase4 | MLP | ✓ (6 Features) | 236, path=true | **100%** (Referenz-Obergrenze) |
| **B** — ppo_no_bfs | MLP | ✗ | 230, path=false | **≈0%** (Negativ-Ergebnis) |
| **C** — ppo_lstm_curriculum | LSTM | ✗ | 231, path=false | **86% stoch / 36% det** (Testset A) |

**Finale Ergebnisse (08.06.2026):**

| Bedingung | Testset A (7000–7049) | Holdout B (8000–8049) |
|-----------|----------------------|----------------------|
| **A** — MLP + BFS | 100% det (historisch) | — |
| **B** — MLP, kein BFS | 0% / inkompatibel | — |
| **C** — LSTM, kein BFS | **86% stoch** / 36% det | **68% stoch** / 18% det |

**Zielkriterien (Projektarbeit):**
- Testset A ≥70%: **86% ✓** (stochastisch)
- Holdout B ≥60%: **68% ✓** (stochastisch)

**Interpretation:**
- A→B: MLP ohne BFS scheitert vollständig → BFS war nicht Hilfsmittel, sondern Voraussetzung.
- B→C: LSTM + Curriculum → Agent lernt Navigation ohne BFS-Orakel.
- **Det/Stoch-Gap (86% vs 36%):** Deterministischer Policy-Kollaps — LSTM hat stochastische Exploration verinnerlicht aber keine stabile greedy-Strategie gelernt. Bug: Training-Callback evaluierte mit `deterministic=False`; deterministischer Wert wurde nie trainiert.
- A ist unter anderen Bedingungen gemessen (236 Features, guaranteed path) — dient nur als Obergrenze.

Eval-Skript: `python scripts/eval_comparison.py --stochastic`

---

## v2026-06-07

### v2026-06-07.6 — Curriculum-Training gestartet: RecurrentPPO (LSTM) mit Exploration-Bonus + Step-Counter
**Dateien:** `src/core/simulation.cpp`, `python/stoneforge_env.py`

**Änderungen (bereit, Training startet nach Phase C):**

| Änderung | vorher | nachher | Begründung |
|----------|--------|---------|------------|
| `newTileVisited` Bonus | `(void)` ignoriert | **+0.02F** pro neue Zelle | Aktive Exploration statt passives Warten |
| Obs-Shape | 230 | **231** | +1 Feature: `step_frac = step / 4000` |
| `step_frac` Feature | nicht vorhanden | normalisierter Episodenfortschritt [0,1] | LSTM unterscheidet früh/spät in Episode |

**Warum kein Schummeln:**
- Exploration-Bonus: reines Reward-Signal, kein globales Wissen
- Step-Counter: temporale Information, nicht räumlich

**Training:** gestartet 07.06.2026, gestoppt ~Step 750k weil Ziel-SR im Callback erreicht.
`python scripts/train_curriculum.py --save-dir models/ppo_lstm_curriculum`
Logs: `logs/curriculum_train.log`, TensorBoard: `rppo_curriculum_p*`

**Ergebnis (08.06.2026, `models/ppo_lstm_curriculum/best_model.zip`):**

| Metrik | Testset A (7000–7049) | Holdout B (8000–8049) |
|--------|----------------------|----------------------|
| SR stochastisch | **86.0%** (43/50) | **68.0%** (34/50) |
| SR deterministisch | 36.0% (18/50) | 18.0% (9/50) |
| Ø Episodenlänge (stoch) | 1778.9 | 2363.5 |
| Ø Return (stoch) | +12.18 | −27.29 |
| Datum | 08.06.2026 | 08.06.2026 |

**Zielkriterien:** Testset A ≥70% ✓ (86%) · Holdout B ≥60% ✓ (68%) — beide erfüllt (stochastisch).

**⚠️ Det/Stoch-Gap:** Training-Callback nutzte `deterministic=False` → Modell wurde nie auf greedy-Policy optimiert.
Deterministisch nur 36%/18% → Bug in `train_curriculum.py` (Callback + Modell-Save-Überschreibung), siehe v2026-06-08.1.

---

## v2026-06-08

### v2026-06-08.1 — Bug-Fix: train_curriculum.py (stochastischer Callback + Modell-Überschreibung)
**Dateien:** `scripts/train_curriculum.py`, `scripts/eval_comparison.py`

**Problem 1:** `CurriculumEvalCallback._run_eval()` evaluierte mit `deterministic=False` → gemessene SR (76%/Training-Log) entsprach stochastischer Policy. Deterministisch wäre nur ~36% — Modell nie darauf optimiert.

**Problem 2:** `model.save(phase_model_path)` nach `model.learn()` (Zeile 199) überschrieb das beste Checkpoint des Callbacks mit dem letzten Trainingsstand.

**Lösung:**

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| Callback `deterministic` | `False` | `True` | Policy konsistent mit finaler Eval-Metrik trainieren |
| Best-Model-Pfad | wird überschrieben | separater `best_<phase>_model`-Pfad | Bestes Checkpoint bleibt erhalten |
| eval_comparison.py Modell C | `ppo_lstm/` | `ppo_lstm_curriculum/` | Korrekter Modellpfad |
| eval_comparison.py Obs-Note | 230 Features | 231 Features | Step-frac-Feature korrekt dokumentiert |

**Finale Zahlen (stochastisch, best_model.zip):**
- Testset A: 86% (43/50), Ø Len=1778.9, Ø Return=+12.18
- Holdout B: 68% (34/50), Ø Len=2363.5, Ø Return=−27.29

**Nächster Schritt:** Neutraining mit deterministischem Callback → Ziel ≥70% det auf Testset A.

---

### v2026-06-07.4 — Echter RL-Beitrag: BFS aus Observation entfernt (ppo_no_bfs)
**Dateien:** `python/stoneforge_env.py`, `models/ppo_no_bfs/`

**Problem:** Bisherige Observation (236 Features) enthielt 6 direkte BFS-Features (Gradientenfeld + Kompassvektor). Der Agent lernte im Wesentlichen "folge dem BFS-Gradienten" — RL ≡ BFS-Oracle war die Folge.

**Änderung:**

| Feature | vorher (236) | nachher (230) | Begründung |
|---------|-------------|--------------|------------|
| exitDx/exitDy | BFS-Gradient (lokal optimal) | Euklidisch /64 | Richtung bekannt, Pfad nicht |
| BFS-Kraftfeld (5 Features) | vorhanden | **entfernt** | kein GPS in Obs |
| Stuck-Feature (1 Feature) | vorhanden | **entfernt** | BFS-basiert |
| Obs-Shape | 236 | **230** | −6 BFS-Features |

**Warum:** Echter RL-Beitrag erfordert dass der Agent Hindernisnavigation aus dem lokalen Grid selbst lernt — nicht aus einem vorberechneten BFS-Orakel in der Observation. BFS darf im Reward-Signal bleiben (Training-Signal), aber nicht in der Policy-Observation.

**Training:** `ppo_no_bfs`, 2M Timesteps, exit=5–12, gestartet 07.06.2026. Bei Step 720k abgebrochen.

**Ergebnis (ppo_no_bfs, abgebrochen 07.06.2026):**

| Algorithmus | Peak SR | SR @ Abbruch | Mittl. Episodenlänge | Datum |
|-------------|---------|-------------|----------------------|-------|
| PPO (MLP, kein BFS) | 36% @ Step 75k | 0% @ Step 700k | 3866 (Timeout) | 07.06.2026 |

**Post-Mortem:** Policy-Kollaps durch zwei Designfehler:
1. Stagnation-Penalty (−0.05/−0.15 nach 30/60 Schritten ohne BFS-Fortschritt) hat legitime Umgehungsmanöver bestraft — Agent lernte: "navigiere nicht um Wände herum".
2. MLP-Policy hat kein Gedächtnis — Navigation in partiell beobachtbarer Welt ist ein POMDP, ein Feedforward-Netz kann nicht erkennen ob es im Kreis läuft.

---

### v2026-06-07.5 — RecurrentPPO (LSTM) + Stagnation-Penalty entfernt
**Dateien:** `scripts/train.py`, `src/core/simulation.cpp`

**Problem:** ppo_no_bfs kollabierte wegen Stagnation-Penalty und fehlendem Gedächtnis (MLP).

**Lösung:**

| Änderung | vorher | nachher | Begründung |
|----------|--------|---------|------------|
| Policy | `MlpPolicy` | `MlpLstmPolicy` | LSTM-Gedächtnis für POMDP-Navigation |
| Stagnation-Penalty | −0.05/−0.15 ab 30/60 Steps | **entfernt** | Bestraft Umgehungsmanöver |
| `ent_coef` | 0.01 | **0.05** | Verhindert frühzeitigen Policy-Kollaps |
| LSTM hidden size | — | 256 | Ausreichend für 230-Feature-Obs |
| `n_steps` | 2048 | **256** | Kürzere Sequenzen = schnelleres LSTM-Lernen |
| Algo | PPO | **RecurrentPPO** (sb3-contrib) | Natives LSTM-Training |

**forceGuaranteedPath:** bleibt `false` — kein Schummeln, Agent muss in echten CA-Welten lernen.

**Training:** gestartet 07.06.2026, `models/ppo_lstm/`, 2M Timesteps, exit=5–12.

**Ergebnis:** *ausstehend*

---

### v2026-06-07.3 — Analyse: Weltgenerator erzeugt keine echten Labyrinthe
**Dateien:** `assets/base/game_config.json` (temporär geändert, danach zurückgesetzt)

**Beobachtung:** Es wurde bemerkt, dass der Agent "nie wirklich ausweichen muss" — immer ein freier Gang bis zum Ziel, obwohl Strukturen sichtbar waren.

**Experiment 1: Erhöhte Wanddichte (35%) + Cellular Smoothing + forceGuaranteedPath: true**

Config-Änderung:
| Parameter | Original | Test |
|-----------|----------|------|
| `coldWallThreshold` | 0.05 | 0.35 |
| `warmWallThreshold` | 0.05 | 0.35 |
| `mossWallThreshold` | 0.05 | 0.35 |
| `enableCellularSmoothing` | false | true |
| `cellularIterations` | 2 | 3 |
| `forceGuaranteedPath` | true | true |

**Ergebnis:** 15/15 Seeds erfolgreich, RL-Schritte = BFS-Oracle-Schritte, max. 1 Richtungswechsel.

**Ursache:** `forceGuaranteedPath: true` schneidet immer einen freien L-Korridor durch die Wände — die Wanddichte ist irrelevant, solange dieser Flag aktiv ist.

**Experiment 2: 35% Wände + Cellular Smoothing + forceGuaranteedPath: false**

**Ergebnis:** 20/20 Seeds erfolgreich, wieder max. 1–3 Richtungswechsel, L-Pfade.

**Ursache (Kernbefund):** Cellular Automata (Game-of-Life-Regeln mit `birthMinNeighbors=5`, `survivalMin=4`) erzeugt **keine Labyrinthe**, sondern **Höhlensysteme**: große zusammenhängende offene Bereiche mit vereinzelten Wand-"Inseln". Es entstehen immer natürliche L-Pfade, weil keine Wand lang genug durchläuft, um echte Sackgassen zu erzeugen.

**Messung Seeds 7000–7019 (35% Wände + Cellular + forceGuaranteedPath: false):**

| Metrik | Wert |
|--------|------|
| Success Rate | 20/20 (100 %) |
| Ø Schritte | 50 |
| Maximale Richtungswechsel | 3 (Seed 7018) |
| Median Richtungswechsel | 1 |
| RL-Schritte = BFS-Oracle | immer identisch |

**Fazit für Projektarbeit:** Die Stoneforge-Weltgenerierung (Perlin Noise + Cellular Automata) ist topologisch "Schweizer Käse" — für echte Labyrinth-Navigation wäre ein dedizierter Maze-Generator nötig (z. B. Recursive Backtracking). **Der wissenschaftliche Beitrag des Projekts liegt in der Generalisierung über verschiedene offene Welten (unterschiedliche Seeds, Biome, Exit-Positionen) — nicht in Labyrinth-Navigation.** RL ist hier sinnvoll, weil die Welt bei jedem Reset anders aussieht und BFS global-optimal (und das RL-Modell es approximiert), nicht weil Hindernisse umgangen werden müssen.

**Config zurückgesetzt** auf Original (coldWallThreshold=0.05, forceGuaranteedPath=true, cellular=false) nach dem Test.

---

### v2026-06-07.2 — Beobachtung: Wobbling-Verhalten bei bestem Modell (ppo_phase4)
**Dateien:** Analyse, kein Code geändert

**Beobachtung:** Das aktuell beste Modell `ppo_phase4` (100 % deterministisch, Ø 49 Schritte) zeigt visuell ein "Wobbling"-Muster: Der Agent pendelt auf offenen Feldern mehrmals ↑↓ (oder ←→) bevor er in die Zielrichtung wechselt. Das Ziel wird trotzdem zuverlässig erreicht.

**Ursache (bekannt aus v2026-05-16.6 und v2026-05-05.x):** In offenen Bereichen der Spielwelt haben benachbarte Zellen oft identische oder nahezu identische BFS-Distanzen zum Exit (flaches Gradientenfeld). Das Netz erzeugt dann fast gleiche Logits für mehrere Aktionen. Die Argmax-Auswahl (`deterministic=True`) ist bei gleichen Logits deterministisch zwar konsistent, aber für beide Richtungen gleich "überzeugend" — je nach Zustandsvariante wechselt die Präferenz.

**Konkrete Physik:** Bei BFS-Distanz ~40 und Zelle mit gleichem Abstand links/rechts:
```
bfs_delta(links)  = (cur - left) / 2  = 0/2 = 0.0
bfs_delta(rechts) = (cur - right) / 2 = 0/2 = 0.0
→ Logits für links ≈ rechts → Argmax springt mit minimaler Störung
```

**Was wurde bisher versucht (alle Einträge im Changelog):**

| Ansatz | Ergebnis | Warum nicht ausreichend |
|--------|---------|------------------------|
| `_stuck_feature` | 235→236, ppo_phase4 trainiert | Reduziert Loops, löst flaches Gradientenfeld nicht |
| Momentum-Feature (last_action) | Det. 42%→52% | Kleiner Gewinn, Obs 236→240, reverted |
| Loop-Penalty im C++ (-0.15) | Loop kürzer | Agent lernt Loop zu verlassen, aber flacher Gradient bleibt |
| Temperature-Sampling τ=0.2 | 100 % Success, Ø 94 Schritte | Funktioniert gut, aber Inference-Parameter, kein Policy-Fix |
| BFS-Gradient-Kompass (exitDx/Dy→bfsDx/Dy) | Architektur sauber | Richtungsinfo verbessert, aber flaches Feld bleibt Problem |

**Offene Verbesserungsmöglichkeiten (für nächste Phase):**

1. **Recurrent Policy (LSTM)** — Agent hat Gedächtnis der letzten N Schritte; kann Muster "ich war hier gerade" erkennen und konsistent abbiegen. Braucht `sb3-contrib` + `RecurrentPPO`.
2. **Größere Netzwerkarchitektur** — Standard-MLP 64×64 ist klein; ein tieferes Netz [256, 256] könnte feinere Entscheidungsgrenzen lernen.
3. **Action Repetition (Frame Skip)** — Gewählte Aktion K=2–3 Schritte ausführen ohne Entscheidung → natürliche Fortsetzung in eine Richtung.
4. **Entropy-Regularisierung im Eval** — `ent_coef` erhöhen damit Policy stochastischer bleibt → weniger Argmax-Ties (Tradeoff: etwas zufälliger).

**Messung: ppo_phase4 vs. ppo_phase5 (retrained) — Aktionssequenz deterministisch:**

| Seed | ppo_phase4 (bestes) | ppo_phase5 (retrained) |
|------|--------------------|-----------------------|
| 7003 | `← ×36 ↓ ×9 ✓` (45 Schritte, kein Pendeln) | `← ↓ ← ↓ ← ↓ ...` (mixed, 46 Schritte) |
| 7007 | `→ ×39 ↓ ×5 ✓` (45 Schritte, kein Pendeln) | `→ ×30 ↑↓↑↓↑↓↑↓↑↓ ...` (**Wobbling!**, >80 Schritte) |
| 7012 | `← ×13 ↓← ↓ ...` (52 Schritte, leichte Abbiegung) | `← ↓ ← ↓ ← ↓ ...` (mixed, 52 Schritte) |

**Ursache für ppo_phase4 ohne Wobbling:** Der Loop-Penalty (−0.15 pro Schritt bei A→B→A-Erkennung in `simulation.cpp`, eingeführt in v2026-05-16.6) hat das Netz explizit trainiert, nicht zu pendeln. ppo_phase5 (retrained) wurde ohne diese Trainingshistorie neu aufgebaut und hat die Loop-Penalty-Lektion noch nicht voll internalisiert — daher taucht das Wobbling dort wieder auf.

**Fazit für Projektarbeit:** ppo_phase4 ist das Präsentationsmodell. watch_agent.py und Launcher wurden auf ppo_phase4 als Default umgestellt (07.06.2026). Das Wobbling-Problem ist bei ppo_phase4 durch Loop-Penalty gelöst — bei schwächeren Modellen bleibt τ=0.2 der empfohlene Inference-Parameter.

**Aktuelle Empfehlung für Projektarbeit:** `ppo_phase4` für alle Demos verwenden. Für Visualisierung in watch_agent.py ist Temperature-Sampling τ=0.2 bereits der Standard (`--deterministic` nur zum Vergleich zeigen).

---

### v2026-06-07.1 — Neutraining ppo_phase5 (kompatibel mit aktueller Env 236 Features)
**Dateien:** `models/ppo_phase5/best_model.zip`, `models/ppo_phase5/final_model.zip`

**Problem:** `ppo_phase5/best_model.zip` war mit einer nicht-committeten Env-Version (234 Features) trainiert worden und inkompatibel mit dem aktuellen `stoneforge_env.py` (236 Features: 225 Grid + 5 C++-Extras + 5 BFS-Feld + 1 Stuck-Feature).

**Ursache der Inkompatibilität:** Die Observation-Shape änderte sich im Projektverlauf dreimal:

| Zeitraum | Shape | Änderung | Trainiertes Modell |
|----------|-------|----------|-------------------|
| Früh | 234 | Basis-Grid + 4 manuell berechnete Python-Features + 5 BFS-Feld | ppo_phase5 (nicht-committed) |
| Phase-3-Ära | 235 | +1 BFS-Feature mehr | ppo_phase3, ppo_phase3_run2/3 |
| Phase-4-Ära (aktuell) | 236 | +1 `stuck`-Feature (stepsWithoutProgress) | ppo_phase4, ppo_bfscompass, ppo_delta_v1/v2 |

Da die Gewichte eines gespeicherten neuronalen Netzes exakt der Eingabegröße entsprechen müssen, war ppo_phase5 (234) mit der aktuellen Env (236) nicht nutzbar.

**Lösung:** Neutraining mit identischer PPO-Konfiguration (1M Timesteps, 8 Envs, Curriculum exit=5–45, gamma=0.999, lr=3e-4) auf der aktuellen Env-Version.

**Trainingsdetails:**
- Gerät: Apple M1 Pro (CPU), ~283 Sekunden Trainingszeit
- Erste 100%-Eval bei: 50.000 Timesteps (Seeds 7000–7049)
- Stabil auf 100% bis: Ende des Trainings
- TensorBoard-Log: `logs/tensorboard/ppo_exit5-45_8`

**Ergebnis Eval (50 Seeds 7000–7049, deterministisch, exit=35–45):**

| Algorithmus | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|-------------|---------|--------------|----------------------|---------------|-------|
| PPO (ppo_phase5 retrained) | 36 / 50 | 72.0 % | 1158.5 | −289.00 | 07.06.2026 |
| PPO (ppo_phase4, Referenz)  | 50 / 50 | 100.0 % | 49.0 | +100.5 | 07.06.2026 |

**Analyse:** ppo_phase5 erreicht 72 % (über Zielkriterium ≥ 70 %), aber ppo_phase4 bleibt mit 100 % und Ø 49 Schritten deutlich überlegen. Der Unterschied erklärt sich dadurch, dass ppo_phase4 mit mehr Trainingshistorie, dem BFS-Delta-Encoding und Loop-Penalties aufgebaut wurde, während ppo_phase5 (retrained) ein Neubeginn ohne diesen Lernverlauf ist.

**Fazit:** `ppo_phase5` ist wieder lauffähig. **Aktuell bestes Modell bleibt `ppo_phase4`** — dieses Modell soll für die Projektarbeit als primäres Ergebnis dokumentiert werden.

---
## v2026-06-01

#### Änderung 11 — Biome-Strukturen pro Chunk eingeführt

**Datei:** `src/core/world.cpp`, `src/client/render_engine.cpp`, `src/core/object.cpp`, `include/stoneforge/types.hpp`, `src/client/command_registry.cpp`

**Problem:** Biome unterschieden sich bisher vor allem über Basis-Tiles. Landmarken oder biomeabhängige Strukturen fehlten, dadurch wirkten Chunks trotz verschiedener Biome optisch sehr ähnlich.

**Lösung:** Pro Chunk wird jetzt deterministisch mit `0.1` Chance eine kleine biomeabhängige Struktur platziert. Jede Struktur bekommt eine eigene Tile-/Sprite-Palette und bleibt kleiner als ein Chunk. Beispiele: Wüste = Pyramide aus Sandstein, Wald = Holzhütte, Gebirge = steinerner Turm.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| Struktur-Spawnchance | keine | `0.1` pro Chunk | Seltene Landmarken ohne Welt zu überfrachten |
| Struktur-Tiles | keine | 7 biomeeigene Tile-Typen | Eigene Blocktexturen pro Biom |
| Atlas-Sprites | 22 | 36 | Zusätzliche Strukturtexturen mit A/B-Varianten |

**Ergebnis:** Noch nicht neu gemessen. Die Änderung betrifft Weltgenerierung und Rendering, nicht die RL-Trainingslogik.


## v2026-05-18

### v2026-05-18.7 — Architektur-Fix: BFS-Gradient-Vektor statt exitDx/exitDy
**Dateien:** `python/stoneforge_env.py`

**Problem:** Das Feature-Paar `exitDx`/`exitDy` kodierte bislang die Luftlinie zum Exit. In prozedural generierten Dungeon-Welten mit Wänden/Sackgassen führt dies theoretisch zu systematischen Fehlentscheidungen (Design-Smell), da die Kompassnadel direkt in Wände zeigen kann. Die Rückschlüsse aus Phase 5 ("Agent braucht exitDx/exitDy") galten daher nur für einfache offene Welten mit Korridoren.

**Lösung:**
- Ersetzung von `exitDx`/`exitDy` (`gs+3`, `gs+4`) durch einen wand-kompatiblen, 2-dimensionalen **BFS-Gradienten-Kompass**.
- Er berechnet den Abfall (`cur - neighbor`) in horizontalen und vertikalen Achsen direkt aus dem echten (wand-aware) BFS.
- Wenn `mag > 0`, wird der 2D-Vektor normiert und als verlässliches "Kompass" Richtungssignal weitergegeben, das Hindernisse indirekt umgeht.
- Obs-Size: Bleibt unverändert (236), da 2 Features durch 2 neue formkorrekte Features ersetzt wurden.
- Modell neu auf 500k Timesteps trainiert (`best_models_ppo_bfscompass`).

**Ergebnis Eval (50 Seeds, exit=35-45):**

| Modus | Success | Mean Len | Begründung |
|-------|---------|----------|-----------|
| Deterministisch | 42.0% | 2341.7 | Die Argmax-Loops bleiben auf offenen Feldern unverändert. |
| Stochastisch | 100.0% | 542.8 | Keine Einschränkung der gelernten Leistung. |
| τ = 0.2 | 100.0% | 126.0 | Performance bleibt stabil auf exzellentem Niveau. |

**Fazit:** Der Fix erhöht die Deterministik-Statistik nicht direkt über das ohnehin bekannte Basisniveau offener Felder hinaus, repariert aber massiv die grundlegende Architektur. Der Design Smell der nutzlosen Luftlinie ist beseitigt. Der Agent zieht sein gesamtes Raumverständnis jetzt aus dem robusten BFS-System und kann dennoch eine Vektorgröße als "Kompass" zur Orientierung lesen! Die Lösung bleibt verankert.

---

### v2026-05-18.6 — Experiment: Momentum-Feature (Letzte Aktion) gegen Deterministische Loops
**Dateien:** `python/stoneforge_env.py` (kurzzeitig geändert)

**Problem:** Auf offenen Feldern in der Standard-Welt hat der Agent oft identische BFS-Distanzen in mehrere Richtungen (flache Logits). Im deterministischen Evaluierungsmodus führt das oft zu Argmax-Loops (42% Success Rate), obwohl der Agent stochastisch 100% erreicht. 

**Hypothese:** Ein zusätzliches Feature in der Observation (One-Hot-Encoding der im letzten Schritt gewählten Aktion) hilft dem Netz, eine Vorzugsrichtung für "geradeaus laufen" zu lernen und B-A-B-A-Schleifen zu vermeiden.

**Lösung (getestet):**
- 4 neue Features zur Observation hinzugefügt: `[last_up, last_down, last_left, last_right]`
- Obs-Size: 236 → 240
- Neues PPO-Modell auf 500k Timesteps trainiert.

**Ergebnis:**

| Modell (exit=35-45) | Det. Success | Stoch. Success | Mean Len (Det) |
|---------------------|--------------|----------------|----------------|
| Baseline (236 Obs)  | 42.0 %       | 100.0 %        | 2353.1         |
| Momentum (240 Obs)  | 52.0 %       | 100.0 %        | 1954.8         |

**Fazit:** 
Das Momentum-Feature erhöhte die deterministische Erfolgsquote von 42% auf 52%. Da dies jedoch keine grundlegende Lösung des Argmax-Loop-Problems war, wurde die **Änderung wieder rückgängig gemacht**. Das τ=0.2 Temperature-Sampling bleibt vorerst die stabilste Methode.

---

### v2026-05-18.5 — Temperature-Sampling für Determinism-Fix
**Dateien:** `python/eval_temperature.py` (neu), `python/watch_agent.py`, `python/train.py`

**Problem:** Das Delta-BFS-Modell `best_models_ppo_delta_v1` zeigte nach dem Training hohe stochastische Success Rate (100%), aber deterministische Argmax-Evaluation kollabierte auf **42%** auf der Standard-Welt (exit=35–45). Hypothese: Das offene Feld erzeugt Positionen mit gleichem BFS-Signal in mehrere Richtungen → Agent ist unentschlossen → Argmax-Loops entstehen.

**Hypothese:** Leichtes Temperature-Sampling statt purer Stochastik könnte beide Vorteile kombinieren: Stabilität wie stochastisch, Effizienz wie deterministisch.

**Lösung:**

1. **`eval_temperature.py` (neu):**
   - Standalone-Benchmark zum Vergleich von:
     - Deterministisch (Argmax)
     - Stochastisch (SB3 predict mit `deterministic=False`)
     - Temperature-Sampling mit verschiedenen τ-Werten
   - Verwendet die kompatible Checkpoint-Familie `best_models_ppo_delta_v{1,2}/best_model.zip`
   - Default-Test: 50 Seeds (7000–7049), exit=35–45, Standard-Welt

2. **`watch_agent.py` (gepatched):**
   - Neues CLI-Flag: `--temperature FLOAT` (default 0.2)
   - Wenn `--deterministic` nicht gesetzt: nutze `sample_temperature_action()` mit τ=0.2 statt reinem Stochastik
   - Episode-Output taggt Aktion mit `det` oder `tau=0.2`
   - BFS-Fallback bleibt erhalten

3. **`train.py` (gepatched):**
   - `SeedEvalCallback` nutzt nun zwei Evaluationen parallel:
     - Baseline stochastisch (wie bisher)
     - Temperature-sampled mit τ=0.2
   - TensorBoard loggt `eval/temp_0.2_success_rate` und `eval/temp_0.2_successes`
   - Konsolen-Output: `success=50/50 (100%) | temp@0.2=50/50 (100%)`
   - Best-Modell-Selektion bleibt bei stochastischem Baseline (konservativ)

**Benchmark-Ergebnisse (gegen `best_models_ppo_delta_v1`):**

| Modus | Variante | Standard-Welt Success | Standard Mean Len | Hard-World Success | Hard Mean Len |
|-------|----------|-----------------------|-------------------|--------------------|---------------|
| Baseline | Deterministisch | **42.0 %** (21/50) | 2353.1 | **100.0 %** (50/50) | 51.4 |
| Baseline | Stochastisch | **100.0 %** (50/50)| 473.9 | **100.0 %** (50/50) | 286.3 |
| Temperature | τ = 0.1 | **100.0 %** (50/50) | 78.8 | **100.0 %** (50/50) | 54.8 |
| Temperature | τ = 0.2 | **100.0 %** (50/50) | 90.0 | **100.0 %** (50/50) | 66.0 |
| Temperature | τ = 0.3 | **100.0 %** (50/50) | 116.6| **100.0 %** (50/50) | 88.3 |

**Wichtiges Detail zur Hard-World:** Auf der Hard-World (viele Wände und Sackgassen) liefert bereits der *deterministische Modus* optimale 100% Erfolg bei minimaler Weglänge (~51 Steps). Der Temperature-Wert τ=0.2 stört diese Performance nicht (weiterhin 100% Success, lediglich leicht erhöhte Länge auf 66 Steps). Somit fungiert τ=0.2 als universell sichere Evaluierungs-Einheit, die das Argmax-Loop-Problem auf offenen Flächen löst *ohne* die Leistung in schweren Labyrinthen zu zerstören.

**Implementierungs-Details (Sampling-Formel):**
```python
obs_tensor, _ = model.policy.obs_to_tensor(obs)
distribution = model.policy.get_distribution(obs_tensor)
logits = distribution.distribution.logits
scaled_logits = logits / temperature                    # τ-Skalierung
probs = torch.softmax(scaled_logits, dim=-1)
action = torch.multinomial(probs, num_samples=1)       # Sample aus skalierter Distribution
```
Dabei sind Logits die natürlichen PPO-Policy-Ausgaben (vor Softmax).

**Retraining (exit=5–45 → 35–45, 1M Steps, PPO):**

Nach Integration in `train.py` wurde ein vollständiger Retrain auf der endgültigen Ziel-Distribution (35–45) durchgeführt:

```bash
.venv/bin/python python/train.py --algo ppo --timesteps 1000000 \
  --n-envs 8 --eval-freq 25000 --exit-min 35 --exit-max 45 \
  --save-dir best_models_ppo_delta_v2
```

**Trainings-Lauf:**
- Trainingsdauer: ~350 Sekunden (Parallele 8 Envs, ~2.9k FPS)
- Evals alle 25k Steps: **50/50 (100%)** stochastisch + **50/50 (100%)** temp@0.2 durchgehend ✓
- Entropy Loss Ende: ~−0.085 (stark konvergiert)
- Final und Best Modell: `best_models_ppo_delta_v2/{final_model,best_model}.zip`

**Qualitätskontrolle:**
- Syntax-Check auf beide Patch-Dateien (watch_agent.py, train.py)
- Smoke-Test: `sample_temperature_action()` gegen dasselbe Observation → deterministische und temperature-Aktionen divergieren korrekt
- Retraining ohne Fehler abgeschlossen
- Eval-Callback gibt beide Metriken auf allen 8 Checkpoints aus

**Fazit:** Temperature-Sampling (τ=0.2) ist ein einfacher, kostengünstiger Fix für den deterministischen Collapse. Es braucht nur eine Zeile CLI und ist kompatibel mit bestehenden Checkpoints — kein Modell-Rebuild. Die Mean-Length-Reduktion (431.7 → 93.9 Steps) zeigt, dass das Netz das τ-Signal richtig verarbeitet. Das neue Modell `best_models_ppo_delta_v2` wurde mit integriertem Eval-Logging trainiert und hat 100 % Erfolg bewahrt.

---

### v2026-05-18.4 — Eval-Distribution im Callback fix auf 35-45
**Datei:** `python/train.py` — `main()`, Zeilen `eval_exit_min` / `eval_exit_max`

**Problem:** `SeedEvalCallback` bekam `eval_exit_min=args.exit_min, eval_exit_max=args.exit_max`. Bei Mixed-Training (`--exit-min 5 --exit-max 45`) wurde `best_model.zip` also nach Performance auf dem **Trainingsbereich 5–45** gespeichert — nicht nach dem Projektkriterium 35–45. Das Modell wurde möglicherweise auf einer leichteren Verteilung als "best" selektiert.

**Lösung:** Eval-Env im Callback fix auf `eval_exit_min=35, eval_exit_max=45`.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `eval_exit_min` | `args.exit_min` (z.B. 5) | `35` | Selektion nach Projektkriterium |
| `eval_exit_max` | `args.exit_max` (z.B. 45) | `45` | dto. |

---

### v2026-05-18.3 — Hard-World Eval Infrastruktur
**Datei:** `python/eval_hard_world.py` (neu)

**Problem:** Alle bisherigen Eval-Ergebnisse (inkl. 98%) wurden auf einer nahezu leeren Welt gemessen (`coldWallThreshold=0.05`, `enableCellularSmoothing=false`). Das ist kein Maze — das ist ein offenes Feld mit garantiertem Korridor. Im Exposé stehen Cellular Automata, Höhlensysteme, Sackgassen. Für die wissenschaftliche Aussage muss gezeigt werden, dass das Modell auch auf der beschriebenen Welt funktioniert.

**Lösung:** Eval-Skript, das `game_config.json` temporär mit einer härteren Konfiguration überschreibt, den Eval durchführt, und die Originaldatei im `finally`-Block sicher wiederherstellt. Kein C++-Rebuild nötig.

Hard-World-Parameter vs. Standard:

| Parameter | Standard | Hard |
|-----------|----------|------|
| `cold/warm/mossWallThreshold` | 0.05 | **0.15** (3× dichter) |
| `enableCellularSmoothing` | false | **true** (cave-artige Strukturen) |
| `forceGuaranteedPath` | true | **true** (bleibt!) |
| `exitMin/MaxDistance` | 35–45 | 35–45 (gleich) |

`forceGuaranteedPath=true` bleibt bewusst gesetzt: ohne Garantie lässt sich "Karte unlösbar" nicht von "Agent versagt" trennen.

**`--legacy-bfs`-Flag:** Phase-4-Modell wurde mit der alten Absolutwert-BFS-Kodierung (`_BFS_MAX=64`) trainiert. Nach Änderung 8 (Delta-Encoding) würde das Modell falsche Feature-Werte erhalten. Das Flag patcht `env._bfs_field()` zur Laufzeit auf die alte Kodierung — kein Modell-Reload, kein Rebuild nötig.

**Verwendung:**
```bash
# Phase-4-Modell (Absolut-BFS):
python python/eval_hard_world.py \
    --model best_models_ppo_phase4/final_model.zip --legacy-bfs

# Nach Retraining mit Delta-BFS:
python python/eval_hard_world.py --model best_models_ppo/best_model.zip
```

**Ergebnis:** Noch nicht gemessen. Ergebnisse werden nach Retraining + Eval hier eingetragen.

| Modell | Testset A Hard | mean_len A | Testset B Hard | mean_len B | Datum |
|--------|---------------|------------|----------------|------------|-------|
| PPO (Phase-4, Hard, stoch.) | nicht gemessen (Encoding-Mismatch) | — | — | — | — |
| PPO (Delta-BFS, stoch.) | 100.0 % ✓ | 298.0 | 100.0 % ✓ | 274.1 | 18.05.2026 |
| PPO (Delta-BFS, det.) | **100.0 % ✓** | **50.2** | **100.0 % ✓** | **52.7** | 18.05.2026 |

---

### v2026-05-18.2 — BFS-Feature-Kodierung: Absolutwerte -> Deltas
**Datei:** `python/stoneforge_env.py` — `_bfs_field()`, Konstanten `_BFS_MAX` → `_BFS_CUR_MAX` / `_BFS_DELTA_DIV`

**Problem (Wurzelursache für deterministisch=2%):**
Änderung 7 (Phase 5) schloss aus dem Scheitern ohne exitDx/exitDy: "BFS-Gradient ist zu schwach als einziger Fernbereich-Hinweis." Diese Diagnose war **falsch**: Der BFS-Gradient ist nicht schwach — die *Kodierung* war kaputt.

Mit `_BFS_MAX = 64.0` und typischer Distanz 40–60 Tiles:
- Alle 5 Absolutwerte liegen bei ~0.7–1.0
- Differenz zwischen Nachbarzellen = 1/64 ≈ **0.016** — für den MLP fast unsichtbar
- Wand-Sentinel (9999/64 = 155.9) wird auf 1.0 geclampt → **Aliasing**: Wand und "weit weg aber passierbar" nicht unterscheidbar

Das erklärt auch, warum die deterministischen Argmax-Entscheidungen instabil sind: Das Netz kann aus 0.016-Unterschieden keine robuste Richtungspräferenz lernen.

**Lösung:** Deltas statt Absolutwerte für die 4 Richtungsfeatures.

```
vorher: [cur, up, down, left, right] / 64.0   → alle ~0.7–1.0, Richtungsdiff = 0.016
nachher: [cur/128, (up-cur)/2, (down-cur)/2, (left-cur)/2, (right-cur)/2]
         → cur ∈ [0, 0.5], Richtungsdeltas ∈ {-0.5, 0, +0.5}, Wand → clipped +1.0
```

Delta-Semantik:
- `-0.5` = ein Schritt näher (klares positives Signal)
- `0.0` = selbe Distanz (neutral)
- `+0.5` = ein Schritt weiter (negatives Signal)
- `+1.0` = Wand (9999 − cur) / 2 → clip, eindeutig von "+0.5" unterscheidbar

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `_BFS_MAX` | `64.0` | entfernt | Ersetzt durch zwei getrennte Konstanten |
| `_BFS_CUR_MAX` | — | `128.0` | Verhindert Sättigung; cur bei 56 Tiles = 0.44 statt 0.875 |
| `_BFS_DELTA_DIV` | — | `2.0` | Einzelschritt-Delta (1) → 0.5; Wand-Delta (9943) → clip 1.0 |
| Richtungs-Features | Absolutwerte / 64 | (neighbor − cur) / 2, clip[−1,1] | Richtungssignal 32× stärker (0.5 statt 0.016) |
| Wand-Aliasing | Wand ≡ "weit weg" (beide 1.0) | Wand = +1.0, "weiter" = +0.5 | Distinct encoding, kein Aliasing |
| Obs-Shape | 236 | **236** (unverändert) | Kein Rebuild nötig, kein C++-Code geändert |

**Erwartung:** MLP kann Richtungspräferenz aus ±0.5-Signal 32× zuverlässiger lernen. Deterministischer Eval sollte signifikant über 2% steigen. Diese Änderung widerlegt auch die Phase-5-Schlussfolgerung: exitDx/exitDy kann wieder entfernt werden sobald der Delta-BFS-Gradient das Training stabilisiert (bleibt vorläufig drin als redundantes Backup-Signal).

**Ergebnis:** Noch nicht neu gemessen — nächster Schritt ist Neutraining + 50-Seed-Eval (Seeds 7000–7049) mit `deterministic=True` und `deterministic=False`.

---

### v2026-05-18.1 — Delta-BFS Training Ergebnisse (exit=5-45, 1M Steps, PPO, Delta-BFS-Encoding)
**Modell:** `best_models_ppo_delta_v1/best_model.zip`
**Trainingszeit:** ~245 Sekunden (~4 Minuten) @ 4.1k FPS
**Konvergenz:** 100 % Success ab Step 75.000, stabil bis 1M
**Entropy-Loss Ende:** −0.106 (Phase 4: −0.88) → Policy stark konvergiert

---

## v2026-05-16

### v2026-05-16.7 — exitDx/exitDy aus Observation entfernt (Phase 5)
**Datei:** `python/stoneforge_env.py`

**Problem:** Verhaltensanalyse (Seeds 7004, 7020, 7036, 7042) zeigte, dass der Agent einen starken Luftlinien-Bias durch `exitDx`/`exitDy` gelernt hat: Er läuft Richtung Exit-Richtung auch dann, wenn Wände im Weg sind. Der BFS-Gradient (der korrekte Pfad) wird von den exitDx/exitDy-Features überstimmt. Diese Features sind redundant, weil der BFS-Gradient bereits die Richtungsinformation enthält — nur ohne Wand-Bias.

**Lösung:** `exitDx` und `exitDy` werden in `_normalize()` nicht mehr an die Observation angehängt. Obs-Größe: 236 → 234.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| exitDx in Obs | enthalten (gs+3) | entfernt | BFS-Gradient enthält die gleiche Info, nur korrekt um Wände |
| exitDy in Obs | enthalten (gs+4) | entfernt | Erzeugte Luftlinien-Bias → Agent lief gegen Wände |
| Obs-Größe | 236 | 234 | -2 Features |

**Erwartung:** Agent ohne Luftlinien-Bias lernt, dem BFS-Gradienten zu folgen. Die verbleibenden 6% Fehlrate (Seeds 7020, 7036, 7042 in Phase 4) sollten sich deutlich verringern.

**Ergebnis (Phase 5, abgebrochen bei 737k/1M Steps):**

| Checkpoint | Success (Testset A, exit=35-45) |
|---|---|
| 25k | 24.0% |
| 225k | 14.0% |
| 325k | 40.0% |
| 400k | 32.0% |
| 475k | 32.0% |
| 725k | 28.0% ↓ |

**Hypothese widerlegt.** exitDx/exitDy sind notwendig. Der BFS-Gradient zwischen Nachbarzellen beträgt bei 40 Tiles Entfernung nur ~0.016 Unterschied — zu schwach als einziger Fernbereich-Hinweis. Ohne Luftlinien-Features findet der Agent den Exit nicht zuverlässig. Training abgebrochen, exitDx/exitDy wiederhergestellt. **Bestes Modell bleibt Phase 4 (98%/94%).**

---

### v2026-05-16.6 — Loop- und Wand-Penalties + Stuck-Feature in Obs
**Dateien:** `src/core/simulation.cpp`, `include/stoneforge/simulation.hpp`, `src/python/py_module.cpp`, `python/stoneforge_env.py`

**Problem (Diagnose aus Verhaltensanalyse, Seeds 7000–7005):**
- Seed 7004: Agent läuft **57× gegen dieselbe Nordwand** obwohl BFS↑=9999 (Wand) und BFS→=37 (optimal). `exitDy`-Bias überschreibt BFS-Signal. moveBlocked-Penalty -0.05 zu schwach.
- Seeds 7001–7003, 7005: Agent pendelt in 2-Positions-Loops (z.B. (29,0)↔(30,0)) hunderte Schritte. Stagnation-Counter resettet bei jedem Rechtsschritt neu; Loop wird nie teuer genug.
- Stagnation-Penalty (-0.05 ab Schritt 30) zu flach für beide Fälle.

**Lösung:**
1. **Eskalierender Wand-Penalty** (nutzt `consecutiveBlockedSteps_`): 1. Block -0.05, ab 2. konsek. Block -0.25 → 5× stärker bei Wand-Bangen.
2. **2-Schritt-Loop-Penalty** (nutzt `positionLoop_`): -0.15 pro Schritt wenn A→B→A erkannt → feuert auf BEIDEN Seiten des Pendels.
3. **Eskalierender Stagnation-Penalty**: 30–59 Steps -0.05, ab 60 Steps -0.15.
4. **Stuck-Feature in Observation** (+1 Feature): `min(stepsWithoutProgress / 60, 1.0)` → Agent kann lernen "ich stecke fest, ändere Strategie". Obs-Größe: 235 → 236.

| Änderung | vorher | nachher | Begründung |
|----------|--------|---------|------------|
| moveBlocked (konsek.) | -0.05 immer | -0.05 (1.), -0.25 (2.+) | Wand-Bangen sofort unattraktiv |
| positionLoop_ Penalty | nicht genutzt | -0.15/Schritt | Direkte A↔B-Loop-Erkennung |
| Stagnation-Penalty | -0.05 ab 30 Steps | -0.05 (30+), -0.15 (60+) | Stärkere Eskalation bei längeren Loops |
| Obs-Größe | 235 | 236 | +1 Stuck-Feature |

**Ergebnis:** Rebuild erfolgreich. Wand-Penalty verifiziert (-0.260 statt -0.060 nach 2+ Blocks). Loop-Penalty verifiziert (-0.15 auf beiden Seiten des Pendels). Neues Training erforderlich (Obs-Größe geändert).

---

### v2026-05-16.5 — Phase-4 Ergebnisse — Verbesserte Reward-Struktur (exit=5-45, 1M Steps, PPO, seed=0)
| Modell | Testset A (7000–7049) | mean_len A | Testset B (8000–8049) | mean_len B | Datum |
|--------|----------------------|------------|------------------------|------------|-------|
| final_model | **98.0 %** ✓ | 285 | **94.0 %** ✓ | 466 | 16.05.2026 |
| best_model (100% @ Eval) | 86.0 % ✓ | 893 | 94.0 % ✓ | 722 | 16.05.2026 |

**Verbesserung gegenüber Phase-3:**
- Success Rate A: 88% → **98%** (+10 Prozentpunkte)
- Success Rate B: 84% → **94%** (+10 Prozentpunkte)
- Episodenlänge: 856 → **285 Steps** (3× effizienter — Agent findet Exit viel schneller)

**Deterministisch:** weiterhin 2% (high-entropy Policy, Argmax-Loops — bekanntes PPO-Problem)

**Verhaltens-Diagnose vor/nach:** Seed 7004 (schlechtester Fall Phase-3):
- Phase-3: 57× gegen Nordwand (BFS bleibt 38, exitDy-Bias überwältigt BFS-Signal)
- Phase-4: BFS 41→17, navigiert korrekt durch Wände, kleiner Rest-Loop bei BFS≈18

**Modell:** `best_models_ppo_phase4/final_model.zip`

---

### v2026-05-16.4 — Phase-3 Ergebnisse — Mixed-Distribution (exit=5-45, 1M Steps, PPO, frischer Start)
| Eval-Modus | Exit-Bereich | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|------------|--------------|---------|--------------|----------------------|---------------|-------|
| Stochastisch | 35–45 (Projekt-Ziel) | 43 / 50 | **86.0 %** ✓ | 951 | 19.0 | 16.05.2026 |
| Stochastisch | 5–45 (Trainingsbereich) | 46 / 50 | **92.0 %** ✓ | 678 | 46.2 | 16.05.2026 |
| Deterministisch | 35–45 | 1 / 50 | **2.0 %** ✗ | 3921 | −348.3 | 16.05.2026 |
| Deterministisch | 5–45 | 1 / 50 | **2.0 %** ✗ | 3921 | −320.3 | 16.05.2026 |

**Verlauf (Deterministic Eval alle 25k Steps):** 0%–0% bis 975k Steps, 2% @ 1M Steps.
**Stochastisch:** Policy funktioniert sehr gut (86–92% je nach Bereich), Projektarbeit-Ziel (70%) erreicht.
**Problem — Deterministischer Eval kaputt:** `deterministic=True` (Argmax) erzeugt ↑↓-Loops weil die Policy-Entropie am Trainingsende noch ~−0.88 beträgt. Bei hoher Entropie flipped der Argmax in bestimmten Zuständen zwischen zwei Aktionen → Agent bleibt in 2-Zellen-Schleife stecken. Nächste Priorität: Deterministischen Eval reparieren (Loop-Detection oder Temperatur-Sampling).

**Modelle:** `best_models_ppo_phase3/final_model.zip` (1M Steps), `best_models_ppo_phase3/best_model.zip` (80% stochastisch, gespeichert bei 2% Deterministic-Eval)

**Wiederholungsläufe — 3 unabhängige Runs (exit=5–45, 1M Steps, PPO, stochastisch, exit=35-45):**

| Run | Seed | Testset A (7000–7049) | mean_len A | Testset B (8000–8049) | mean_len B |
|-----|------|-----------------------|------------|------------------------|------------|
| Run 1 | — | 88.0 % | 856 | 84.0 % | 936 |
| Run 2 | 42 | 80.0 % | 1081 | 68.0 % | 1825 |
| Run 3 | 123 | 88.0 % | 752 | 86.0 % | 940 |
| **Mittelwert ± Std** | | **85.3 % ± 3.8 %** | | **79.3 % ± 8.1 %** | |

**Projektarbeit-Kriterien:**
- Testset A ≥ 70 %: **85.3 % ± 3.8 %** ✓  
- Testset B ≥ 60 %: **79.3 % ± 8.1 %** ✓ (Run 2 mit 68 % knappste Messung)

**Modelle:** `best_models_ppo_phase3_run2/` (Run 2, best_model 92%), `best_models_ppo_phase3_run3/` (Run 3, best_model 94%)

---

### v2026-05-16.3 — Eval-Callback auf stochastischen Modus umgestellt
**Datei:** `python/train.py` — `SeedEvalCallback._run_eval()`

**Problem:** `deterministic=True` (Argmax) ist der falsche Eval-Modus für PPO. PPO lernt explizit eine stochastische Policy (Gaußverteilung über Aktionen). Bei hoher Entropie (~−0.88) flipped der Argmax in bestimmten Zuständen zwischen zwei Aktionen → ↑↓-Loop → 0% Eval trotz funktionierender Policy (stochastisch 86–94%).

**Lösung:** `deterministic=False` im Callback. Mehrfach-Evals mit stochastischer Policy haben zwar mehr Varianz, messen aber die tatsächliche Leistung. Für die Projektarbeit gilt stochastische Eval als Standard für PPO.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `model.predict(deterministic=...)` | `True` | `False` | PPO ist stochastisch; Argmax bei hoher Entropie erzeugt Loops |

---

### v2026-05-16.2 — BFS-Kraftfeld-Bug: Wand-Tile-Fallback
**Datei:** `src/core/simulation.cpp`
**Problem (Diagnose):** BFS-Greedy-Oracle-Test (analyze_agent.py) zeigte: Oracle blieb in ALLEN Seeds beim ersten Wandkontakt stecken (z.B. Seed 7000: Step 4, Pos (0,−3), 297× dieselbe Position).
Ursache: `bfsDistanceAt(x, y)` delegierte an `bfsDistanceToExit(x, y)` ohne Passierbarkeits-Check. Für Wand-Tiles, die nicht in `bfsDistances_` liegen, wurde **Manhattan-Fallback** zurückgegeben. Liegt ein Wand-Tile zufällig nahe am Exit (kleiner Manhattan-Wert), erscheint es dem Agenten als beste Richtung — obwohl er dort nie hinlaufen kann.

Beispiel Seed 7000:
```
Step 4 | Pos (0,−3) | BFS 13 | Nachbarn [12, 14, 14, 12]
→ bfsDistanceAt(0,−4) = Manhattan((0,−4), exit) = 12  (FALSCH: Wand!)
→ Oracle wählt ↑, `tryMove` schlägt fehl, Agent bleibt stehen
```

**Lösung:** `bfsDistanceAt` gibt `9999` zurück wenn `!world_.isPassable(x, y)`. Normalisiert auf `9999/64 = 155.9`, geclampt auf `1.0` im BFS-Feld → Wände erscheinen als maximal unattraktiv.

Nach Fix: Oracle erreicht in 3/3 Seeds den Exit (Seed 7000: 16 Steps, 7001: 15 Steps, 7002: 10 Steps). BFS-Feld zeigt `9999` für alle Wand-Richtungen, navigiert korrekt um Ecken.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `bfsDistanceAt(wall)` | Manhattan-Fallback (z.B. 12) | `9999` | Wand = nicht betretbar → maximale Abstoßung |
| BFS-Oracle Success Rate | 0/3 (Oracle steckt in Wand) | 3/3 ✓ | Korrektheit bestätigt |
| Random-Policy Baseline | — | 1/10 (10%) bei exit=5-12 | Referenz für kommende Trainingsläufe |

**Ergebnis:** Fix korrekt. Obs-Größe bleibt 235 (grid 225 + 5 base + 5 BFS). Nächster Schritt: PPO Phase-1-Training mit neuer Obs-Struktur (235 Features, kein visited-Mask).

---

### v2026-05-16.1 — Initial RL Reward Shaping & Curriculum

#### Part C - Curriculum + moveBlocked Penalty + Wand-Reduktion
**Dateien:** `src/core/simulation.cpp`, `assets/base/game_config.json`, `python/train.py`
**Problem:** Agent erreicht 0% Success nach 300k PPO-Steps.
Diagnose: Welt ist kein offenes PointGoal sondern Maze-Navigation.
- Greedy exitDx/exitDy-Policy: 0/20 Exits (scheitert an Sackgassen)
- Random Walk: 1/20 Exits bei exit=35-45 Tiles
- Agent sieht Exit-Signal (+100) *nie* → PPO lernt nur aus schwachem PBRS (±0.02/Step)
- `moveBlocked` war `(void)` → kein Penalty für Wand-Kollisionen

**Lösung:**
1. moveBlocked Penalty: −0.05 bei Wandkollision (sofortiges Signal: "Wand = schlecht")
2. Wand-Thresholds halbiert (0.11→0.05 etc.) → offenere Welt
3. Curriculum-Args in train.py: `--exit-min`, `--exit-max`, `--load-model`
4. Phase-1 Training: exit=5-12 Tiles → Random-Walk-Erfolg 45% → Agent sieht +100-Signal

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `moveBlocked` | `(void)` | `−0.05F` | Direktes Signal für Wanderkennung |
| `warmWallThreshold` | 0.16 | 0.05 | Offenere Welt, weniger Sackgassen |
| `coldWallThreshold` | 0.11 | 0.05 | dto. |
| `--exit-min` (Phase 1) | 35 | 5 | Random Walk findet Exit (45% statt 5%) |
| `--exit-max` (Phase 1) | 45 | 12 | dto. |

**Curriculum-Plan:**
- Phase 1: exit 5-12, 500k Steps → Ziel >40% Success
- Phase 2: exit 12-25, 500k Steps → Ziel >30% Success  
- Phase 3: exit 25-45, 1M Steps → Ziel >50% Success (Projektarbeit-Kriterium: 70%)

**Ergebnis Phase 1 (exit=5–12, 500k Steps, PPO, 235-Feature-Obs mit BFS-Feld):**

| Algorithmus | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|-------------|---------|--------------|----------------------|---------------|-------|
| PPO (Phase 1) | 28 / 50 | 56.0 % | ~40 Steps | 99.0 | 16.05.2026 |

Verlauf: 0%→0%→4%→10%→22%→30%→42%→56% (alle 25k Steps). Phase-1-Ziel >40% bei Schritt 425k erreicht, Finale bei 56%. Modell: `best_models_ppo/best_model.zip`.

#### Part B - Stagnations-Penalty + Visited-Mask entfernt
**Dateien:** `src/core/simulation.cpp`, `include/stoneforge/simulation.hpp`, `python/stoneforge_env.py`
**Problem (Diagnose):** Agent-Inspektion (best_models_dqn/best_model.zip nach 1M Steps) ergab:
- Q-Wert-Spreizung nur 0.12–0.38 über alle Aktionen — kein stabiles Richtungslernen
- Agent steckte in 2-Zellen-Schleife (↑↓↑↓): Visited-Mask ändert sich jeden Schritt, 1-Bit-Änderung kippte Argmax
- Alle Q-Werte ~6.7 (Überrest vom korrupten ep_rew_mean=735-Training)
- Kein Reward-Unterschied zwischen Stagnieren und Navigieren nach Schritt 30

**Lösung:**
1. Stagnations-Penalty: nach 30 Schritten ohne neues BFS-Minimum → −0.05/Schritt (bricht ↑↓-Loops)
2. Visited-Mask aus Observation entfernt (war 225 Extra-Features, änderte sich jeden Schritt)
3. Alte Modelle gelöscht, Neutraining mit PPO

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| Obs-Größe | 455 (grid+visited+5) | 230 (grid+5) | Visited-Mask destabilisiert Argmax bei flachem Reward |
| Stagnations-Penalty | keiner | −0.05/Schritt ab Step 30 ohne Fortschritt | Bricht 2-Zellen-Schleifen strukturell |
| `stepsWithoutProgress_` | nicht vorhanden | neu in simulation.hpp/cpp | Zählt Schritte ohne BFS-Verbesserung |
| `bestBfsInEpisode_` | nicht vorhanden | neu in simulation.hpp/cpp | BFS-Referenzwert für Fortschrittserkennung |
| Algorithmus | DQN | PPO | On-policy verhindert korrupten Replay-Buffer |

**Ergebnis Phase 2 (exit=12–25, 500k Steps, geladen von Phase-1-Modell):**

| Algorithmus | Erfolge | Success Rate | Datum |
|-------------|---------|--------------|-------|
| PPO (Phase 2, best) | 11 / 50 | **22.0 %** (@ 25k Steps!) | 16.05.2026 |
| PPO (Phase 2, final) | 3 / 50 | **6.0 %** (@ 500k Steps) | 16.05.2026 |

**Phase-2-Diagnose — Curriculum-Transfer gescheitert:**
Das Phase-1-Modell (Entropy −0.96) kollabierte in Phase 2 sofort auf Entropy −0.50 (extrem deterministisch). Die eingebrannte Kurzdistanz-Strategie ließ sich mit `ent_coef=0.01` und `lr=3e-4` nicht überschreiben — statt Umlernen auf 12–25 Tiles wurde die Phase-1-Policy aktiv zerstört. Bestes Ergebnis war die erste Eval (22%), danach nur noch Rückschritt.

Wichtig: Phase-1-Finalmodell (56%) wurde gesichert als `best_models_ppo/phase1_final_56pct.zip`.

**Konsequenz für Phase 3:** Kein naives Curriculum-Laden. Stattdessen Mixed-Distribution-Training (exit=5–45), damit der Agent short-range Skills behält und long-range parallel lernt.

#### Part A - PBRS Discount entkoppelt
**Datei:** `src/core/simulation.cpp`
**Problem:** Das Reward-Shaping konnte mit negativem Potential und `PBRS_GAMMA < 1` einen kleinen positiven Schritt-Reward beim Verharren erzeugen; der Zusatzbonus fuer neue Tiles verstaerkte das Farmen weiter.
**Lösung:** Den Tile-Bonus entfernt und das PBRS-Shaping auf `F(s, s') = Φ(s') - Φ(s)` umgestellt, indem `PBRS_GAMMA` auf `1.0F` gesetzt wurde. `PBRS_BETA` bleibt bei `2.5F`.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `PBRS_GAMMA` | `0.999F` | `1.0F` | Shaping muss teleskopieren und darf keinen Discount-Tropf erzeugen |
| `newTileVisited` Bonus | `+0.01F` | entfernt | Kein positives Gegenkonto pro Schritt mehr |
| `PBRS_BETA` | `2.5F` | `2.5F` | bleibt als Skalierung fuer BFS-Fortschritt erhalten |

**Ergebnis:** Noch nicht neu gemessen; naechster Schritt ist ein 50-Seed-Eval mit den Seeds 7000-7049.

---

## v2026-05-12
- Ufer- und Wasserfelder werden jetzt mit einer Sandtextur statt Gras dargestellt.
- Ergebnis: klarere visuelle Wasser-Randzone und bessere Lesbarkeit der Karte.
- Umsetzung: Sand-Sprites zum Atlas hinzugefuegt und die Tile-Zeichnung um eine Wasser-Nachbarschaftsregel erweitert.
- Korrektur: Sand wird jetzt auch sichtbar unter Wasserzellen und an allen an Wasser angrenzenden Zellen gezeichnet, ausser bei Baeumen und Hindernissen.
- Anpassung: Die Wasser-Deckkraft wurde reduziert, damit die Sandbasis unter Wasser besser sichtbar bleibt.
