# Verbesserungsplan — Stoneforge RL (LSTM-Curriculum ohne BFS)

> **Zielgruppe dieses Dokuments:** Ein LLM-Agent (z. B. Claude Code), der die
> Verbesserungen eigenständig umsetzen soll. Alle Aufgaben sind mit Datei,
> konkreter Änderung, Begründung und Verifikation beschrieben.
>
> **Stand:** 11.06.2026. Bestes Modell: `ppo_lstm_curriculum` (Ablation C,
> LSTM ohne BFS): 86 % stochastisch / 36 % deterministisch auf Testset A
> (Seeds 7000–7049), 68 % / 18 % auf Holdout B (8000–8049).
> Ziel der Projektarbeit: ≥ 70 % auf A, ≥ 60 % auf B, je 3 Läufe mit Mittelwert ± Std.

---

## Kontext (vor Beginn lesen)

- `CHANGELOG.md` — Pflichtlektüre, enthält alle bisherigen Experimente und Negativ-Ergebnisse.
- `scripts/train_curriculum.py` — Haupttrainings-Skript (RecurrentPPO, 3 Phasen).
- `python/stoneforge_env.py` — Gym-Env (Obs 231-dim: 15×15-Grid + Kompass + step_frac).
- `src/core/simulation.cpp` → `computeReward()` (Zeile ~1454) — Reward inkl. PBRS auf BFS-Distanz.
- **Dokumentationspflicht:** Jede Änderung und jede Messung MUSS in `CHANGELOG.md`
  (Format siehe `.claude/CLAUDE.md`) dokumentiert werden.
- **Nicht wiederholen (bereits gescheitert):** `n_epochs=1` (0 % SR, Value-Fn
  konvergiert nicht), zeitbasiertes Curriculum (Reward-Kollaps),
  Stagnation-Penalty (bestraft legitime Umgehungsmanöver).

---

## P0 — Methodische Fehler (vor jedem weiteren Training fixen)

### P0.1 — Test-Set-Leakage: Eval-Callback nutzt die Test-Seeds 7000–7049

**Datei:** `scripts/train_curriculum.py` (Zeile 35: `EVAL_SEEDS = list(range(7000, 7050))`)

**Problem:** Der `CurriculumEvalCallback` selektiert das beste Modell und steuert
die Phasenübergänge anhand der Seeds 7000–7049 — das ist aber **Testset A der
Projektarbeit**. Modellselektion auf dem Testset ist Data Leakage; die
berichteten 86 %/36 % sind dadurch optimistisch verzerrt und in der Arbeit angreifbar.

**Lösung:**
1. Validierungs-Seeds einführen: `VAL_SEEDS = list(range(6000, 6050))`.
2. Callback ausschließlich auf `VAL_SEEDS` evaluieren (Best-Model-Auswahl + Phasen-Stopp).
3. Seeds 7000–7049 (Testset A) und 8000–8049 (Holdout B) **nur noch** in der
   finalen Evaluation (`scripts/eval_comparison.py`) verwenden.

**Verifikation:** `grep -n "7000" scripts/train_curriculum.py` darf keinen Treffer
mehr im Callback-Pfad liefern.

### P0.2 — Lösbarkeit der Welten prüfen (`force_guaranteed_path=False`)

**Datei:** `python/stoneforge_env.py` (Zeile ~98)

**Problem:** Training UND Eval laufen mit `force_guaranteed_path=False`. Wenn ein
Teil der Seeds gar keinen Pfad zum Exit hat, ist die maximal erreichbare SR < 100 %
— die Zielkriterien (70 %/60 %) wären dann gegen eine unbekannte Obergrenze gemessen,
und unlösbare Episoden verrauschen das Training.

**Lösung:**
1. Einmalig per BFS (C++-Binding: `current_bfs_distance_to_exit()` direkt nach
   Reset) für die Seeds 6000–6049, 7000–7049, 8000–8049 und je 1000 zufällige
   Trainings-Seeds pro Phase prüfen, wie viele Welten lösbar sind.
2. Ergebnis als „Oracle-Ceiling" im `CHANGELOG.md` dokumentieren.
3. Falls < 100 % lösbar: entweder `force_guaranteed_path=True` setzen (Rebuild nötig!)
   oder unlösbare Seeds beim Env-Reset neu würfeln — Entscheidung dokumentieren.

### P0.3 — 3-Läufe-Protokoll automatisieren

**Problem:** Zielkriterium verlangt ≥ 3 Trainingsläufe pro Konfiguration mit
Mittelwert ± Standardabweichung. Bisher gibt es nur Einzelläufe.

**Lösung:** Wrapper-Skript `scripts/run_experiment.sh` (oder Python), das
`train_curriculum.py` mit `--seed 0/1/2` und getrennten `--save-dir`-Suffixen
(`_s0`, `_s1`, `_s2`) nacheinander startet und am Ende `eval_comparison.py`
über alle drei Modelle laufen lässt und Mittelwert ± Std ausgibt.

---

## P1 — Lernverbesserungen mit hohem erwartetem Effekt

### P1.1 — `batch_size=16` ist viel zu klein

**Datei:** `scripts/train_curriculum.py` (`RPPO_KWARGS`)

**Problem:** Rollout = `n_steps=512 × n_envs=16` = 8192 Transitions, aber
`batch_size=16` → 512 Minibatches/Epoche × 4 Epochen = extrem verrauschte
Gradienten und langsames Training. Für RecurrentPPO sind 128–512 üblich
(Sequenzen bleiben dabei intakt).

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `batch_size` | 16 | **256** | Stabilere Gradienten; 32 Minibatches/Epoche |

**Verifikation:** TensorBoard `train/explained_variance` sollte schneller > 0.5
steigen als im v4-Lauf; `train/approx_kl` stabil < 0.03.

### P1.2 — Letzte Aktion + letzter Reward in die Observation (Standard für POMDP-LSTM)

**Datei:** `python/stoneforge_env.py`

**Problem:** Der LSTM muss den Belief-State allein aus Grid-Beobachtungen
rekonstruieren. In der POMDP-Literatur (z. B. R2D2, IMPALA) ist es Standard,
**letzte Aktion (one-hot, 4 dims) und letzten Reward (1 dim, geclippt)** an die
Obs anzuhängen — der LSTM lernt damit deutlich schneller, ob eine Bewegung
blockiert wurde (Aktion gewählt, Position unverändert ⇒ Wand), was den
Det/Stoch-Gap direkt adressiert (Oszillations-Loops an Wänden).

**Lösung:** Obs 231 → 236: `[grid(225) | hp, energy, inv, exitDx, exitDy, step_frac |
last_action_onehot(4) | clip(last_reward, −1, 1)]`. Achtung: neue Modellgeneration,
alte 231er-Modelle bleiben kompatibel über alte Env-Version — Obs-Shape-Änderung
im `CHANGELOG.md` und in der Memory-Notiz zur Obs-Shape-Evolution dokumentieren.

### P1.3 — Entropie-Annealing gegen den Det/Stoch-Gap

**Datei:** `scripts/train_curriculum.py`

**Problem:** Konstantes `ent_coef=0.01` hält die Policy bis zum Ende stochastisch
— der Agent „verinnerlicht" das Sampling und hat keine stabile Greedy-Strategie
(Kern des 86 %-stoch vs. 36 %-det Gaps).

**Lösung:** `ent_coef` als Schedule: Phase 1–2 bei 0.01 belassen, in Phase 3
linear von 0.01 → 0.001 über die ersten 500k Steps abklingen lassen
(SB3: callable übergeben oder eigenen Callback schreiben, der
`model.ent_coef` setzt). Erwartung: deterministische SR steigt, stochastische
bleibt ≈ gleich.

**Verifikation:** Det-SR auf VAL_SEEDS am Phasenende mit/ohne Annealing vergleichen
(Ablation, je 1 Lauf reicht zur Vorauswahl).

### P1.4 — Curriculum-Ziele anheben: Phase 1/2 stoppen zu früh

**Datei:** `scripts/train_curriculum.py` (`PHASES`)

**Problem:** Phase 1 (exit 5–12) stoppt bereits bei 40 % SR, Phase 2 bei 30 %.
Damit startet Phase 3 mit einer halbgaren Policy. Kurze Distanzen sind fast
trivial — dort sind 85–90 % erreichbar und ein viel besseres Fundament.

| Phase | target_sr vorher | nachher |
|-------|------------------|---------|
| 1 (5–12)  | 0.40 | **0.85** |
| 2 (12–25) | 0.30 | **0.70** |
| 3 (25–45) | 0.70 | 0.70 (unverändert) |

Zusätzlich: Stopp erst, wenn das Ziel in **2 aufeinanderfolgenden Evals**
erreicht wird (eine einzelne Eval kann Ausreißer sein).

### P1.5 — Prioritized Level Replay statt Erfolgs-Seed-Replay (Swarm invertieren)

**Datei:** `python/stoneforge_env.py` (`SwarmSeedPool`)

**Problem:** Der aktuelle Pool wiederholt mit 30 % Wahrscheinlichkeit **bereits
gelöste** Seeds — das verstärkt, was schon funktioniert, und verschiebt die
Trainingsverteilung Richtung „leicht". Die Literatur (Prioritized Level Replay,
Jiang et al. 2021) zeigt: man soll Levels mit hohem **Lernpotenzial** wiederholen
— also knapp gescheiterte, nicht gelöste.

**Lösung (einfache Variante):** Pool-Semantik umdrehen — Seeds aufnehmen, bei
denen die Episode per Early-Stop/Timeout scheiterte, und gelöste Seeds aus dem
Pool entfernen. `swarm_prob=0.3` beibehalten. Als Ablation gegen die aktuelle
Variante fahren (1 Lauf), Gewinner in die finale Konfiguration übernehmen.

### P1.6 — Asymmetrischer Critic: BFS-Features nur für die Value-Funktion

**Dateien:** `scripts/train_curriculum.py`, `python/stoneforge_env.py`, ggf. eigener Policy-Code

**Idee:** Die Forschungsfrage verbietet BFS nur in der **Actor-Observation**.
Der Critic existiert nur im Training — ihm privilegierte Information (BFS-Distanz)
zu geben ist methodisch sauber („Asymmetric Actor-Critic", Pinto et al. 2017)
und reduziert die Value-Varianz massiv. PBRS im Reward nutzt ohnehin schon
BFS-Distanz, das ist konsistent.

**Aufwand:** mittel (eigene Policy-Klasse, die Critic-Input erweitert).
Nur angehen, wenn P1.1–P1.5 die 70 %-det-Marke noch nicht erreichen.

---

## P2 — Konsistenz / kleinere Fixes

### P2.1 — PBRS-Gamma an RL-Gamma angleichen

**Datei:** `src/core/simulation.cpp` (`computeReward`, `PBRS_GAMMA = 1.0F`)

Die Policy-Invarianz-Garantie von PBRS (Ng et al. 1999) gilt nur für
`F(s,s') = γ·Φ(s') − Φ(s)` mit **demselben γ wie im RL-Algorithmus** (0.999).
`PBRS_GAMMA` auf 0.999 setzen (Rebuild nötig). Effekt ist klein, aber für die
Projektarbeit theoretisch sauber — im Text als Korrektheits-Fix erwähnen.

### P2.2 — Toter Code: Stuck-Penalty-Schwelle 300 wird nie erreicht

**Datei:** `python/stoneforge_env.py` (Zeile ~200)

Early-Stop truncated nach 256 Schritten ohne positiven Reward — eine Tile 300-mal
zu besuchen, bevor das greift, ist praktisch unmöglich. Entweder Schwelle auf
~25 senken (und Wirkung in 1 Lauf prüfen) oder den Block entfernen und im
Changelog als toten Code dokumentieren.

### P2.3 — SubprocVecEnv statt DummyVecEnv prüfen

**Datei:** `scripts/train_curriculum.py`

16 Envs sequenziell in einem Prozess (`DummyVecEnv`). Da der Core in C++ läuft,
ist der Step billig — aber bei 512-LSTM lohnt ein kurzer Benchmark
(`SubprocVecEnv` vs. `DummyVecEnv`, je 50k Steps, FPS vergleichen).
Nur wechseln, wenn ≥ 1.3× schneller; StreamWrapper/ws_map funktioniert
in Subprozessen nicht in-process — dann Live-Map nur via `--no-live-map`.

### P2.4 — Ablation D (CNN + Visited Mask) abschließen

`scripts/train_cnn.py` existiert, Ergebnis steht aus. Nach den P0/P1-Fixes
einen Lauf mit identischem Protokoll fahren, damit die Ablationsmatrix
A/B/C/D in der Projektarbeit vollständig ist.

---

## Evaluationsprotokoll (für ALLE Experimente verbindlich)

1. **Training/Selektion:** nur VAL_SEEDS 6000–6049 (nach P0.1).
2. **Finale Messung:** Testset A 7000–7049 und Holdout B 8000–8049, je
   deterministisch UND stochastisch (τ=0.2) berichten.
3. **Pro Konfiguration 3 Läufe** (Seeds 0/1/2) → Mittelwert ± Std.
4. Jedes Ergebnis sofort ins `CHANGELOG.md` (Tabellenformat aus `.claude/CLAUDE.md`).
5. Det/Stoch-Gap immer mit ausweisen — er ist Teil der POMDP-Argumentation
   der Projektarbeit (siehe Changelog-Abschnitt „Wissenschaftliche Einordnung").

## Empfohlene Reihenfolge

1. P0.1 (Leakage-Fix) + P0.2 (Lösbarkeits-Check) — ohne die sind alle weiteren Messungen wertlos.
2. P1.1 (batch_size) + P1.4 (Curriculum-Ziele) + P1.2 (last action/reward in Obs) → ein Lauf „v5".
3. P1.3 (Entropie-Annealing) als Ablation auf v5.
4. P1.5 (PLR-Swarm) als Ablation.
5. Beste Konfiguration → 3-Läufe-Protokoll (P0.3) → finale Zahlen für die Arbeit.
6. P2.x parallel/danach, P1.6 nur falls 70 % det noch nicht erreicht.
