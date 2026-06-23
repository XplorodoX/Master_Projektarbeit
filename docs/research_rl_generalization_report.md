# Reinforcement Learning für Generalisierung auf prozedural generierten Navigations-Tasks — Forschungsreport für Florian (Stoneforge / Hochschule Aalen)

## Kurzantwort vorweg

Für ein 2D-Tile-Grid mit prozedural generierten Karten, in dem der Agent ein Exit finden muss, und mit dem aktuellen 15×15-Beobachtungsformat ist die empirisch klar dominierende Familie **PPO-basierte on-policy Actor-Critic-Methoden mit Impala-/Impoola-CNN-Encoder, kombiniert mit Prioritized Level Replay (PLR) und Data Augmentation (DrAC)**. Phasic Policy Gradient (PPG) ist der bisher konsistenteste Procgen-SOTA, DreamerV3 ist die stärkste model-based Alternative, und Curriculum-Learning auf festen Distanz-Bins (wie aktuell genutzt) ist mit hoher Wahrscheinlichkeit ein zentraler Mitverursacher des beobachteten Generalisierungsproblems. Der konkrete „nächste Schritt“ für Florian: **PPO + Impala-CNN + LayerNorm + PLR + Random-Crop-Augmentation auf der vollen Distanzverteilung trainieren, mit 25–50 M Timesteps und 64 parallelen Envs** (siehe Abschnitt 10). Im Folgenden die ausführliche Begründung mit Papers und Zahlen.

---

## 1. Algorithmen-Vergleich für prozedurale Navigation

### 1.1 PPO (Schulman et al., 2017) — die zähe Baseline

PPO ist *der* De-facto-Standard auf Procgen. Cobbe et al. (2020, „Leveraging Procedural Generation to Benchmark Reinforcement Learning“) verwenden PPO als Referenz für alle 16 Procgen-Spiele. Aus der Procgen-NeurIPS-2020-Challenge geht hervor (Mohanty et al., arXiv:2103.15332), dass **die Top-Teams im Generalization-Track Varianten von PPG oder PPO einsetzten**; SAC, Reactor, P3O lagen schlechter. PPO ist *langsam* in Sample-Efficiency, aber *robust* und *einfach abzustimmen* — entscheidende Vorteile in einem Studierenden-Projekt.

Pro: Einfach, stabile Implementierungen (SB3, CleanRL), gut dokumentierte Hyperparameter. Contra: ohne Augmentation/Regularisierung deutlicher Train-Test-Gap auf Procgen (Cobbe et al., 2020 zeigen 20–50 % Gap auf den meisten Spielen).

### 1.2 PPG — Phasic Policy Gradient (Cobbe et al., ICML 2021, arXiv:2009.04416)

PPG trennt Policy- und Value-Training in zwei Phasen mit unterschiedlichem Sample-Reuse und einer „Auxiliary Value“-Distillation. Auf Procgen-Hard verbessert PPG die Test-Returns deutlich gegenüber PPO. Wichtig: Die nachfolgende Studie **„PPG Reloaded“ (Sun et al., OpenReview UlOHeXD4MD)** zeigt empirisch, dass nicht der hohe Value-Sample-Reuse der Hauptgrund ist, sondern **Policy-Regularisierung und Datenvielfalt**. Praktische Konsequenz: PPG-Effekte lassen sich z.T. günstiger mit Augmentation/Regularisierung reproduzieren — also DrAC/UCB-DrAC sind oft ein billigerer Hebel.

### 1.3 IMPALA (Espeholt et al., ICML 2018)

IMPALA ist ein distributed actor-learner mit V-trace Off-Policy-Korrektur. Es liefert die **Impala-CNN-Architektur** (Residual-Blocks, 15 Layer), die heute Standard für Procgen ist. Als Algorithmus eigenständig nicht mehr SOTA für Generalisierung (Cobbe 2020 zeigt: PPO schlägt IMPALA auf Procgen-Test-Returns), aber als **Backbone-Architektur** kanonisch.

### 1.4 DreamerV3 (Hafner et al., Nature 2025, arXiv:2301.04104)

Model-based mit Recurrent State-Space Model (RSSM). DreamerV3 erreicht mit *fixen* Hyperparametern SOTA über 8 Domänen inkl. Atari, DMLab, Crafter, Minecraft. Auf DMLab schlägt DreamerV3 in 100 M Frames die scalierte IMPALA und R2D2+-Baseline bei 1 Mrd. Schritten („data-efficiency gain >1000×“). Auf Crafter erzielt DreamerV3 SOTA. **Für Florian-Setup interessant, weil prozedurale Welten mit räumlicher Struktur sehr nahe an Crafter sind**, aber: Implementierungsaufwand erheblich (offizieller Code: github.com/danijar/dreamerv3, JAX), und der C++/pybind11-Workflow muss Reset-/Step-Stabilität liefern. Empfehlung: nur als Stretch-Goal.

### 1.5 MuZero / EfficientZero (Schrittwieser et al. 2020; Ye et al. 2021)

Tree-search-basierte Modell-RL, sehr sample-efficient (EfficientZero hält SOTA auf Atari100k mit 400 k Frames), aber Implementierungs- und Compute-Aufwand massiv. Für prozedurale Navigation mit häufiger Topologieänderung ist der Vorteil von MCTS weniger ausgeprägt, da der Suchbaum pro Episode neu ist. **Nicht empfohlen für eine Projektarbeit.**

### 1.6 R2D2 / Agent57 / APE-X (Kapturowski et al., ICLR 2019; Badia et al., ICML 2020)

R2D2 (Recurrent Replay Distributed DQN) und Agent57 sind distributed off-policy mit LSTM-Memory und (Agent57) NGU-Exploration. Sehr leistungsstark auf Atari, aber: brauchen Hunderte parallele Actors und große Replay-Buffer. **Für 8 Envs unrealistisch.** Wichtiger Lerneffekt: Wenn Memory wichtig ist (POMDP), helfen rekurrente Architekturen — Florians 15×15 lokales Sichtfeld macht sein Setup zu einem POMDP.

### 1.7 Rainbow DQN (Hessel et al., AAAI 2018)

Kombination aus Double-DQN, Prioritized Replay, Dueling, Multi-Step, Distributional, Noisy Nets. Auf Procgen empirisch schwächer als PPO/PPG für Generalisierung (Cobbe et al., 2020); off-policy Value-Learning leidet stärker unter Train/Test-Distribution-Shift. Florians DQN-Erfolg (40 % bei Version 0) ist eher Glück bzw. liegt am dichten BFS-Reward — er wird damit schwer auf 35–45 Tiles skalieren.

### 1.8 SAC-Discrete / MaxEnt RL (Christodoulou 2019)

Soft Actor-Critic in der diskreten Variante. Für Procgen-Style-Tasks selten verwendet; Procgen-Challenge zeigt SAC unter Top-3 in keinem Track. Ein Pluspunkt ist die Entropie-Regularisierung, aber PPO mit Entropy-Bonus erzielt ähnliches.

### 1.9 PPO + Auxiliary Losses (UNREAL Jaderberg et al. 2017; ICM Pathak et al. 2017; RIDE Raileanu & Rocktäschel 2020)

Diese Familie ist besonders wertvoll für Florians Problem (siehe Abschnitt 7). RIDE wurde explizit für prozedural generierte MiniGrid-Umgebungen entwickelt und schlägt RND/ICM dort deutlich.

### 1.10 Decision Transformer / Trajectory Transformer (Chen et al. 2021; Janner et al. 2021)

Offline-RL via Sequence-Modelling. **Nicht für Online-Training-from-scratch** auf prozeduralen Levels gedacht; nur sinnvoll mit großem Offline-Demonstrations-Datensatz.

### 1.11 NetHack/MiniHack-Algorithmen

NetHack Learning Environment (Küttler et al. 2020) und MiniHack (Samvelyan et al. 2021) liefern wichtige Lessons: **IMPALA mit RND** war Baseline und RND verbesserte teils, aber „RND exploration leads to consistently worse results“ in einigen NetHack-Subtasks. **BeBold (Zhang et al., 2021)** und **RIDE (Raileanu & Rocktäschel, 2020)** sind die spezifisch für prozedurale Gridworlds entwickelten Exploration-Boni und sind für Stoneforge sehr relevant.

### 1.12 Empirisches Ranking für Procgen-style Generalisierung (in absteigender Reihenfolge)

1. **PPG + DrAC + PLR** — best-published Generalization-Combo (UCB-DrAC ~+40 % Test-Performance ggü. PPO, Raileanu et al. 2020)
2. **PPO + DrAC + PLR** — Brot-und-Butter-Setup, leichter zu implementieren
3. **PPO + Impala-CNN** — robust Baseline
4. **DreamerV3** — model-based, sehr sample-efficient, aber teuer
5. PPG ohne Augmentation
6. PPO ohne Augmentation
7. Rainbow-DQN — schwächer in Generalisierung
8. SAC-Discrete — nicht kompetitiv

---

## 2. Architektur-Empfehlungen

### 2.1 CNN vs MLP

Für jede räumlich strukturierte 2D-Observation ist ein CNN essenziell, weil Translationsinvarianz eine harte Inductive Bias liefert, die ein MLP erst aus den Daten lernen müsste — und das wird in der Generalisierung selten gelingen. Florians 15×15 Grid mit 4 Kanälen (Tile-Type, Visited-Mask, Potential-Field-Features) ist genau das Format, für das CNNs konstruiert sind. Die zusätzlichen Skalare (HP, Energy, exitDx/Dy) sollten als separater MLP-Branch verarbeitet und mit dem CNN-Feature konkateniert werden.

### 2.2 Nature-CNN vs Impala-CNN vs Impoola-CNN

- **Nature-CNN** (Mnih et al. 2015): 3 Conv-Layer (32, 64, 64), kompakt, ~1 M Parameter. Auf Procgen klar unterlegen ggü. Impala-CNN (Cobbe et al. 2020).
- **Impala-CNN** (Espeholt et al. 2018): 15-Layer Residual-Net, drei Stages à zwei Residual-Blocks mit MaxPool. Heute De-facto-Standard für Procgen.
- **Impoola-CNN** (Trinkle et al. 2025, arXiv:2503.05546): Impala-CNN, aber **Flatten ersetzt durch Global Average Pooling (GAP)** vor dem Linear-Head. Empirisch übertrifft Impoola Impala-CNN deutlich auf Procgen und ist sogar besser als größere komplexere Modelle. Die Autoren begründen es mit reduzierter Translation-Sensitivity. **Größte Gewinne bei Spielen *ohne* Agent-zentrierte Observation** — Florians Setup ist agent-zentriert, sodass der Gewinn geringer ausfallen wird, aber GAP reduziert auch die Anzahl Parameter und ist eine billige Ergänzung.

**Empfehlung für 15×15 Grid:** Da die Eingabe sehr klein ist, ist eine *abgespeckte Impala-CNN* sinnvoll. Konkretes Design (siehe Code im Roadmap-Abschnitt):
- Stage 1: 16 Channels, ein Residual-Block, kein Pooling oder MaxPool(2) → 7×7
- Stage 2: 32 Channels, ein Residual-Block, MaxPool(2) → 3×3
- Stage 3: 32 Channels, ein Residual-Block, GAP → 32-dim Vektor
- Konkateniere mit MLP-encoded Skalaren (5 Skalare → 32-dim) und 9 Potential-Field-Features (→ 16-dim)
- Linear → 256 → Policy/Value-Heads

Typische Procgen-Impala-CNN nutzt 64×64-Inputs, dort sind drei MaxPools sinnvoll. Bei 15×15 würde dreimaliges MaxPool auf 1×1 reduzieren — daher **maximal zwei Pool-Stufen**.

### 2.3 Frame Stacking

Für Navigation typischerweise weniger relevant, weil der Markov-State (Position, Visited-Mask) im Observation bereits enthalten ist. Falls Bewegungsdynamiken (Geschwindigkeit, Richtung) wichtig wären, lohnt Frame-Stacking — bei Tile-basiertem Movement nicht. **Empfehlung: kein Frame-Stacking, stattdessen Visited-Mask (hat Florian bereits) als Memory-Surrogat.**

### 2.4 LSTM/GRU/Transformer für Memory

Bei lokalem 15×15 Fenster und 35–45 Tiles Exit-Distanz ist die Umgebung **partiell beobachtbar** — der Exit ist initial außerhalb des Sichtfelds. Florians exitDx/exitDy-Skalare lösen das teilweise (Cheating-State), aber für die Erkundungsgeometrie braucht der Agent Memory.

- **LSTM/GRU**: Klassisch für POMDPs in RL. Procgen-Studien zeigen, dass rekurrente PPO **nicht immer hilft** und teils sogar schadet (Cobbe et al. 2020 berichten gemischte Resultate). Für MiniHack/NetHack sind LSTMs Standard.
- **Transformer**: GTrXL (Parisotto et al. 2020), Decision Transformer; teurer und schwerer zu stabilisieren in Online-RL.

**Empfehlung:** Erst feedforward mit Visited-Mask versuchen. Wenn klar Memory-bedingte Fehler auftauchen (gleiche Sackgasse mehrfach betreten), GRU mit Hidden Size 256 hinzunehmen. RecurrentPPO aus sb3-contrib ist Florians einfachster Weg.

### 2.5 Network Size und Skalierung

Aktuelle RL-Forschung (Obando-Ceron et al. 2024, „In value-based deep reinforcement learning, a pruned network is a good network“; Schwarzer et al. 2023) zeigt: **größere Netze helfen oft, aber nur mit passenden Regularisierungstechniken** (LayerNorm, Pruning, Resets). Bei 15×15 Input überdimensionieren leicht. Faustregel: 1–3 M Parameter sind ausreichend.

### 2.6 LayerNorm vs BatchNorm vs GroupNorm

- **BatchNorm**: in RL **problematisch**, weil Batch-Statistiken zwischen Train und Inference variieren und off-policy Korrekturen verschiebt. Häufige Quelle für Instabilität.
- **LayerNorm**: in DreamerV3 (Hafner et al. 2025) und vielen aktuellen PPO-Implementierungen Standard. **Empfohlen.**
- **GroupNorm**: gute Alternative, weniger gebräuchlich.
- LayerNorm hilft zudem nachweislich gegen **Plasticity Loss** (siehe Abschnitt 8).

### 2.7 Action-Conditioned Architectures

Forward-Dynamics-Heads, die `next_state | state, action` vorhersagen (siehe Abschnitt 7). Lohnt sich primär als Auxiliary-Loss, nicht als architektonische Bedingung der Policy.

---

## 3. Curriculum Learning vs Domain Randomization vs PLR

### 3.1 Kernfindung — relevant für Florians aktuelles Problem

Florians manuelles Distanz-Curriculum (5–15 → 15–30 → 35–45) hat ein bekanntes Problem: **Catastrophic Forgetting und Distribution-Shift zwischen Stages**. Die Forschung zeigt sehr klar, dass *manuelle Stage-Curricula auf prozeduralen Umgebungen häufig schlechter generalisieren als gleichverteiltes Sampling oder adaptive Methoden*. Das passt exakt zu Florians Symptomatik: gut auf Stages, scheitert auf voller Schwierigkeit.

### 3.2 Prioritized Level Replay — PLR (Jiang, Grefenstette & Rocktäschel, ICML 2021, arXiv:2010.03934)

PLR sampelt Levels nicht uniform, sondern proportional zu deren *Learning Potential*, gemessen meist als **L1-Value-Loss** oder positive TD-Error. Auf Procgen verbessert PLR Test-Returns um über **76 % relativ zur PPO-Baseline** im kombinierten State-of-the-Art (Jiang et al. 2021). PLR induziert ein emergentes Curriculum — vom Algorithmus selbst, basierend auf der aktuellen Policy. Wichtig: PLR braucht eine Möglichkeit, **Levels per Seed reproduzierbar zu setzen** — Florian hat das (seedbasierte Welten).

**Empfehlung für Florian: PLR ist mit hoher Wahrscheinlichkeit der größte Einzelhebel.** Implementierung in `facebookresearch/dcd` und `facebookresearch/level-replay` verfügbar.

### 3.3 ACCEL (Parker-Holder et al., ICML 2022, arXiv:2203.01302)

ACCEL = PLR + Evolutionäre Mutation hoch-Regret-Levels. State-of-the-art im Unsupervised Environment Design auf MiniGrid-Mazes und BipedalWalker. Setzt voraus, dass man Levels **editieren** kann (z.B. einzelne Tiles ändern). Stoneforge ist seed-basiert generiert; eine Mutation würde eine explizite Welt-Repräsentation jenseits des Seeds erfordern. **Implementierungsaufwand: hoch.** Eher als Master-Thesis-Erweiterung.

### 3.4 POET / Open-Ended Coevolution (Wang et al. 2019/2020)

Adversariale Welt-Generation. Konzeptuell mächtig, in der Praxis komplex. Nicht empfohlen für Florians Setup.

### 3.5 Domain Randomization

Statt manuellem Curriculum: **trainiere von Anfang an auf der vollen Distanzverteilung 5–45 mit überdurchschnittlicher Repräsentation einfacher Levels (z.B. Mixed-Sampling 50/50 leicht/schwer)**. Empirisch in MiniGrid, Procgen oft besser als Curriculum.

### 3.6 Wann Curriculum vs. Sampling

- **Curriculum hilft**, wenn die schwierigste Stufe ohne Vorwissen praktisch unlernbar ist (extrem sparse Reward, lange Horizonte).
- **Curriculum schadet**, wenn die Stages andere Lösungsstrategien begünstigen als die Final-Stage. Florians 5–15 Tile-Welten haben ggf. andere Pfad-Topologien als 35–45, sodass die im Curriculum gelernte Policy nicht direkt extrapoliert.

**Konkrete Empfehlung: Curriculum durch PLR ersetzen oder zumindest „Burning-in“-Phase auf gemischter Verteilung anschließen.**

---

## 4. Reward Shaping Best Practices

### 4.1 Potential-Based Reward Shaping (Ng, Harada & Russell, ICML 1999)

Theorie: Wenn man die Belohnung modifiziert per `F(s,s') = γΦ(s') − Φ(s)` mit einer beliebigen Potentialfunktion Φ, bleibt die **optimale Policy unverändert** (Policy Invariance). Alle anderen Shaping-Formen können die Optimalpolicy verschieben.

**Florians BFS-Distanz-Reward ist *fast* PBRS, aber NICHT korrekt formuliert.** Der typische Fehler: `r_shape = +0.5 · Δ_BFS` ohne γ-Skalierung. Korrektur:

```
Φ(s) = -BFS(s, exit)        # negative Distanz als Potential
F(s, s') = γ · Φ(s') - Φ(s)
       = γ · (-BFS(s')) - (-BFS(s))
       = BFS(s) - γ · BFS(s')
```

Mit γ=0.999 ist das nahe an `BFS(s) − BFS(s') = ΔBFS`, aber **mathematisch sauber und garantiert policy-invariant**. Die multiplikative Skalierung +0.10 bis +0.50 ist erlaubt, solange sie auf F() angewendet wird.

### 4.2 BFS-Distance als Reward in der Literatur

Wiewiora (2003) zeigt, dass PBRS mit distanz-basiertem Potential äquivalent zu einer initialen Q-Value-Initialisierung ist. Distanz-Heuristiken sind eine der von Ng et al. (1999) ausdrücklich vorgeschlagenen Klassen. **Florians BFS-Reward ist also theoretisch fundiert — er muss nur korrekt potential-basiert formuliert werden.**

### 4.3 Sparse vs Dense

Für prozedurale Navigation mit echter Pfaddistanz (BFS) ist **dense Shaping fast immer besser** als pure sparse Exit-Belohnung — vorausgesetzt das Shaping ist policy-invariant. Sparse-Only funktioniert nur mit starker Exploration (RND, NGU).

### 4.4 Optimaler Trade-off

Aus der Procgen/MiniGrid-Praxis und Florians eigenen Zahlen:
- Exit-Bonus: groß genug, dass er Shaping-Signal dominiert (Faktor ≥ 10× max kumulierte Shaping-Reward). Florians +100 ist gut, sofern Episoden ≤ ~200 Schritte.
- Schritt-Strafe: klein. -0.01 ist okay, aber **dieser Term ist nicht potential-basiert** und kann bei pendelndem/explorierendem Verhalten Bias erzeugen.
- Shaping-Faktor: 0.1–0.5 × ΔBFS ist ein üblicher Range. Empfehlung: starte bei 0.25, dann sweepen.

### 4.5 Penalty-Design — kritisch!

Die Forschung zu Reward-Hacking warnt klar: **Stuck-Penalties, Pendel-Penalties, Multi-Visit-Penalties sind NICHT potential-basiert und können die Optimal-Policy verschieben.** Sie sind oft notwendig in der Praxis, aber:

- Sie sollten **klein** sein (Größenordnung der Schritt-Strafe, nicht der Exit-Belohnung).
- Sie können konkurrierende Optimierungsziele erzeugen (Exit finden vs. „nicht stuck sein“).
- **Empfehlung für Florian: Alle drei Penalties (Stuck, Pendel, Multi-Visit) deaktivieren und nur PBRS-BFS + Exit + minimale Step-Penalty testen.** Wenn der Agent dann pendelt, ist das ein Hinweis auf Exploration-Probleme — RND/RIDE statt Penalty einsetzen.

---

## 5. Exploration für sparse-reward Navigation

Selbst mit dichtem BFS-Reward können prozedurale Welten Sackgassen und „Trough“-Bereiche enthalten, in denen lokale Gradient-Maxima entstehen. Die wichtigsten Exploration-Methoden:

### 5.1 RND — Random Network Distillation (Burda et al., ICLR 2019)

Zwei Netze: ein fest randomisiertes Target und ein trainierbarer Predictor. Intrinsic-Reward = MSE(target, predictor) auf der aktuellen Observation. RND knackte als erstes Montezuma's Revenge ohne Demos. **Schwächen in prozeduralen Umgebungen** (Raileanu & Rocktäschel 2020): RND ist epistemisch-unscharf, da neue Levels per Definition neue States haben — der Predictor lernt nie zu Ende.

### 5.2 ICM — Intrinsic Curiosity Module (Pathak et al., ICML 2017)

Forward + Inverse-Dynamics-Modell, intrinsic-reward = Forward-Prediction-Error. Klassisch, aber leidet am „Noisy-TV-Problem“. ICM ist auf einfachen MiniGrid-Tasks brauchbar, in komplexen prozeduralen Welten schwächer als RIDE.

### 5.3 RIDE — Rewarding Impact-Driven Exploration (Raileanu & Rocktäschel, ICLR 2020)

Intrinsic-Reward proportional zur **Änderung des latenten State-Embeddings** zwischen aufeinanderfolgenden Schritten — belohnt Aktionen mit *großem Einfluss* auf den State. **Empirisch klarer Sieger auf prozeduralen MiniGrid-Tasks** gegenüber ICM, RND, Count-Based. Sehr relevant für Stoneforge.

### 5.4 NGU + Agent57 (Badia et al., ICLR/ICML 2020)

Episodic Curiosity-Memory + lifelong Novelty. Sehr leistungsstark, aber implementations-aufwendig und in Florians Compute-Budget kaum realistisch.

### 5.5 Go-Explore (Ecoffet et al., Nature 2021)

Speichert „interesting states“ in einem Archiv, kehrt gezielt dorthin zurück. Exploits Determinismus. Für seedbasierte prozedurale Welten begrenzt anwendbar (jede Episode neuer Seed = neues Archiv).

### 5.6 Count-Based / Episodic Curiosity through Reachability (Savinov et al. 2019)

ECR (Episodic Curiosity Reachability) ist konzeptuell elegant — Reward für Erreichen schwer-erreichbarer States. In MiniHack durchwachsene Ergebnisse.

### 5.7 Empfehlung für Florian

Reihenfolge:
1. **Erst PBRS-BFS korrekt setzen** und ohne Exploration-Bonus testen.
2. Falls Agent in Sackgassen pendelt: **RIDE** als Auxiliary Intrinsic-Reward.
3. RND als Fallback (einfach zu implementieren in CleanRL: `ppo_rnd_envpool.py`).

---

## 6. Data Augmentation und Regularization

### 6.1 RAD — Reinforcement Learning with Augmented Data (Laskin et al., NeurIPS 2020)

Wendet Bildaugmentationen auf Observations vor dem CNN an. **Random-Crop** ist auf Procgen die am konstantesten beste Augmentation. Verbessert Test-Performance auf Procgen um ~17–40 %.

### 6.2 DrAC / UCB-DrAC (Raileanu et al., NeurIPS 2021, arXiv:2006.12862)

DrAC = RAD + zusätzliche Regularisierungs-Losses, die **Policy und Value-Function invariant unter der Augmentation** machen sollen (KL-Penalty zwischen π(s) und π(aug(s))). UCB-DrAC wählt automatisch die beste Augmentation per Multi-Armed Bandit. **UCB-DrAC erzielt SOTA auf Procgen-Easy mit ~40 % Test-Performance-Boost über vanilla PPO** und schlägt PLR und Mixreg.

### 6.3 DrQ / DrQ-v2 (Kostrikov et al. 2020; Yarats et al. 2021)

Originär für Pixel-basierte continuous control mit SAC. Random-Shift-Augmentation als simple Mittlung mehrerer geshifteter Q-Werte. Für PPO/Discrete weniger relevant.

### 6.4 Empfehlung für 15×15 Grid

Für ein **Grid mit Tile-Type-Channel** sind klassische Bild-Augmentationen wie Color-Jitter sinnlos. Sinnvoll:
- **Random-Translation** (Padding + Crop): hat den größten generalisierungssteigernden Effekt auf Procgen. **Aber Vorsicht**: bei agent-zentrierter Observation ist der Agent immer in der Mitte — Translation würde diese Semantik brechen. Stattdessen: simuliere Translation durch **gelegentliches off-center Rendering** (Agent nicht immer in (7,7)).
- **Random-Cutout** (kleine Quadrate auf Visited-Mask oder Tile-Map maskieren): trainiert Robustheit gegen Sichtbeschränkungen.
- **Mixup für Grid-Observations**: nicht etabliert für diskrete Tile-Types, eher schädlich.
- **Dropout im FC-Head** (~0.1): mild, hilft.
- **L2-Regularisierung** auf Weights (~1e-5): sehr mild, oft positiv.

### 6.5 BatchNorm vermeiden, LayerNorm verwenden

Wie in 2.6 erläutert.

---

## 7. Auxiliary Tasks

### 7.1 Inverse Dynamics Prediction

Vorhersage `a_t | s_t, s_{t+1}`. Klassisch aus ICM und UNREAL. **Ye et al. (PMLR v155, 2021, „Auxiliary Tasks Speed Up Learning PointGoal Navigation“) zeigen +22 % SPL** für PointGoal-Navigation. Sehr empfohlen für Stoneforge.

### 7.2 Temporal-Distance Prediction

Vorhersage |t_j − t_i| aus zwei Observation-Embeddings einer Trajectory. In Ye et al. (2021) ebenfalls performance-steigernd. **Variante für Florian**: direkt **BFS-Distanz vorhersagen** als Auxiliary-Head — extrem natürlich, da BFS-Distance bereits berechnet wird. Das ist im Grunde eine Distanz-Heuristik-Repräsentation, die in der Navigation-Literatur (z.B. „Navigating to Objects by Distance Prediction“, arXiv:2202.03735) +2.6 % Success Rate brachte.

### 7.3 Forward Dynamics Prediction

Wie in ICM. Risiko: Noisy-TV. In strukturierten Tile-Welten gut implementierbar.

### 7.4 Reward Prediction (UNREAL, Jaderberg et al. 2017)

Sample-effizienter im Pixel-Control-Setup. Marginal nützlich, wenn der Reward bereits dicht ist.

### 7.5 Pixel Control / Feature Control

UNREAL-Idee: separate Q-Funktionen, die maximale Änderung in Sub-Patches der Observation maximieren. In Tile-Welten weniger anwendbar.

### 7.6 Contrastive Learning — CURL (Laskin et al. 2020), ATC (Stooke et al. 2021)

InfoNCE-Loss auf zwei augmentierten Views derselben State. Auf Procgen Mixed-Results — DrAC mit Regularisierung schlägt CURL meist.

### 7.7 Value Function Distillation

Zentraler Bestandteil von PPG — auxiliary value head am Policy-Netz, distilliert vom „echten“ Value-Netz. Wenn Florian nicht PPG nimmt, kann er den Distillation-Term separat hinzufügen.

### 7.8 Empfehlung — die „BFS-Distance Auxiliary Head“ Idee

Florian sollte einen **MLP-Head am CNN-Feature** trainieren, der die BFS-Distanz zum Exit aus dem aktuellen State vorhersagt (Regression, Huber-Loss). Das ist **selbstüberwachte Repräsentation**, die garantiert relevant für die Policy ist, und ist im C++-Sim trivial verfügbar. Erwartungswert: +10–20 % Test-Performance.

---

## 8. Plasticity Loss und Neural Network Pathologies in Deep RL

### 8.1 Das Problem

Lyle et al. (2022, „Understanding and Preventing Capacity Loss in RL“), Nikishin et al. (2022, „The Primacy Bias in Deep Reinforcement Learning“), Abbas et al. (2023, „Loss of Plasticity in Continual Deep RL“) und Dohare et al. (2024, Nature) zeigen: Deep-RL-Netze **verlieren über die Trainings-Dauer ihre Lernfähigkeit** — Aktivierungen werden sparser, Effective-Rank kollabiert, Gradienten verschwinden. Für lange Runs (>10 M Steps) und Curriculum-Übergänge ein zentrales Risiko.

**Florians Symptom — Version 0 erreichte 40 %, aktuelle Runs 0–6 % — passt phänomenologisch zu Plasticity Loss bei Curriculum-Transitions.** Wenn jede Stage das Netz „verbraucht“, verbleibt für die nächste Stage zu wenig Lern-Kapazität.

### 8.2 Gegenmaßnahmen

- **CReLU** (Abbas et al., CoLLAs 2023): Concatenated ReLU `[ReLU(x), ReLU(-x)]` verdoppelt die effektive Aktivierungsdichte. Sehr leichter Einbau, hilft messbar in Continual RL.
- **Periodic Resets** (Nikishin et al., ICML 2022, „Primacy Bias“): periodisches Neuinitialisieren der letzten Layer (z.B. alle 1 M Steps) bei Erhaltung des Replay-Buffers. Empirisch sehr stark bei DrQ, SPR, SAC auf Atari100k und DMC. Für on-policy PPO weniger gut etabliert, aber adaptierbar.
- **Shrink-and-Perturb** (Ash & Adams, NeurIPS 2020): `θ_new = α·θ_old + ε`, periodisch.
- **Plasticity Injection** (Nikishin et al., NeurIPS 2023): neue Layer hinzufügen statt zurücksetzen.
- **LayerNorm**: Lyle et al. (2024, „NaP — Normalize-and-Project“ bzw. Plasticity-Loss-Studien) zeigt LayerNorm als robuster Mitigation-Baseline.
- **L2-Weight-Regularization** kombiniert mit Dohare's Continual Backprop.

### 8.3 Empfehlung für Florian

- **LayerNorm in allen Conv- und FC-Layern** (kostenfrei, robuste Mitigation).
- **CReLU testen** (1-Zeilen-Änderung in der Aktivierungs-Funktion).
- **Auf Curriculum verzichten oder pro Stage einen Soft-Reset** der letzten 1–2 FC-Layer durchführen.

---

## 9. Procgen Benchmark Lessons Learned

Aus Cobbe et al. (2020, ICML), der NeurIPS-2020-Procgen-Challenge (Mohanty et al., arXiv:2103.15332), PPG (Cobbe et al. 2021), DrAC (Raileanu et al. 2021), PLR (Jiang et al. 2021), Impoola (Trinkle et al. 2025) extrahiert:

### 9.1 Wichtige Findings

1. **Training auf wenigen Levels = massiver Train-Test-Gap.** Cobbe et al. (2020) zeigen, dass bei 200 Trainings-Levels der Test-Score teilweise nur 50 % des Train-Scores erreicht. Mehr Trainings-Levels (∞ in Procgen-Hard) reduzieren den Gap.
2. **Größere Netze (Impala-CNN > Nature-CNN) generalisieren besser.** Counterintuitive: in Supervised Learning ist oft Underfitting der Schutz, in RL ist *Repräsentations-Capacity* der Engpass.
3. **PPG > PPO um 5–20 % Test-Score** auf den meisten Procgen-Spielen (Cobbe et al. 2021).
4. **UCB-DrAC > PPG** auf Procgen-Easy (Raileanu et al. 2021, ~40 % Verbesserung ggü. PPO).
5. **PLR allein bringt > 76 % über PPO**, kombinierbar mit DrAC.
6. **Impoola-CNN > Impala-CNN** (Trinkle et al. 2025) bei gleichen Parametern.
7. **Entropy-Bonus = 0.01** ist universell, **GAE-λ = 0.95, γ = 0.999** sind Procgen-Standard (anders als Atari mit γ=0.99!).
8. **Mehr parallele Envs (64+)** stabilisieren den Gradient — 8 Envs sind unterdimensioniert.

### 9.2 Standard-Hyperparameter aus CleanRL `ppo_procgen.py`

```
total_timesteps = 25_000_000
learning_rate = 5e-4
num_envs = 64
num_steps = 256
gamma = 0.999
gae_lambda = 0.95
update_epochs = 3
num_minibatches = 8 (Batch ~2048)
ent_coef = 0.01
vf_coef = 0.5
clip_coef = 0.2
max_grad_norm = 0.5
```

Diese sind das **Florian-Setup-Baseline-Recipe**, ableitbar 1:1 aus seinem 4 vs 5 Kanal-Input.

---

## 10. Konkrete Implementierungs-Empfehlungen (Roadmap)

### 10.1 Die drei nächsten Algorithmen, priorisiert

**Priorität 1: PPO mit Impala-CNN + LayerNorm + PLR, *ohne* manuelles Curriculum.**
Begründung: Größter erwarteter Einzelhebel auf Florians Symptom. PLR ersetzt das Distanz-Curriculum durch ein adaptives, value-loss-getriebenes Curriculum, das nachweislich besser generalisiert (Jiang et al. 2021). Verfügbare Code-Basis: `facebookresearch/level-replay` direkt einbindbar.

**Priorität 2: PPO + DrAC mit Random-Translation/Cutout + BFS-Auxiliary-Head.**
Begründung: DrAC ist auf Procgen-Easy mit ~40 % Boost über PPO empirisch sehr stark. Die BFS-Distance-Prediction als Auxiliary-Loss nutzt die bereits berechneten BFS-Werte gratis und stabilisiert die Repräsentation.

**Priorität 3: PPG (CleanRL-Implementation `ppg_procgen.py`).**
Begründung: Wenn 1+2 nicht ausreichen, ist PPG der kanonische Procgen-SOTA-Schritt. PPG-Reloaded zeigt allerdings, dass viel davon auch durch Regularisierung+Augmentation gewonnen wird — sodass 1+2 oft bereits ähnliche Resultate liefern.

**Nicht empfohlen**: weiter DQN-Tuning, RecurrentPPO ohne klaren Memory-Bedarf, DreamerV3 (Implementierungsaufwand für Bachelor zu hoch).

### 10.2 Architektur — konkretes Design für 15×15×Kanäle

Input: Stack mehrerer Kanäle in `[B, C, 15, 15]`:
- C1: Tile-Type (One-Hot oder Integer + Embedding)
- C2: Visited-Mask (0/1)
- C3: 9 Potential-Field-Features (oder als separater Vektor verarbeiten)

```python
import torch, torch.nn as nn, torch.nn.functional as F

class ResidualBlock(nn.Module):
    def __init__(self, c):
        super().__init__()
        self.ln1 = nn.GroupNorm(1, c)  # LayerNorm-äquivalent für 2D
        self.conv1 = nn.Conv2d(c, c, 3, padding=1)
        self.ln2 = nn.GroupNorm(1, c)
        self.conv2 = nn.Conv2d(c, c, 3, padding=1)
    def forward(self, x):
        h = self.conv1(F.relu(self.ln1(x)))
        h = self.conv2(F.relu(self.ln2(h)))
        return x + h

class ImpalaTinyEncoder(nn.Module):
    """Impala-CNN scaled to 15x15 inputs with GAP (Impoola-style)."""
    def __init__(self, in_channels):
        super().__init__()
        self.stem1 = nn.Conv2d(in_channels, 16, 3, padding=1)   # 15x15
        self.res1  = ResidualBlock(16)
        self.pool1 = nn.MaxPool2d(2, ceil_mode=True)            # 8x8
        self.stem2 = nn.Conv2d(16, 32, 3, padding=1)
        self.res2  = ResidualBlock(32)
        self.pool2 = nn.MaxPool2d(2, ceil_mode=True)            # 4x4
        self.stem3 = nn.Conv2d(32, 32, 3, padding=1)
        self.res3  = ResidualBlock(32)
        # GAP statt Flatten (Impoola)
        self.gap = nn.AdaptiveAvgPool2d(1)
    def forward(self, x):
        x = self.pool1(self.res1(self.stem1(x)))
        x = self.pool2(self.res2(self.stem2(x)))
        x = self.res3(self.stem3(x))
        return self.gap(x).flatten(1)  # [B, 32]

class StoneforgePolicy(nn.Module):
    def __init__(self, n_grid_channels=2, n_scalars=14, n_actions=4):
        # 14 = 5 stats + 9 potential field
        super().__init__()
        self.enc = ImpalaTinyEncoder(n_grid_channels)
        self.scalar_mlp = nn.Sequential(
            nn.Linear(n_scalars, 64), nn.LayerNorm(64), nn.ReLU(),
            nn.Linear(64, 32), nn.ReLU()
        )
        self.trunk = nn.Sequential(
            nn.Linear(32+32, 256), nn.LayerNorm(256), nn.ReLU(),
            nn.Linear(256, 256), nn.LayerNorm(256), nn.ReLU(),
        )
        self.pi   = nn.Linear(256, n_actions)
        self.v    = nn.Linear(256, 1)
        # Auxiliary BFS-distance head
        self.aux_bfs = nn.Linear(256, 1)
    def forward(self, grid, scalars):
        h = torch.cat([self.enc(grid), self.scalar_mlp(scalars)], dim=-1)
        h = self.trunk(h)
        return self.pi(h), self.v(h).squeeze(-1), self.aux_bfs(h).squeeze(-1)
```

Parameter: ~250 k — schlank, schnell, passt zu kleinem Grid.

### 10.3 Training-Budget

- **Phase 1 (Sanity-Check)**: 5 M Timesteps, 16 parallele Envs, ohne Curriculum, **direkt 35–45 Tiles** mit PLR.
- **Phase 2 (Main-Run)**: 25–50 M Timesteps, 64 parallele Envs, mit DrAC + BFS-Aux-Head.
- **Eval alle 200 k Steps** auf den 50 Eval-Seeds.

### 10.4 Hyperparameter-Startwerte (für PPO+Impala+PLR)

| Parameter | Wert | Begründung |
|---|---|---|
| `learning_rate` | 5e-4, linear annealing | CleanRL Procgen-Default |
| `num_envs` | 64 | Procgen-Standard; 8 ist unterdimensioniert |
| `num_steps` | 256 | Rollout-Länge |
| `gamma` | 0.999 | Procgen-Default, NICHT 0.99 |
| `gae_lambda` | 0.95 | Standard |
| `update_epochs` | 3 | Procgen, nicht 10 wie Atari |
| `num_minibatches` | 8 (Batch=2048) | Standard |
| `ent_coef` | 0.01 | Standard |
| `vf_coef` | 0.5 | Standard |
| `clip_coef` | 0.2 | Standard |
| `max_grad_norm` | 0.5 | Standard |
| `aux_bfs_coef` | 0.5 | Florians eigene Idee, normalisierter Huber |
| PLR `staleness_coef` | 0.1 | Jiang et al. |
| PLR `replay_prob` | 0.5 | Jiang et al. |
| PLR `score_transform` | "rank" | Jiang et al. — empirisch bestes |

### 10.5 Libraries — Empfehlung

- **Primär: CleanRL** (`vwxyzjn/cleanrl`). Single-File-Implementations, ideal zum Modifizieren. `ppo_procgen.py` als Template, dazu BFS-Aux-Head und LayerNorm einbauen.
- **PLR**: `facebookresearch/level-replay` als Modul einbinden.
- **DrAC**: `rraileanu/auto-drac` als Referenz.
- **Stable-Baselines3 / sb3-contrib**: für schnelle Baseline-Vergleiche, aber für Custom-Architekturen ist CleanRL praktischer.
- **Sample Factory** (Petrenko et al. 2020): falls Compute-Skalierung gewünscht (1000+ Steps/sec/Env).
- **RLlib**: für reine Forschung overkill, gut für Production.

### 10.6 Empfohlene Reward-Funktion

```
Φ(s) = -BFS(s, exit) / max_distance     # normalisiert in [-1, 0]
F(s, s') = γ · Φ(s') - Φ(s)             # potential-based, policy-invariant
r_total = R_env(s, s')                  # +100 bei Exit, 0 sonst
        + β · F(s, s')                  # β = 0.5..1.0, getunte Stärke
        - 0.01                          # minimale Step-Penalty
# KEINE Stuck/Pendel/Multi-Visit-Penalties.
```

Begründung: Sauber potential-basiert (Ng et al. 1999), normalisiertes Potential vermeidet Reward-Skalen-Probleme mit verschiedenen Distanzen im Curriculum-Wechsel.

### 10.7 Konkrete Aktionsliste für Florian

1. **Curriculum deaktivieren**, stattdessen alle Trainings-Episoden Distanzen 5–45 uniform sampeln (oder PLR-getrieben).
2. **PBRS-konformes Reward-Shaping** (Code oben) statt aktueller ΔBFS-Formel.
3. **Architektur auf Impala-Tiny + GAP + LayerNorm** umstellen (Code oben).
4. **64 parallele Envs** für PPO.
5. **BFS-Distance-Auxiliary-Loss** hinzufügen — bereits berechnete BFS-Werte als Supervisions-Signal.
6. **PLR** einbauen.
7. **5 M Sanity-Run direkt auf 35–45 Tiles** — wenn das nicht funktioniert, ist nicht der Curriculum-Übergang das Problem.
8. Falls weiterhin <30 % SR: **Random-Translation/Cutout-Augmentation + DrAC-Regularizer**.
9. Falls Plasticity-Verdacht (Performance kollabiert nach n M Steps): **CReLU statt ReLU**, plus periodischer Soft-Reset der letzten zwei FC-Layer alle 2 M Steps.

### 10.8 Erwartete Wirkungs-Hierarchie (best guess)

| Maßnahme | Erwarteter SR-Boost auf 35–45 Tiles |
|---|---|
| Curriculum entfernen + uniformes Sampling | +10–20 % |
| PBRS-konforme Shaping-Formel | +5–10 % |
| PLR statt manuelles Curriculum | +15–25 % |
| 64 statt 8 Envs (mehr Datendiversität) | +5–15 % |
| LayerNorm + Impala-CNN | +5–10 % |
| BFS-Auxiliary-Head | +5–15 % |
| Random-Translation Augmentation + DrAC | +10–20 % |
| Summiert (nicht additiv, mit Diminishing Returns) | **Ziel: 60–80 % SR** |

---

## Zusammenfassung in einem Satz

**Florians dringendster Schritt ist nicht ein neuer Algorithmus, sondern (a) das manuelle Distanz-Curriculum durch PLR oder uniformes Sampling zu ersetzen, (b) das BFS-Shaping in mathematisch korrekte potential-basierte Form (Ng et al. 1999) zu bringen, (c) auf Impala-/Impoola-CNN mit LayerNorm umzustellen und (d) DrAC-Augmentation plus eine BFS-Distance-Auxiliary-Head hinzuzufügen — diese Kombination ist auf Procgen-Style-Tasks empirisch belegt der Pfad zu State-of-the-Art Generalisierung, mit PPO als ausreichend starkem Basisalgorithmus und PPG als optionalem späteren Upgrade.**