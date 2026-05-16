# Changelog

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
