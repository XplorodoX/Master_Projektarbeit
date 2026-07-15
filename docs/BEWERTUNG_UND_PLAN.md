# Bewertung: Sind wir auf dem richtigen Weg? + Verbesserungs- & Beschleunigungsplan

*Erstellt am 06.07.2026. Grundlage: Code-Analyse (Env, C++-Core, Trainingsskript), Changelog v2–v10,
**praktische Verifikation am Spiel** (Messungen unten) und externe Recherche.*

---

## 1. Verifikation am Spiel — heute gemessen ✅

Alle Checks direkt gegen den gebauten C++-Core (`build/stoneforge_sim.so`) ausgeführt:

| Check | Ergebnis | Bewertung |
|---|---|---|
| Env-Smoke-Test | Obs (231,), Werte in [0, 1], `Discrete(4)`, step() liefert Reward + `reached_exit`/`bfs_distance` | ✅ funktioniert |
| **Lösbarkeit** Val (6000–6049), Test A (7000–7049), Holdout B (8000–8049) | **150/150 lösbar** | ✅ kein Ceiling-Problem |
| **BFS-Oracle-Agent** (greedy nach `bfs_distance_at_offset`) auf 10 Test-Seeds | **10/10 Erfolg**, Schritte = exakt BFS-Distanz, Return ≈ +101.7 | ✅ Spielmechanik + Reward-Signal arbeiten korrekt; PBRS summiert sich sauber |
| Random-Agent auf 20 Phase-1-Seeds (exit=5–12) | 12/20 (60%), Ø 1240 Schritte | ✅ Nahbereich trivial lösbar — Curriculum-Einstieg passt |
| Bestes Modell `ppo_lstm_curriculum` auf 20 Test-Seeds (7000–7019) | **stochastisch 14/20 (70%), deterministisch 5/20 (25%)** | ✅ Claims (86%/36% auf 50 Seeds) in der Größenordnung bestätigt; 20er-Stichprobe streut |

**Fazit Verifikation: Ja — das, was wir machen, funktioniert mit dem Spiel.**
Die Umgebung ist zu 100% lösbar, das Reward-Signal ist korrekt verdrahtet (Oracle läuft auf dem
kürzesten Pfad und kassiert genau die erwarteten Boni), und der trainierte Agent löst die Aufgabe
reproduzierbar. Das Problem liegt nicht im Spiel — es liegt in Trainings-Effizienz und Det/Stoch-Gap.

### ⚠️ Drei Befunde aus der Verifikation (bitte korrigieren/dokumentieren)

1. **Exit-Distanz ist Luftlinie, nicht BFS.** `World::chooseExitPoint()` (`src/core/world.cpp:328`)
   wählt Kandidaten nach **euklidischer** Distanz 35–45 (`dist2 >= min2 && dist2 <= max2`).
   Die tatsächliche BFS-Pfadlänge auf den Eval-Seeds ist **42–75 Tiles, Ø ≈ 55**.
   Die Doku (`stoneforge_game_description.md`: „35–45 BFS-Tiles") ist falsch.
   → Kein Bug, aber für die Projektarbeit korrekt beschreiben: die Aufgabe ist *schwerer* als dokumentiert.
2. **Wand-Penalty-Revert ist nicht dokumentiert.** Changelog v2026-06-13 sagt „Wand-Penalty entfernt,
   Loop-Penalty −0.15 → −0.05". Commit `16dc8dd` hat beides **bewusst wieder eingebaut**
   (−0.05/−0.25 + −0.15, `simulation.cpp:1481–1488`). Der Revert braucht einen Changelog-Eintrag,
   sonst ist unklar, mit welchem Reward v10 trainiert.
3. **CLAUDE.md-Fallstrick veraltet.** „Manhattan-Distanz im Reward-Shaping … BFS wäre sauber (C++ nötig)"
   — BFS-PBRS ist längst implementiert (`simulation.cpp:1501`, β=2.5, Buffer=80, policy-invariant). Eintrag aktualisieren.

---

## 2. Urteil: War die Richtung richtig?

**Ja, im Kern klar richtig.** Einordnung pro Entscheidung:

| Entscheidung | Bewertung | Begründung |
|---|---|---|
| LSTM (RecurrentPPO) statt MLP | ✅ richtig | Stoneforge ist ein POMDP (15×15-Sicht, Luftlinien-Kompass). Eigener Beleg: MLP det = 0%, LSTM det = 36%. Deckt sich mit der Literatur: Mazes mit Sackgassen brauchen Gedächtnis. |
| Curriculum (Phasen nach Exit-Distanz, leistungsbasiert) | ✅ richtig | Standard-Vorgehen; Wechsel weg vom zeitbasierten Curriculum (Reward-Kollaps) war die richtige Korrektur. |
| PBRS auf BFS-Distanz | ✅ richtig & theoretisch sauber | Potential-based Shaping ist policy-invariant (Ng et al. 1999). In Sokoban bringt Distanz-Shaping ~10× schnellere Konvergenz. Buffer-Fix 20→80 war korrekt diagnostiziert. |
| Val/Test/Holdout-Trennung (6000/7000/8000) | ✅ richtig | Leakage-Fix war methodisch essenziell. |
| Root-Cause-Analyse v7–v9 → v10 (v2-Reverse-Engineering) | ✅ vorbildlich | ent_coef=0.05 als Hauptfaktor, n_steps=256 gegen BPTT-Vanishing-Gradient — durch approx_kl/EV-Metriken belegt. |
| `batch_size=8` | ⚠️ funktioniert, nie isoliert getestet | Komplettes v2-Paket reproduziert; ob batch=8 nötig ist oder nur ent_coef=0.05 zählte, ist offen — und batch=8 kostet den Großteil der Trainingszeit (Abschnitt 4). |
| Det. Phasen-Gating in Phase 1–2 | ❌ strukturell inkonsistent | Mit ent_coef=0.05 ist die Policy fast-uniform → det. SR konstruktionsbedingt ~0% (v10-Logs). Phasen 1–2 enden nie vorzeitig, best_model-Selektion läuft ohne Signal. |

**Ergebnisse im Literaturvergleich:** Bei Zero-Shot-Generalisierung auf prozedurale Mazes erreichen
publizierte Methoden typischerweise **25–53% SR** (teils mit ~350M Steps). Unsere 86% stoch. auf A /
68% auf Holdout B mit nur 2.2M Steps sind **gut** — der Det/Stoch-Gap ist die verbleibende Schwäche,
und die POMDP-These (stochastische Eval legitim) ist extern gestützt.

---

## 3. Gab es sowas schon? — Vergleichbare Projekte (fürs Paper)

- **MiniGrid / BabyAI** (Farama): partiell beobachtbare Grid-Navigation zum Ziel, PPO+LSTM als
  Standard-Baseline — strukturell fast identisch zu unserem Setup.
- **Crafter** (Hafner 2021): prozedural generiertes 2D-Survival-Spiel mit Mining/Crafting/Tech-Tree —
  der akademische Zwilling von Stoneforge. Referenz-Agenten: DreamerV3, IRIS. Ideal zur Motivation der Umgebung.
- **Procgen** (Cobbe et al.): Standard-Benchmark für Generalisierung auf prozeduralen Leveln.
  Kernaussage: Train/Test-Gap ist normal; **Prioritized Level Replay** (Jiang et al., ICML 2021)
  reduziert ihn deutlich (Test-Returns +28–76%). Unser `SwarmSeedPool --plr` ist eine vereinfachte Variante.
- **POPGym / POMDP-Benchmarks**: bestätigen, dass Recurrent-Policies bei maskierter Information nötig
  sind — und dass PPO+Frame-Stacking oft kompetitiv und schneller ist (→ Ablation B3).

---

## 4. Warum das Training so langsam ist (Rechnung)

Pro Rollout: 16 Envs × 256 n_steps = **4.096 Transitions**.
Mit `batch_size=8`, `n_epochs=10`: 4096/8 × 10 = **5.120 Gradient-Updates pro Rollout** —
mehr Optimizer-Schritte als Env-Steps. Die Zeit steckt im Optimizer, nicht in der C++-Simulation
(vgl. v7d mit batch=256: 64 Updates/Rollout → 424 FPS statt 110 FPS).

Zweiter Zeitfresser: Der Eval-Callback läuft alle ~25k Steps 50 Episoden sequenziell,
single-env, bis zu 4.000 Steps — bei niedriger SR mehr Env-Steps pro Eval als das Trainingsintervall.

---

## 5. Plan

### A — Beschleunigung

| # | Maßnahme | Erwartung | Aufwand |
|---|---|---|---|
| A1 | **Batch-Ablation** batch_size ∈ {8, 64, 128}, je ~200k Steps Phase 1; ent_coef=0.05, n_steps=256, 231-dim Obs **unverändert**. Erfolgskriterium: approx_kl > 0.01, EV steigt wie v10 | ~3–5× FPS | ½ Tag |
| A2 | **CPU vs. MPS** messen (10-min-Vergleichslauf). Bei tausenden Mini-Updates frisst MPS-Dispatch-Overhead den Gewinn | bis 2× | 30 min |
| A3 | **Eval entschärfen:** `MAX_EVAL_STEPS` pro Phase (P1: ~500 statt 4000); **stochastische SR als Gating/Selektion in P1–P2** (det zusätzlich loggen, ab P3 det); behebt zugleich das Gating-Problem aus Abschnitt 2 | weniger Totzeit + korrekte Modellselektion | 1 h |
| A4 | **3-Seeds-Pflicht parallel in die Cloud** (Kaggle/VPS; `kaggle_train.ipynb`, `docs/gpu_and_cloud_training.md` existieren) | 3× Kalenderzeit gespart | vorhanden |

### B — Bessere Ergebnisse / Konvergenz

| # | Maßnahme | Begründung |
|---|---|---|
| B1 | **Annealing-Bug fixen:** `EntropyAnnealingCallback` startet bei 0.01, trainiert wird mit 0.05 → abrupter Entropie-Sprung beim P2→P3-Wechsel. Start auf 0.05 (`train_curriculum.py:372`) | kleiner Fix, direkt Det/Stoch-Gap-relevant |
| B2 | **Det/Stoch-Gap:** v10-Plan (Annealing 0.05→0.001 + Phase 4 Greedy) beibehalten; in P3/P4 **beide** SRs loggen → Gap-Kurve als Paper-Diagramm | richtiger Ansatz, nur Messung ergänzen |
| B3 | **Ablation PPO (MLP) + Frame-Stacking (n_stack=8):** laut SB3-Autoren oft kompetitiv zu RecurrentPPO und deutlich schneller. Gewinnt es → Iterationen 3–4× billiger; verliert es → starkes Pro-LSTM-Argument | Win-win fürs Paper |
| B4 | **Echtes PLR:** Seeds nach Lernpotenzial priorisieren (Proxy: knapp gescheiterte Episoden hoch gewichten, Top-K-Sampling) statt fester swarm_prob | direkter Hebel für Holdout-B-Ziel ≥60% |
| B5 | **`enable_critic_lstm=False` ablatieren:** SB3-Doku — bremst auf manchen Envs und schadet; halbiert LSTM-Last | Phase-1-Kurzrun genügt |

### C — Reihenfolge

1. **Sofort (~1h, risikofrei):** B1 + A3 + die drei Doku-Fixes aus Abschnitt 1 (Exit-Distanz, Wand-Penalty-Revert, CLAUDE.md-Fallstrick) ins Changelog.
2. **~½ Tag:** A1 + A2 → schnellste Konfiguration mit intakter Lernkurve fixieren.
3. **Voller v11-Run** (4 Phasen) mit Gewinner-Konfiguration. Ziel: ≥86% stoch halten, det deutlich >36%.
4. **Cloud parallel:** 3 Seeds final (A4) + Ablationen B3/B5.
5. **Falls Holdout B < 60%:** B4 (PLR).

> Doku-Pflicht: jede Änderung + Messung ins `CHANGELOG.md` (Was/Warum/Ergebnis, 50-Seed-Eval).

---

## 6. Spiel- & Reward-Design: Anforderungen für RL auf generierten Welten

*Recherche 06.07.2026 (Procgen-Designprinzipien, PBRS-Theorie, Reward-Engineering-Überblicke),
abgeglichen mit dem Stoneforge-Ist-Zustand.*

### 6.1 Anforderungen an das Spiel

| # | Prinzip | Stoneforge heute |
|---|---------|------------------|
| 1 | **Garantierte Lösbarkeit** — jede Welt muss lösbar sein, sonst verrauschtes Signal + unbekanntes SR-Ceiling | ✅ verifiziert: 150/150 Seeds lösbar |
| 2 | **Verlässlicher Schwierigkeits-Regler** — Curriculum braucht einen Knopf, der die Schwierigkeit monoton steuert | ⚠️ Regler nutzt Luftlinie; echte Schwierigkeit (BFS-Pfad) streut 42–75 bei „35–45" |
| 3 | **Hohe Diversität** — unbegrenzt viele, nicht memorisierbare Level → erzwingt Generalisierung | ✅ Hash-Noise, 7 Biome, unbegrenzte Seeds |
| 4 | **Beobachtung + Gedächtnis müssen ausreichen** (POMDP); Vorsicht bei Aliasing (viele identisch aussehende Orte) | ✅ 15×15 + Kompass + LSTM reicht (86% belegen das); Kompass ist essenziell |
| 5 | **Schnell, deterministisch, parallelisierbar** | ✅ C++-Core, seed→Welt |
| 6 | **Minimaler Aktionsraum** — irrelevante Aktionen verdünnen Exploration | ✅ Discrete(4); Mining erst wenn Navigation sitzt |
| 7 | **Saubere Episodengrenzen** — Timeout großzügig vs. Optimalpfad; Zeit-Abbruch als Truncation ohne Extra-Strafe | ✅ 4000 vs. ~55 optimal; Early-Stop korrekt als Truncation |
| 8 | **Train-/Test-Verteilung identisch** — gleiche Generierung, neue Seeds | ✅ (P3 trainiert 25–45, Eval 35–45 — im Paper erwähnen) |

### 6.2 Anforderungen an den Reward

1. **Primärsignal sparse & eindeutig:** großer Terminal-Reward fürs Ziel, nichts darf ihn dominieren.
   → ✅ +100 Exit ≫ alle Shaping-Terme; −10 Timeout.
2. **Dichte Hilfe nur als PBRS** (`F = γ·Φ(s′) − Φ(s)`) — einzige beweisbar policy-invariante Form
   (Ng et al.). Bedingungen: Φ = **echte** Fortschrittsmetrik (BFS-Pfaddistanz, nie Luftlinie —
   sonst lockt das Shaping in Sackgassen) und Shaping-γ = RL-γ.
   → ✅ beides erfüllt (β=2.5 → netto +0.01/Tile, γ=0.999 beidseitig). Der frühere Buffer-Bug
   (Manhattan-Fallback gab +2.5/Schritt fürs Zurücklaufen) war das Lehrbuchbeispiel für Verletzung
   von Bedingung 1 — korrekt gefixt.
3. **Kein Straf-Stacking — Information gehört in die Observation, nicht in den Reward.**
   Viele kleine Strafen (Wand, Loop, Idle, Step) summieren sich, bis „nichts tun" lokal optimal ist
   (genau die v6-Diagnose „Stehen ist billiger als Erkunden"). Faustregel: typische Strafe/Schritt ≪
   erwartbarer Fortschritts-Reward. Wenn der Agent etwas *wissen* soll („Wand!", „war schon hier"),
   dann als Obs-Feature (Aktions-Buffer, visit_count), nicht als Strafe.
   → ⚠️ Wand-Penalty (−0.05/−0.25) + Loop-Penalty (−0.15) sind aktuell wieder aktiv.
4. **Kein Reward-Hacking-Potenzial:** jeden Bonus auf „Farmbarkeit" prüfen.
   → ✅ Explorations-Bonus endlich & netto nur +0.01; Oracle-Test bestätigt: optimaler Pfad
   maximiert auch den Reward (+101.7).
5. **Beträge klein & stabil:** pro Schritt grob in [−1, +1] (Terminal ausgenommen); γ zum Horizont
   passend. → ✅.
6. **Empirisch validieren statt nur designen:** Oracle-Return ≈ optimal ✅, Random-Return negativ,
   Agentenverhalten regelmäßig ansehen (`watch_agent.py`).

### 6.3 Konsequenzen → zwei neue Plan-Punkte

> **Status-Update 06.07.2026:** B6 und B7 (Variante „ohne Strafen") sind umgesetzt — siehe
> CHANGELOG v2026-07-06. Zusätzlich entdeckt & gefixt: prozess-globales WorldGen-Config-Leck
> (Eval-Env verschob die Phase-3-Trainingsverteilung). Obs jetzt 229-dim (Energie/Inventar raus).

| # | Maßnahme | Begründung |
|---|---|---|
| **B6** | ✅ **(umgesetzt 06.07.2026)** **Exit-Platzierung auf BFS-Distanz umstellen** (`world.cpp:328` — Kandidaten nach BFS-Ebene statt `dx²+dy²` wählen; die Flood-Fill läuft dort ohnehin schon) | Präzises Curriculum (Phase 1 „5–12" heißt dann wirklich 5–12 Laufweg), ehrliche Eval-Angaben, weniger Schwierigkeits-Streuung im Training. Rebuild + Vergleichslauf nötig; ändert die Aufgabenverteilung → im Changelog als Umgebungsversion dokumentieren! |
| **B7** | **A/B-Test Wand-/Loop-Penalty:** v11 einmal mit (Status quo) und einmal ohne (`moveBlocked`/`positionLoop` → 0), gleiche Seeds, gleiches Budget | Beendet das Hin-und-Her (v6 entfernt → 16dc8dd revertiert) mit Daten statt Meinung. Literatur-Tendenz: Strafen raus, Information in die Obs (visit_count/Aktions-Buffer aus v7 waren die richtige Idee — nur die Hyperparameter waren damals falsch). |
| **B8** | **A/B-Test Swarm-Semantik (Recherche 07.07.2026):** Run A = Status quo (Erfolgs-Swarm, 30% Replay gelöster Seeds), Run B = `--plr` (gescheiterte Seeds wiederholen, gelöste raus), optional Run C = `--no-swarm`. Gleiches Budget, gleicher Seed. | PLR-Literatur (Jiang et al., ICML 2021, arXiv:2010.03934) empfiehlt Replay von Levels mit **hohem Lernpotenzial** (= noch nicht gemeistert) — der aktuelle Erfolgs-Swarm macht das Gegenteil und riskiert Layout-Memorierung (widerspricht dem Generalisierungsziel). Aber: der 86%-Referenzlauf lief MIT Erfolgs-Swarm → mit Daten statt Meinung entscheiden. Hinweis: `--plr` ist binäre PLR-Näherung (echtes PLR gewichtet nach TD-Error). Phasenwechsel-Bug (Pool überlebte Phasen) am 07.07.2026 gefixt: Pool wird bei Phasenstart geleert. |
| **B9** | **Asymmetrischer Critic statt FOV-Shrinking (Recherche 07.07.2026):** Critic erhält im Training privilegierte Info (z. B. echte BFS-Distanz, Exit-Position), Actor bleibt bei 15×15. Custom-Policy in sb3-contrib nötig (mittlerer Aufwand). | Literaturkonforme Umsetzung der Idee „anfangs mehr Sicht" (Asymmetric Actor-Critic, Pinto et al. 2017; Informed AAC 2025, arXiv:2509.26000). Naives Schrumpfen des Actor-Sichtfelds verworfen: fixe Netz-Eingabe erzwingt totes 21×21-Padding, Kollaps-Risiko beim Wegnehmen der Weitsicht, redundant zum Exit-Distanz-Curriculum. Adressiert direkt die beobachtete Critic-Schwäche (EV ≈ 0.1 im v11-Lauf). |

---

## 7. Deterministische Performance verbessern (Det/Stoch-Gap) — Maßnahmenkatalog

*Recherche 06.07.2026. Ausgangslage: 86% stochastisch / 36% deterministisch (Testset A).
Diagnose bleibt: Die Policy nutzt Sampling-Zufall als Loop-Breaker; Argmax läuft in 2-Schritt-Loops.*

| # | Maßnahme | Idee | Aufwand / Risiko |
|---|---|---|---|
| **D1** | **Entropie-Annealing + Greedy Fine-Tune sauber durchziehen** (= B1/B2) | Temperatur-/Entropie-Annealing ist laut Literatur *der* Standardweg, eine stochastische Policy zu determinisieren. Voraussetzungen: Annealing-Start-Bug fixen (0.05 statt 0.01), Modellselektion in P3/P4 auf **det.** SR | klein / gering — bereits geplant |
| **D2** | **Self-Imitation / Behavior Cloning auf eigenen Erfolgs-Episoden** | Mit dem fertigen Modell stochastisch N Episoden sammeln, **nur die erfolgreichen** behalten, Policy supervised auf deren (obs, action)-Paare nachtrainieren (Cross-Entropy auf den Policy-Head, LSTM-States mitführen). Argmax reproduziert danach die demonstrierten Erfolgs-Trajektorien. Theoriebasis: Self-Imitation Learning (Oh et al. 2018), GCSL (Ghosh et al.) — Lernen aus eigener erfolgreicher Erfahrung, ohne externen Experten | mittel (eigener PyTorch-Loop, ~100 Zeilen) / gering — **größter neuer Hebel** |
| **D3** | **Low-Temperature-Eval statt Argmax** (`scripts/eval_temperature.py` existiert schon!) | Sweep τ ∈ {0.1, 0.25, 0.5, 0.75, 1.0}: bei kleinem τ fast-deterministisch, aber Rest-Zufall bricht Loops. SR-vs-τ-Kurve ist zugleich das perfekte Paper-Diagramm zur POMDP-These („wieviel Zufall braucht die Policy wirklich?") | minimal / keins — nur Messung |
| **D4** | **Loop-Evidenz in die Observation** (v7-Ideen mit v10-Hyperparametern wiederholen) | visit_count + Aktions-Buffer geben dem LSTM *deterministische* Evidenz „ich war hier schon / ich oszilliere". v7 scheiterte an falschen Hyperparametern (batch/ent_coef), nicht an der Idee. Einzeln ablatieren: erst nur visit_count (+1 dim), dann ggf. Buffer | mittel / mittel (Obs-Änderung = neues Modell nötig) |
| **D5** | **Invalid-Action-Masking beim Eval** | Beim deterministischen Eval Aktionen maskieren, die in eine Wand führen (Argmax über gültige Aktionen). Etablierte Technik (Huang & Ontañón); eliminiert Wand-Banging, löst aber A↔B-Loops auf freien Tiles nicht | klein / dokumentationspflichtig (Inference-Hilfe, im Paper ausweisen) |
| **D6** | **Selbst-Distillation mit Temperatur** (TS-OPSD-Idee) | Die geglättete Hochtemperatur-Verteilung in den Studenten destillieren → Policy internalisiert Exploration; experimentell, aus der LLM-Literatur | hoch / hoch — nur als Ausblick erwähnen |

**Empfohlene Reihenfolge:** D3 sofort (kostenlos, nur messen) → D1 im v11-Run → D2 als Post-Processing
auf das beste Modell (unabhängig vom Training testbar!) → D4 falls der Gap danach noch > 15 pp ist.

**Für die Projektarbeit:** D3 liefert die Kernaussage sauber quantifiziert („die Policy braucht τ ≥ X,
um Loops zu brechen"), D2 zeigt einen konstruktiven Weg, den Gap zu schließen — beides stärkt die
POMDP-These, statt sie zu ersetzen.

---

## 8. Quellen

- [SB3-Contrib: Recurrent PPO Doku](https://sb3-contrib.readthedocs.io/en/master/modules/ppo_recurrent.html) — Frame-Stacking oft kompetitiv & schneller; Critic-LSTM ggf. deaktivieren
- [W&B: PPO vs RecurrentPPO auf masked-velocity Envs](https://wandb.ai/sb3/no-vel-envs/reports/PPO-vs-RecurrentPPO-aka-PPO-LSTM-on-environments-with-masked-velocity-SB3-Contrib---VmlldzoxOTI4NjE4)
- [Prioritized Level Replay (Jiang et al., ICML 2021)](https://arxiv.org/pdf/2010.03934) · [Referenz-Implementierung](https://github.com/facebookresearch/level-replay)
- [Potential-based Reward Shaping in Sokoban](https://arxiv.org/pdf/2109.05022) · [PBRS-Überblick](https://www.emergentmind.com/topics/potential-based-reward-shaping)
- [Crafter (Hafner 2021, PyPI)](https://pypi.org/project/crafter/1.2.2/) · [BALROG-Benchmark mit Crafter-Beschreibung](https://arxiv.org/html/2411.13543v2)
- [Robust RL Navigation via Procedural Map Generators](https://arxiv.org/pdf/2605.02528) — Zero-Shot-Maze-SRs 25–53%
- [POPGym: Benchmarking Partially Observable RL](https://matteobettini.com/publication/popgym-benchmarking-partially-observable-reinforcement-learning/POPGym-Benchmarking-Partially-Observable-Reinforcement-Learning.pdf)

**Spiel-/Reward-Design (Abschnitt 6):**
- [Procgen: Leveraging Procedural Generation to Benchmark RL (Cobbe et al.)](https://cdn.openai.com/procgen.pdf) — Designprinzipien
- [Comprehensive Overview of Reward Engineering and Shaping](https://arxiv.org/html/2408.10215v1) — Reward-Hacking, Overpowering Shaping
- [Revisiting Sparse Rewards for Goal-Reaching RL](https://arxiv.org/pdf/2407.00324)
- [Adversarial RL for Procedural Content Generation](https://arxiv.org/abs/2103.04847) — „challenging but not impossible"

**Det/Stoch-Gap (Abschnitt 7):**
- [Entropy Augmented Reinforcement Learning](https://arxiv.org/pdf/2208.09322) — Temperatur-Annealing als Schlüssel
- [Dynamic Entropy Tuning: Stochasticity vs Determinism](https://arxiv.org/pdf/2512.18336)
- [Learning to Reach Goals via Iterated Supervised Learning (GCSL)](https://arxiv.org/pdf/1912.06088) — Self-Imitation auf eigene Trajektorien
- [Episodic Self-Imitation Learning with Hindsight](https://arxiv.org/pdf/2011.13467)
- [Solving Deep Memory POMDPs with Recurrent Policy Gradients](https://people.idsia.ch/~alexander/2007/2/icann2007.pdf)
- [Temperature-Scaled On-Policy Self-Distillation](https://arxiv.org/pdf/2606.00755)
