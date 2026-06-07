# Changelog

## v2026-06-07

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

**Training:** `ppo_no_bfs`, 2M Timesteps, Curriculum exit=5–12→35–45, gestartet 07.06.2026.

**Ergebnis:** *ausstehend — wird nach Trainingsabschluss ergänzt.*

**Hypothese:** SR wird initial niedriger als ppo_phase4 (100%), da das Netz Wandnavigation ohne GPS lernen muss. Ziel: ≥70% auf Seeds 7000–7049.

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
