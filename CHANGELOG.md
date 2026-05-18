# Changelog

## v2026-05-18

#### Änderung 10 — Eval-Distribution im Callback fix auf 35–45

**Datei:** `python/train.py` — `main()`, Zeilen `eval_exit_min` / `eval_exit_max`

**Problem:** `SeedEvalCallback` bekam `eval_exit_min=args.exit_min, eval_exit_max=args.exit_max`. Bei Mixed-Training (`--exit-min 5 --exit-max 45`) wurde `best_model.zip` also nach Performance auf dem **Trainingsbereich 5–45** gespeichert — nicht nach dem Projektkriterium 35–45. Das Modell wurde möglicherweise auf einer leichteren Verteilung als "best" selektiert.

**Lösung:** Eval-Env im Callback fix auf `eval_exit_min=35, eval_exit_max=45`.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `eval_exit_min` | `args.exit_min` (z.B. 5) | `35` | Selektion nach Projektkriterium |
| `eval_exit_max` | `args.exit_max` (z.B. 45) | `45` | dto. |

---

#### Änderung 9 — Hard-World Eval Infrastruktur

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

#### Änderung 8 — BFS-Feature-Kodierung: Absolutwerte → Deltas

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

## v2026-05-18

### Delta-BFS Training — Ergebnisse (exit=5–45, 1M Steps, PPO, Delta-BFS-Encoding)

**Modell:** `best_models_ppo_delta_v1/best_model.zip`
**Trainingszeit:** ~245 Sekunden (~4 Minuten) @ 4.1k FPS
**Konvergenz:** 100 % Success ab Step 75.000, stabil bis 1M
**Entropy-Loss Ende:** −0.106 (Phase 4: −0.88) → Policy stark konvergiert

#### Standard-Welt Eval (game_config.json, exit=35–45, Seeds 7000–7049 / 8000–8049)

| Eval-Modus | Testset | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|------------|---------|---------|--------------|----------------------|---------------|-------|
| Stochastisch | A (7000–7049) | 50 / 50 | **100.0 %** ✓ | 467.9 | 57.18 | 18.05.2026 |
| Stochastisch | B (8000–8049) | 50 / 50 | **100.0 %** ✓ | 480.9 | 56.97 | 18.05.2026 |
| Deterministisch | A (7000–7049) | 21 / 50 | **42.0 %** | 2353.1 | −682.24 | 18.05.2026 |
| Deterministisch | B (8000–8049) | 19 / 50 | **38.0 %** | 2512.5 | −735.63 | 18.05.2026 |

#### Hard-World Eval (coldWallThreshold=0.15, enableCellularSmoothing=true, exit=35–45)

| Eval-Modus | Testset | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |
|------------|---------|---------|--------------|----------------------|---------------|-------|
| Stochastisch | A (7000–7049) | 50 / 50 | **100.0 %** ✓ | 298.0 | 82.66 | 18.05.2026 |
| Stochastisch | B (8000–8049) | 50 / 50 | **100.0 %** ✓ | 274.1 | 84.89 | 18.05.2026 |
| Deterministisch | A (7000–7049) | 50 / 50 | **100.0 %** ✓ | 50.2 | 100.47 | 18.05.2026 |
| Deterministisch | B (8000–8049) | 50 / 50 | **100.0 %** ✓ | 52.7 | 100.30 | 18.05.2026 |

**Vergleich Phase 4 → Delta-BFS:**

| Metrik | Phase 4 | Delta-BFS |
|--------|---------|-----------|
| Stoch. Success A | 98.0 % | **100.0 %** |
| Stoch. Success B | 94.0 % | **100.0 %** |
| Det. Success A | 2.0 % | **42.0 %** (Standard) / **100.0 %** (Hard) |
| Stoch. mean_len A | 285 | 467.9 (Standard) / 298.0 (Hard) |
| Det. mean_len A | 3921 | 2353 (Standard) / **50.2** (Hard) |
| Entropy-Loss | −0.88 | **−0.106** |

**Kernbefund — Hard-World deterministisch besser als Standard-Welt:**
Der Argmax ist auf der harten Welt (Cellular Smoothing, dichte Wände) stabiler als auf dem offenen Feld:
- Offene Welt: viele Positionen mit gleicher BFS-Distanz in mehrere Richtungen → Policy unentschlossen → Argmax-Loops → 42 % det.
- Höhlen-Korridor: Wände geben Delta +1.0, genau eine Richtung gibt −0.5 → eindeutiges Signal → Argmax stabil → 100 % det. in ~50 Steps (nahezu optimal)
- mean_len deterministisch Hard = 50 Steps ≈ BFS-Pfadlänge → Agent läuft den optimalen Pfad

Dies widerlegt die Hypothese aus Phase 5 ("BFS-Gradient zu schwach"): Das Problem war die **Absolutwert-Kodierung**, nicht die BFS-Methode selbst.

---

#### Änderung 10 — Eval-Distribution im Callback fix auf 35–45
**Datei:** `python/train.py`
**Problem:** `SeedEvalCallback` bekam `eval_exit_min=args.exit_min, eval_exit_max=args.exit_max`. Bei Mixed-Training (`--exit-min 5 --exit-max 45`) wurde `best_model.zip` nach Performance auf dem **Trainingsbereich 5–45** gespeichert, nicht nach Projektkriterium 35–45.
**Lösung:** Eval-Env im Callback fix auf `eval_exit_min=35, eval_exit_max=45`.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `eval_exit_min` | `args.exit_min` | `35` | Selektion nach Projektkriterium |
| `eval_exit_max` | `args.exit_max` | `45` | dto. |

---

## v2026-05-16

### Phase-3 Ergebnisse — Mixed-Distribution (exit=5–45, 1M Steps, PPO, frischer Start)

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

### Phase-4 Ergebnisse — Verbesserte Reward-Struktur (exit=5–45, 1M Steps, PPO, seed=0)

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

#### Änderung 6 — Loop- und Wand-Penalties + Stuck-Feature in Obs

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

#### Änderung 7 — exitDx/exitDy aus Observation entfernt (Phase 5)

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

#### Änderung 5 — Eval-Callback auf stochastischen Modus umgestellt

**Datei:** `python/train.py` — `SeedEvalCallback._run_eval()`

**Problem:** `deterministic=True` (Argmax) ist der falsche Eval-Modus für PPO. PPO lernt explizit eine stochastische Policy (Gaußverteilung über Aktionen). Bei hoher Entropie (~−0.88) flipped der Argmax in bestimmten Zuständen zwischen zwei Aktionen → ↑↓-Loop → 0% Eval trotz funktionierender Policy (stochastisch 86–94%).

**Lösung:** `deterministic=False` im Callback. Mehrfach-Evals mit stochastischer Policy haben zwar mehr Varianz, messen aber die tatsächliche Leistung. Für die Projektarbeit gilt stochastische Eval als Standard für PPO.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `model.predict(deterministic=...)` | `True` | `False` | PPO ist stochastisch; Argmax bei hoher Entropie erzeugt Loops |

---

#### Änderung 1 - PBRS Discount entkoppelt
**Datei:** `src/core/simulation.cpp`
**Problem:** Das Reward-Shaping konnte mit negativem Potential und `PBRS_GAMMA < 1` einen kleinen positiven Schritt-Reward beim Verharren erzeugen; der Zusatzbonus fuer neue Tiles verstaerkte das Farmen weiter.
**Lösung:** Den Tile-Bonus entfernt und das PBRS-Shaping auf `F(s, s') = Φ(s') - Φ(s)` umgestellt, indem `PBRS_GAMMA` auf `1.0F` gesetzt wurde. `PBRS_BETA` bleibt bei `2.5F`.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `PBRS_GAMMA` | `0.999F` | `1.0F` | Shaping muss teleskopieren und darf keinen Discount-Tropf erzeugen |
| `newTileVisited` Bonus | `+0.01F` | entfernt | Kein positives Gegenkonto pro Schritt mehr |
| `PBRS_BETA` | `2.5F` | `2.5F` | bleibt als Skalierung fuer BFS-Fortschritt erhalten |

**Ergebnis:** Noch nicht neu gemessen; naechster Schritt ist ein 50-Seed-Eval mit den Seeds 7000-7049.

#### Änderung 2 - Stagnations-Penalty + Visited-Mask entfernt
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

#### Änderung 3 — Curriculum + moveBlocked Penalty + Wand-Reduktion
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

#### Änderung 4 — BFS-Kraftfeld-Bug: Wand-Tile-Fallback
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

## v2026-05-12
- Ufer- und Wasserfelder werden jetzt mit einer Sandtextur statt Gras dargestellt.
- Ergebnis: klarere visuelle Wasser-Randzone und bessere Lesbarkeit der Karte.
- Umsetzung: Sand-Sprites zum Atlas hinzugefuegt und die Tile-Zeichnung um eine Wasser-Nachbarschaftsregel erweitert.
- Korrektur: Sand wird jetzt auch sichtbar unter Wasserzellen und an allen an Wasser angrenzenden Zellen gezeichnet, ausser bei Baeumen und Hindernissen.
- Anpassung: Die Wasser-Deckkraft wurde reduziert, damit die Sandbasis unter Wasser besser sichtbar bleibt.
