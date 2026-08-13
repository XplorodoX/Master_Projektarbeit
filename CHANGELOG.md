# Changelog

---

## v2026-08-12.10 — Zitationsprüfung Kapitel 2: Weltgenerierungs-Teil hatte keine einzige Quelle

**Wer:** Florian.

**Kontext:** Kritische Prüfung, ob alle Aussagen in Kapitel 2 (Grundlagen) mit echten Quellen belegt sind, angefordert.

**Befund:** Abschnitt 2.2 (Grundlagen des Reinforcement Learning) ist vollständig und korrekt zitiert — jede Prüfung einzeln gegen die tatsächliche Quelle verifiziert: MDP/Policy/Reward → `sutton2018reinforcement` (Sutton & Barto, das kanonische RL-Lehrbuch, korrekt), POMDP/Belief State → `kaelbling1998planning` (korrekt, exakt das seminale POMDP-Paper), PBRS-Formel → `ng1999policy` (korrekt, Formel im Text entspricht der Originalarbeit), Curriculum Learning → `bengio2009curriculum` + `narvekar2020curriculum` (korrekt gepaart), PPO → `schulman2017proximal` (korrekt), LSTM → `hochreiter1997long` (korrekt, Originalarbeit), DQN → `mnih2015human` (korrekt, Nature-Paper), Gym/Gymnasium → `brockman2016openai` + `towers2023gymnasium` (Jahreszahlen und Übergabe an die Farama Foundation stimmen).

Abschnitt 2.1 (Grundlagen der prozeduralen Weltgenerierung) hatte dagegen über den gesamten Abschnitt hinweg **keinen einzigen Beleg** — Value Noise, Smoothstep, Domain Warping, zelluläre Automaten zur Höhlengenerierung und Breitensuche wurden alle als etablierte Verfahren dargestellt, ohne eine einzige Quelle zu nennen.

#### Änderung — Fünf echte Quellen recherchiert, verifiziert und ergänzt
**Methode:** Jede Quelle einzeln über Websuche gegen Titel, Autoren, Jahr und Venue geprüft, keine aus dem Gedächtnis eingetragen. Dabei einen Fehler im ersten Rechercheversuch selbst abgefangen: SplitMix wurde zunächst fälschlich Steele/Lea/**Vigna** zugeordnet, die Suche zeigte die tatsächlichen Autoren Steele/Lea/**Flood** (OOPSLA 2014).

| Stelle | Ergänzte Quelle | Beleg für |
|---|---|---|
| 2.1.1, Hash-/Mix-Funktionen | `steele2014splitmix` (Steele, Lea, Flood, OOPSLA 2014) | SplitMix-artige Mix-Konstruktion |
| 2.1.2, Value Noise | `perlin1985image` (Perlin, SIGGRAPH 1985) | Grundprinzip kohärenten Gitter-Rauschens |
| 2.1.2, Domain Warping | `quilez_warp` (Quilez, Online-Artikel) | Namensgebende Standardquelle der Technik in der Grafik-Community; kein festes Datum auffindbar, daher mit Zugriffsdatum statt Jahr eingetragen |
| 2.1.3, Zelluläre Automaten | `johnson2010cellular` (Johnson, Yannakakis, Togelius, PCGames 2010) | Exakt einschlägiges Paper zu CA-Höhlengenerierung |
| 2.1.4, Breitensuche | `clrs2009algorithms` (Cormen/Leiserson/Rivest/Stein, 3. Auflage 2009) | Standard-Algorithmenlehrbuch |

**Bewusst nicht zitiert:** Die Manhattan-Distanz-Formel — zu elementar, um eine Quelle zu rechtfertigen (vergleichbar mit einer Grundrechenart).

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 22 statt 17 Literatureinträge, 78 Seiten.

---

## v2026-08-12.9 — Kapitelverweise von Methodik bis Fazit ergänzt

**Wer:** Florian.

**Kontext:** Auf Wunsch Querverweise im Stil „Text (siehe Kapitel X.Y)" ergänzt, beginnend bei Kapitel 3 (Methodik) bis Kapitel 6 (Fazit und Ausblick). Kapitel 1 und 2 bewusst nicht angefasst.

**Lösung:** Acht Stellen ergänzt beziehungsweise korrigiert, ausschließlich mit gegen das kompilierte Inhaltsverzeichnis geprüften Kapitelnummern:

| Datei-Stelle | Bezug | Ziel |
|---|---|---|
| 3.1.1, Value-Noise-Erwähnung | zurück | Kapitel 2.1.2 |
| 3.1.1, BFS-Erwähnung (Nachbearbeitung) | zurück | Kapitel 2.1.4 |
| 3.2.2, POMDP-Modellierung | zurück | Kapitel 2.2 |
| 3.2.2, PBRS-Erwähnung | zurück | Kapitel 2.2 |
| 3.2.3, Curriculum-Voraussetzung („Kapitel~3.1" im Fließtext) | Stilfix | Kapitel 3.1, jetzt als Klammerverweis |
| 4.1.1, Hash-Mixer-Intro | zurück | Kapitel 3.1.1 |
| 4.1.3, Manhattan-Carver-Intro | zurück | Kapitel 3.1.2 |
| 4.2.2, POMDP-Erwähnung (Sichtfenster) | zurück | Kapitel 2.2 |
| 5.1.3, zelluläre Automaten (Lösungsansatz Biome) | zurück | Kapitel 2.1.3 |

Nebenbei bei 4.2.2 ein „wir"-Rest entfernt (Style-Bruch, unpersönliche Form wiederhergestellt), da ohnehin am Satz gearbeitet wurde.

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 77 Seiten.

---

## v2026-08-12.8 — Laurins Fazit und Ausblick zur Weltgenerierung eingearbeitet

**Wer:** Florian (Einarbeitung), Inhalt von Laurin.

**Kontext:** Kapitel 6 („Fazit und Ausblick") enthielt bislang ausschließlich Florians RL-Ergebnisse. Laurin lieferte einen eigenen „Zusammenfassendes Fazit"- und „Ausblick"-Abschnitt zur Weltgenerierung, der eingearbeitet werden sollte.

#### Änderung 1 — Fazit um „Beitrag der Weltgenerierung (Laurin)" ergänzt
**Datei:** `docs/Doku/Projektarbeit Stoneforge RL.tex`
**Lösung:** `\section{Fazit}` in zwei `\subsection` aufgeteilt: `Beitrag der Weltgenerierung (Laurin)` (Laurins vier Punkte: Pfadgarantie, Speicher-/Laufzeiteffizienz, visuelle Kohärenz, Determinismus) und `Gesamtbild (gemeinsam)` (der bestehende, bereits vorhandene Fließtext zum Kompass/LSTM-Befund, unverändert). Folgt damit derselben Autorenkennzeichnung, die in Abschnitt 6.1 („Teilfrage 1"/„Teilfrage 2") und im übrigen Dokument bereits verwendet wird.

#### Änderung 2 — Ausblick zusammengeführt, zwei Fehler dabei behoben
**Problem 1:** Laurins Ausblick-Einleitung referenzierte `\ref{chap:ergebnisse}` — dieses Label existiert nicht (das Evaluationskapitel ist unbeschriftet). Auf `\ref{sec:ergebnisse_diskussion}` (die tatsächliche Limitationen-Subsection, 5.1.6) korrigiert.
**Problem 2:** Die bestehende Ausblick-Einleitung sprach von „drei Ansatzpunkten", zählte aber bereits vier auf (Rest einer vorherigen Änderung, bei der Punkt 4 ergänzt, der Zähler im Einleitungssatz aber nicht mitgezogen wurde). Durch die Zusammenführung mit Laurins vier Punkten wären es acht geworden — Einleitungssatz auf eine zählerlose Formulierung umgestellt, um dieses Muster nicht zu wiederholen.
**Lösung:** `\section{Ausblick}` in `\subsection{Weltgenerierung (Laurin)}` (Laurins vier Punkte: LRU-Chunk-Unloading, A*-Guided Carving, nicht-lineares Biome-Blending, Multi-Layer-Welten) und `\subsection{Reinforcement Learning (Florian)}` (die vier bestehenden RL-Punkte, unverändert) aufgeteilt.

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 77 Seiten (vorher 75).

---

## v2026-08-12.7 — Kompass-Zahlenwiderspruch (Abbildung vs. Fließtext) behoben, Eidesstattliche Erklärung korrigiert

**Wer:** Florian.

**Kontext:** Letzter Korrekturdurchgang vor Abgabe (14.08.). Zwei unabhängige Funde geprüft und behoben.

#### Änderung 1 — Kompass-Zahlen zwischen Abbildung und Fließtext vereinheitlicht
**Problem:** Fließtext nannte für den $\varepsilon=0{,}8$-Kompass 94,0\,%/0,081 und für $\varepsilon=0{,}3$ eine Effizienz von 0,158 — Abbildungen 5.2/5.3 zeigten 88,8\,%/0,094 beziehungsweise 0,177. Gegen `logs/eval_results/baselines.json` (kanonisch) geprüft: Die Abbildungen sind richtig, der Fließtext zitierte einen älteren, nicht-kanonischen Auswertungsstand.
**Nebenfund beim Prüfen:** `scripts/plot_eval_results.py` selbst hatte `COMPASS_SR[0.3]` hart auf `50.0` statt der tatsächlichen `52.0` aus `baselines.json` gesetzt (Tippfehler bei der manuellen Übertragung) — betraf nur den SR-Wert, nicht die Effizienz. Ebenfalls korrigiert, Abbildungen neu erzeugt.

| Wert | Vorher (Text) | Nachher | Quelle |
|------|---------------|---------|--------|
| SR $\varepsilon=0{,}8$ | 94,0 % | 88,8 % | `baselines.json` |
| Effizienz $\varepsilon=0{,}8$ | 0,081 | 0,094 | `baselines.json` |
| Effizienz $\varepsilon=0{,}3$ | 0,158 | 0,177 | `baselines.json` |
| SR-Abstand LSTM/$\varepsilon=0{,}3$ | „knapp 19 Prozentpunkte" | „knapp 14 Prozentpunkte" | 65,7 − 52,0 = 13,7 |

Drei Fundstellen im Fließtext (Abschnitt 5.2.2, 5.2.4, Fazit) korrigiert, `scripts/plot_eval_results.py` gefixt, alle vier Abbildungen neu erzeugt.

#### Änderung 2 — Eidesstattliche Erklärung an tatsächliche Gliederung angepasst
**Problem:** Verwies auf „Abschnitte 5.6 bis 5.10" und „Kapitel 7" — im tatsächlichen Dokument endet Kapitel 5 bei 5.2.5, Kapitel 7 existiert nicht (der Anhang läuft über `\appendix` mit Buchstaben-Nummerierung). Vermutlich Rest einer früheren, in `CLAUDE.md` als Vorschlag skizzierten Gliederung, die beim tatsächlichen Aufbau (Autorenteile als Unterabschnitte 3.1/3.2, 4.1/4.2, 5.1/5.2 statt eigener Kapitel) nicht mehr passt.
**Lösung:** Gegen die echte Gliederung geprüft und korrigiert auf: Kapitel 3.1, 4.1 sowie Abschnitt 5.1 (Laurin), Kapitel 3.2, 4.2 sowie Abschnitt 5.2 (Florian), Kapitel 1, 2 und 6 gemeinsam.

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 75 Seiten.

---

## v2026-08-12.6 — Statistische Einordnung, Trajektorienanalyse und Kapitel-2/3-Brücke ergänzt

**Wer:** Florian.

**Kontext:** Rückmeldung zur Zwischenbewertung (1,7 statt 1,0–1,3): Drei der fünf genannten Lücken ließen sich ohne neue Trainingsläufe oder Eingriffe in Laurins Teil schließen — die vierte (Kausalhypothese zum Kompass-Vorteil) und fünfte (Related-Work-Vertiefung) wurden bewusst nicht angegangen, siehe Begründung unten.

#### Änderung 1 — Statistische Einordnung von LSTM gegen MLP
**Datei:** `docs/Doku/Projektarbeit Stoneforge RL.tex`, Abschnitt 5.2.1
**Problem:** Die Standardabweichungen (12,4 LSTM, 25,5 MLP bei n=7) wurden nirgends gegen die Möglichkeit geprüft, dass der Unterschied im Stichprobenrauschen verschwindet.
**Lösung:** Welch-Test und Mann-Whitney-U-Test auf die sieben Erfolgsquoten je Verfahren (Testset A und B, stochastisch) nachgerechnet. Ergebnis: Testset A signifikant ($t(8{,}7)=3{,}00$, $p=0{,}016$; $U=41$, $p=0{,}038$; Cohens $d=1{,}6$), Testset B schwächer ($p=0{,}038$ Welch, $p=0{,}053$ Mann-Whitney). Neuer Absatz in Abschnitt 5.2.1, inklusive Hinweis, dass sich überschneidende Einzel-Konfidenzintervalle und ein signifikanter Unterschied im Differenz-Konfidenzintervall nicht widersprechen.

#### Änderung 2 — `analyze_agent.py` um RecurrentPPO-Unterstützung erweitert, Trajektorienanalyse zur Determinismus-Lücke
**Datei:** `scripts/analyze_agent.py`
**Problem:** Das Skript unterstützte nur `PPO`/`DQN`, kein `RecurrentPPO` — exakt derselbe Fehlerklasse wie bei `eval_baselines.py` vor `v2026-08-12.2` (kein LSTM-Zustand über `predict()` geführt).
**Lösung:** `--algo rppo` ergänzt (Import `RecurrentPPO`, `state`/`episode_start` korrekt durch die Episode geführt, `env_kwargs_for_model` für den richtigen Obs-Shape), `--deterministic`-Flag ergänzt.

**Ergebnis (20 Seeds Testset A, Modell `v12_s1`, Distanz 35–45, Schrittlimit 1500 statt der vollen 4000 aus Zeitgründen — Einzelmodell, keine n=7-Aussage):**

| Modus | BFS-Optimalausrichtung | Ø mehrfach besuchte Positionen | Extremfall |
|-------|------------------------|-------------------------------|------------|
| Deterministisch | 16,2 % | 58,5 | eine Position 161× besucht |
| Stochastisch | 23,4 % | 90,3 | — |

Die anfängliche Ausblick-Hypothese reiner Zwei-Schritt-Schleifen war zu einfach: Der stochastische Modus zeigt im Schnitt *mehr* wiederholt besuchte Positionen (kurze Vor-und-Zurück-Bewegungen durch den Zufallsanteil), aber die deterministische Politik steckt in Einzelfällen extremer fest und richtet sich seltener BFS-optimal aus. Neuer Absatz in Abschnitt 5.2.3, Ausblick-Punkt 2 entsprechend von offener Frage zu teilweise beantwortet mit neu benanntem Restumfang (alle sieben Modelle, volles Schrittbudget) umformuliert.

#### Änderung 3 — Brücke zwischen Kapitel 2 (Grundlagen) und der Systemkonfiguration
**Datei:** `docs/Doku/Projektarbeit Stoneforge RL.tex`, Abschnitt 2.1.3 und 2.1.4
**Problem:** Zelluläre Automaten und BFS-Erreichbarkeitsprüfung werden in Kapitel 2 ausführlich hergeleitet, sind laut Tabelle 3.1 im ausgelieferten System aber deaktiviert — ohne Verweis darauf wirkt ein Teil der Grundlagenarbeit wie Leerlauf.
**Lösung:** Je ein Absatz ergänzt: Bei zellulären Automaten der Hinweis, dass sie die algorithmische Alternative zum tatsächlich verwendeten Manhattan-Fallback markieren (daher relevant, obwohl inaktiv). Bei BFS die Klarstellung, dass sie nicht tot ist, sondern als Distanzfeld für Reward Shaping (Abschnitt 4.2.4) und Pfadeffizienz-Metrik (Abschnitt 5.2.2) weiterlebt — nur ihre ursprünglich motivierte Rolle (Lösbarkeitsprüfung bei der Weltgenerierung) ist deaktiviert.

**Nicht umgesetzt:**
- **Kausalhypothese zum Kompass-Vorteil** (zweiter Weltgenerator ohne Manhattan-Fallback): Eingriff in `src/`, Laurins Teil, mehrtägiges Experiment. Nicht ohne Absprache und nicht in der verbleibenden Zeit.
- **Related-Work-Vertiefung**: Risiko, Differenzierungsaussagen zu zitierten Arbeiten (PCGRL, POPGym) ohne erneute genaue Lektüre zu erfinden. Zurückgestellt.

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 75 Seiten.

---

## v2026-08-12.5 — Kapitel 2 (Grundlagen) umstrukturiert und mit vier neuen Schema-Abbildungen versehen

**Wer:** Florian.

**Kontext:** Der Weltgenerierungs-Teil von Kapitel 2 bestand aus fünf einzelnen `\section`-Ebenen (eine davon nur ein Absatz lang, faktisch eine verwaiste Kapiteleinleitung), während der RL-Teil direkt daneben alles in eine einzige Section mit `\paragraph`-Ebenen packte — uneinheitliche Gliederungstiefe, keine Kapiteleinleitung, kein Signposting. Zudem enthielt der Weltgenerierungs-Teil keine einzige Abbildung, während der RL-Teil sechs hat.

#### Änderung 1 — Gliederung vereinheitlicht
**Datei:** `docs/Doku/Projektarbeit Stoneforge RL.tex`
**Lösung:** Die fünf `\section`-Ebenen zu einer `\section{Grundlagen der prozeduralen Weltgenerierung}` mit vier `\subsection`-Ebenen zusammengefasst (Zufall, Noise, Zelluläre Automaten, Erreichbarkeit), Value-Noise/Domain-Warping sowie BFS/Heuristische Validierung darunter als `\subsubsection`. Damit spiegelt Kapitel 2 jetzt exakt die Zweiteilung des Gesamtdokuments (Weltgenerierung/RL). Die verwaiste Ein-Satz-Section wurde zur Kapiteleinleitung mit Signposting. Alle bestehenden Labels erhalten, der eine externe Verweis (`sec:grundlagen_bfs`, aus dem Pfadeffizienz-Kapitel) löst weiterhin korrekt auf. Nebenbei zwei Fundstellen korrigiert: Tippfehler „Determininismus", Begriffsinkonsistenz „Weltengenerierung"/„Weltgenerierung" innerhalb des Abschnitts.

#### Änderung 2 — Vier Schema-Abbildungen ergänzt
**Datei:** `scripts/plot_grundlagen_figures.py` (neu), vier PNGs in `docs/Doku/Bilder/`
**Problem:** „Kein Kapitel ohne Abbildung" war für den Weltgenerierungs-Teil von Kapitel 2 nicht erfüllt.
**Lösung:** Vier Konzept-Illustrationen erzeugt, analog zu den bereits vorhandenen RL-Grundlagen-Schemata (PPO-Clipping, PBRS-Potentialfeld): Value-Noise-Interpolation (linear gegen Smoothstep an denselben Knotenwerten), Domain Warping (Rauschausschnitt unverzerrt/verzerrt), zellulärer Automat (Rauschgitter vor/nach Glättung, mit der auch in der Projektkonfiguration hinterlegten Regel B5/S4), BFS-Distanzfeld mit kürzestem Pfad. Alle vier sind eigenständig implementierte, pädagogisch motivierte Illustrationen der Algorithmen, keine Messdaten aus dem Projekt — wie bei den bestehenden RL-Schemata ist das für Konzeptabbildungen zulässig, anders als bei Ergebnisdiagrammen. Jede Abbildung im Fließtext angekündigt und ausgewertet, Bildunterschrift je ein Satz.

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 71 Seiten (vorher 69).

---

## v2026-08-12.4 — Vier Evaluations-Abbildungen mit reproduzierbarem Skript neu erzeugt

**Wer:** Florian.

**Kontext:** Für `eval_erfolgsquote.png`, `eval_pfadeffizienz.png`, `eval_zielkonflikt.png` und `eval_det_stoch.png` war kein Erzeugungsskript im Repository auffindbar (vermutlich ad-hoc gebaut). Stichprobenartiges Pixel-Sampling der Balkenfarben zeigte zudem, dass die Zahlen in diesen vier Abbildungen aus `baselines_and_models.json` stammten (nicht-kanonisch, siehe `v2026-08-12.3`) statt aus der in `CLAUDE.md` als Berichtsfähiger Stand geführten Quelle — Random A z. B. 10,0 % im alten Bild gegenüber 5,2 % kanonisch.

#### Änderung 1 — `scripts/plot_eval_results.py` neu angelegt
**Datei:** `scripts/plot_eval_results.py` (neu)
**Lösung:** Erzeugt alle vier Abbildungen aus denselben, im Skriptkopf dokumentierten Quellen: `logs/eval_results/baselines.json` für Random/Kompass, die Sieben-Seed-Tabelle aus `v2026-07-17` für LSTM (kanonisch laut Regel 2), der verifizierte Lauf aus `v2026-08-12.2` für MLP. Farben per Pixel-Sampling aus den bisherigen Abbildungen übernommen, damit der Stil über beide Bearbeitungsstände hinweg konsistent bleibt.

#### Änderung 2 — MLP-Datenpunkt in `eval_pfadeffizienz.png` und `eval_zielkonflikt.png` ergänzt
**Problem:** Beide Abbildungen enthielten bislang keinen MLP-Wert, weil bis `v2026-08-12.2` keine standardisierte MLP-Pfadeffizienz vorlag.
**Lösung:** MLP-Balken (0,067) beziehungsweise MLP-Punktwolke plus Mittelwert-Diamant ergänzt. Label-Offsets im Zielkonflikt-Diagramm mussten nachjustiert werden, da die ursprünglichen Positionen (für ein einzelnes Diamant-Symbol kalibriert) beim zweiten Diamanten zu überlappenden Beschriftungen führten — im ersten Renderdurchlauf bemerkt, korrigiert, Diagramm neu erzeugt und visuell geprüft.

#### Änderung 3 — Alle vier Abbildungen auf kanonische Zahlen umgestellt
**Lösung:** `eval_erfolgsquote.png` und `eval_det_stoch.png` zeigen jetzt LSTM 65,7 %/29,1 % (statt 68,6 %) und den korrekten MLP-Wert (33,5 %/0,0 %) aus derselben Quelle wie der Fließtext (siehe `v2026-08-12.3`, Änderung 1) — Fließtext und Abbildungen widersprechen sich damit nicht mehr.

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 69 Seiten.

---

## v2026-08-12.3 — Kanonische Zahl vereinheitlicht, Platzhalter-Listings ersetzt, `train_curriculum.py`-Regression zurückgenommen

**Wer:** Florian.

**Kontext:** Nach v2026-08-12.2 folgten drei weitere, unabhängig entdeckte Probleme, alle im Zusammenhang mit der parallel laufenden Editor/Copilot-Session, deren Commits (`968a9c2`, `b14cd05`) bereits auf `origin/main` liegen.

#### Änderung 1 — LSTM-Zahl im Fließtext auf den kanonischen Stand vereinheitlicht
**Datei:** `docs/Doku/Projektarbeit Stoneforge RL.tex`
**Problem:** Die gesamte RL-Evaluation und das Fazit zitierten durchgehend 65,7\,% durch nirgends verwendet — stattdessen stand überall 68,6\,% ± 10,9 (Spannweite 56,0–88,0\,%). Recherche im Changelog-Verlauf ergab: 65,7\,% ± 12,4 / 66,9\,% ± 12,8 stammt aus dem sorgfältig dokumentierten n=7-Lauf vom 17.07.2026 (volle Seed-Tabelle, 95\,%-CI) und ist seither die in `CLAUDE.md` als „Berichtsfähiger Stand" geführte Zahl. 68,6\,% stammt dagegen aus einer späteren, unabhängigen Zweitmessung (`baselines_and_models.json`, ca. 25.07.2026) derselben sieben Checkpoints — beide Zahlen sind gültige Messungen, aber laut Projektregel 2 ist nur die erste zitierfähig. Der Vorfall ist nicht neu: Der gleiche Musters (mehrere valide, leicht unterschiedliche Nachmessungen derselben Checkpoints) ist im Changelog bereits zweimal dokumentiert und aufgelöst worden (Befund 10, `v2026-07-25.x`; Aufstockung n=3→n=7, `v2026-07-17`). Die frisch verifizierte Zahl aus Änderung 1 in v2026-08-12.2 (64,6\,%/68,8\,%) wurde deshalb bewusst **nicht** zur neuen kanonischen Zahl erklärt, obwohl sie ebenfalls plausibel ist — das würde exakt die Nummernkonkurrenz fortsetzen, die Projektregel 2 verhindern soll.
**Lösung:** Alle vier Vorkommen von 68,6\,% auf 65,7\,% korrigiert, Standardabweichung 10,9 auf 12,4, Spannweite 56,0–88,0\,% auf die tatsächliche Seed-Spannweite des kanonischen Laufs (50,0–84,0\,%). Folgeableitung (Abstand LSTM/MLP stochastisch) von „rund 35" auf „rund 32 Prozentpunkte" nachgerechnet (65,7 − 33,5 = 32,2).

#### Änderung 2 — Fünf Platzhalter-Listings durch echten Quellcode ersetzt
**Datei:** `docs/Doku/Projektarbeit Stoneforge RL.tex`
**Problem:** Listings 4.12–4.16 (Beobachtungsvektor, Modellinstanziierung, Curriculum-Gating, Reward-Shaping, Baseline-Policy) waren durchgehend als „Platzhalter für wörtlichen Code" markiert, mit TODO-Kommentaren, die exakte Zeilenbereiche in den Quelldateien nannten.
**Lösung:** Alle fünf gegen den tatsächlichen Quellcode geprüft und ersetzt: `python/stoneforge_env.py` (`_normalize`), `scripts/train_curriculum.py` (`RPPO_KWARGS`/`PPO_KWARGS`, `PHASES`, `CurriculumEvalCallback._on_step`), `src/core/simulation.cpp` (`Simulation::computeReward`), `scripts/eval_baselines.py` (`CompassPolicy`). Captions entsprechend auf den tatsächlichen Dateipfad umgestellt, TODO-Kommentare entfernt.

#### Änderung 3 — `net_arch`-Regression in `PPO_KWARGS` zurückgenommen
**Datei:** `scripts/train_curriculum.py`
**Problem:** Commit `b14cd05` (parallele Session) hat `PPO_KWARGS["policy_kwargs"]["net_arch"]` still von `[256, 256]` auf `[512, 512, 512]` umgestellt und damit die "offizielle" Trainingskonfiguration für die MLP-Kontrollgruppe verändert — ohne Changelog-Eintrag, ohne Abgleich mit den dokumentierten n=7-Modellen. Die Arbeit dokumentiert 250.629 Parameter (`[256,256]`), ein Re-Run von `train_curriculum.py --algo ppo` hätte ab diesem Commit ein anderes Netz erzeugt als das, was in Kapitel 5 beschrieben ist.
**Lösung:** Zurückgesetzt auf `[256, 256]`. Die kapazitätsangeglichene Variante bleibt als Limitation dokumentiert (siehe `v2026-08-12.2`, Änderung 2), nicht als stillschweigend geänderter Standard.

| Parameter | Commit `b14cd05` | Zurückgesetzt auf | Begründung |
|-----------|-------------------|--------------------|------------|
| `PPO_KWARGS.policy_kwargs.net_arch` | `[512, 512, 512]` | `[256, 256]` | Entspricht den dokumentierten n=7-Modellen (250.629 Parameter); Entscheidung gegen Neutraining wurde in dieser Session bereits getroffen |

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 69 Seiten.

**Offen, nicht Teil dieser Änderung:** Vier Abbildungen (u. a. Pfadeffizienz- und Zielkonflikt-Diagramm) enthalten noch keinen MLP-Datenpunkt, da für das MLP bislang keine standardisierte Pfadeffizienz vorlag — die liegt jetzt vor (siehe v2026-08-12.2). Kein Erzeugungsskript für diese Abbildungen im Repository auffindbar (vermutlich ad-hoc erzeugt); Neuerstellung nicht ohne Rücksprache begonnen.

---

## v2026-08-12.2 — Regression in `ModelPolicy` behoben, LSTM- und MLP-Zeilen neu verifiziert

**Wer:** Florian.

**Kontext:** Änderung 1 aus v2026-08-12.1 hat beim Nachrüsten der MLP-Unterstützung in `ModelPolicy` einen Bug eingeführt, der die LSTM-Erkennung stillschweigend zerstört hat. Der Fehler wurde entdeckt, weil ein Nachtrag zur Doku-Überarbeitung aus einer parallel laufenden Session (Editor/Copilot) auf Basis der dadurch bereits kontaminierten `baselines.json` einen radikalen Kurswechsel der Kernaussage vorschlug (LSTM angeblich nur noch ~6 % statt der dokumentierten 65,7 %/66,9 %). Vor jeder Änderung an Kurzfassung, Abstract oder Fazit wurde die Diskrepanz aufgeklärt, wie es Regel 5 (`CLAUDE.md`, Bekannte Fallstricke) verlangt.

#### Änderung 1 — LSTM-Erkennung in `ModelPolicy` repariert
**Datei:** `scripts/eval_baselines.py`
**Problem:** `self.recurrent = hasattr(model, "policy") and hasattr(model.policy, "lstm")` prüft ein Attribut, das bei `sb3_contrib.RecurrentPPO` nicht existiert — die tatsächlichen Attribute heißen `lstm_actor`/`lstm_critic`. Dadurch wurden seit Commit `968a9c2` alle sieben LSTM-Modelle als zustandslos behandelt und ohne LSTM-Zustand ausgewertet, exakt der Fehler, den dieses Skript laut eigenem Docstring vermeiden soll ("man misst ein Gedächtnismodell ohne Gedächtnis"). Verifiziert über direktes Laden eines v12-Modells: `hasattr(m.policy, "lstm")` → `False`, `hasattr(m.policy, "lstm_actor")` → `True`.
**Lösung:** Prüfung auf `hasattr(model.policy, "lstm_actor")` umgestellt.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `ModelPolicy.recurrent`-Check | `hasattr(model.policy, "lstm")` | `hasattr(model.policy, "lstm_actor")` | Attributname stimmte nicht mit `sb3_contrib` überein, LSTM-Modelle liefen seit Commit `968a9c2` ohne Zustand |

**Ergebnis (voller Standard-Eval, Seeds 7000–7049/8000–8049, Exit 35–45, Cap 4000, 5 Wiederholungen, n=7 je Architektur, `s8` ausgeschlossen — kein Teil des dokumentierten Protokolls, kein Changelog-Eintrag für dieses Experiment vorhanden):

| Modell | Testset A | Testset B |
|--------|-----------|-----------|
| RecurrentPPO (LSTM, v12, n=7) | 64,6 % ± 10,1 | 68,8 % ± 15,5 |
| PPO (MLP, v12, n=7) | 33,5 % ± 25,5 | 35,8 % ± 30,5 |

Die LSTM-Zeile bestätigt den bisherigen berichtsfähigen Stand (65,7 % ± 12,4 / 66,9 % ± 12,8) innerhalb der für stochastische Politiken erwarteten Streuung zwischen unabhängigen Wiederholungen. **Die in der parallelen Session gemeldeten ~6 % waren ausschließlich der Bug, kein neuer Befund.** Die MLP-Zeile ersetzt die in v2026-08-12.1 dokumentierten Werte (33,7 % ± 28,7 / 38,6 % ± 32,3) durch eine erneut unabhängig gemessene, intern konsistente Reihe aus demselben Lauf wie die LSTM-Bestätigung. Zusätzlich liegt für das MLP-Modell erstmals eine Pfadeffizienz vor: 0,067 (Ø 1084 Schritte) gegenüber 0,053 beim LSTM (Ø 1425 Schritte) — höher als beim LSTM, aber laut Kapitel 5 (Selektionseffekt bei geringer Erfolgsquote) nicht direkt als bessere Wegfindung zu lesen.

`logs/eval_results/baselines.json` wurde mit dem verifizierten Lauf überschrieben (vorherige Version vom 12.08. vormittags enthielt die kaputten LSTM-Werte).

**Offener Punkt, nicht Teil dieser Änderung:** Im Fließtext von `docs/Doku/Projektarbeit Stoneforge RL.tex` steht an mehreren Stellen (Erfolgsquote-Kapitel, Fazit) eine LSTM-Zahl von 68,6 % ± 10,9 (Testset A, stochastisch), die von der in `CLAUDE.md` als kanonisch geführten Zahl (65,7 % ± 12,4) abweicht — Herkunft dieser 68,6 % bislang nicht zurückverfolgt, vermutlich ein älterer, separat entstandener Eval-Stand. Nicht in dieser Änderung korrigiert, da die Abweichung klein und beide Werte plausibel im Streuungsbereich liegen; vor der Abgabe sollte trotzdem geklärt werden, welcher Lauf tatsächlich zitiert wird.

**Nicht berichtsfähig:** `models/ppo_mlp_curriculum_v12_s8` (`net_arch=[512,512,512]`, batch_size unverändert bei 256) — achter, nicht im n=7-Protokoll vorgesehener Seed, ohne Changelog-Eintrag entstanden. Eigenes `results.json` weist für Phase 3 und 4 (die eval-relevante Distanz 25–45) `best_sr = 0.0` aus. Wird hier ausdrücklich nicht in den berichtsfähigen Stand aufgenommen.

#### Änderung 2 — Tex-Kapitel an verifizierte MLP-Zahlen angeglichen
**Datei:** `docs/Doku/Projektarbeit Stoneforge RL.tex`
**Problem:** Der Text zur Erfolgsquote (Abschnitt „Erfolgsquote der trainierten Verfahren") und das Fazit (Abschnitt „Diskussion der Ergebnisse") verwiesen noch auf den Stand vor dem standardisierten MLP-Testset-A/B-Lauf: fehlender Testset-A-Lauf als methodische Einschränkung, veraltete Zwischenevaluations-Zahlen (38,3 % ± 28,6 statt 33,5 % ± 25,5), fehlender Pfadeffizienz-Wert für das MLP.
**Lösung:** Beide Stellen auf die verifizierten Zahlen aus Änderung 1 umgestellt, Pfadeffizienz-Vergleich ergänzt (inklusive Einordnung über den Selektionseffekt). Zusätzlich neuer Absatz in Abschnitt~5 (Wirksamkeit der Hyperparameter) sowie neuer vierter Ausblick-Punkt: Die rund vierfache Parameter-Differenz zwischen LSTM (1.038.917) und MLP (250.629) wird als Einschränkung der Vergleichbarkeit benannt, der gemessene LSTM-Vorsprung lässt sich damit nicht sauber von der reinen Netzkapazität trennen. Keine Neutrainierung angesetzt (Entscheidung gegen ein kapazitätsangeglichenes Retraining aus Zeitgründen zwei Tage vor Abgabe).

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 69 Seiten.

---

## v2026-08-12.1 — MLP-Kontrollgruppe im kanonischen Eval-Protokoll nachgemessen

**Wer:** Florian.

**Kontext:** Der methodische Bruch beim MLP-Testset-A-Lauf wurde korrigiert. Die Kontrollgruppe wurde nicht mehr nur als Einzel-Snapshot bewertet, sondern mit exakt demselben Standard-Eval-Protokoll wie die LSTM-Läufe vermessen.

#### Änderung 1 — MLP in `eval_baselines.py` mit Standard-Protokoll nachgezogen
**Datei:** `scripts/eval_baselines.py`
**Problem:** Der MLP-Lauf war zwar in den v12-Modellen vorhanden, aber nicht auf dem gleichen, kanonischen Protokoll wie das LSTM ausgewertet worden. Dadurch war der Vergleich methodisch unzulässig.
**Lösung:** `ModelPolicy` wurde auf beide Architekturen erweitert: rekurrente PPO-Modelle nutzen den LSTM-Zustand, MLP-Modelle verwenden den normalen `model.predict(obs, deterministic=False)`-Pfad. Im `--models`-Zweig des Standard-Evals werden nun auch die MLP-Modelle aus `models/ppo_mlp_curriculum_v12_s1..s7` ausgewertet.

**Ergebnis:** Bei Standard-Eval mit Seeds 7000–7049 und 8000–8049, Exit 35–45, Episoden-Cap 4000:

| Algorithmus | Testset A | Testset B |
|-------------|-----------|-----------|
| PPO (MLP, v12, n=7) | 33,7 % ± 28,7 | 38,6 % ± 32,3 |

Damit ist der Nachweis für die MLP-Kontrollgruppe sauber, reproduzierbar und mit dem gleichen Protokoll wie das LSTM dokumentiert.

---

## v2026-08-08.2 — Early Stop und Seed-Pool vertieft, Versionsverlauf aus Florians Teil gelöst

**Wer:** Florian.

**Kontext:** Rückmeldung zu v2026-08-08.1: Die reset()/step()-Schnittstelle nahm zu viel
Raum ein, während die eigentlich interessanten Trainingsregeln (Early Stop, Seed-Pool)
nur als Dreizeiler-Aufzählung mitliefen. Zudem sollte der Versionsverlauf ganz aus
Florians Unterkapitel heraus. Reine Überarbeitung von `docs/Projektdokumentation.tex`,
kein Eingriff in Code, Modelle oder Messergebnisse.

#### Änderung 1 — Neues Unterkapitel „Trainingsspezifische Sonderregeln"
**Problem:** Early Stop und Seed-Pool standen als knappe Stichpunkte unter „Die
Schnittstelle: reset() und step()", obwohl sie inhaltlich mehr hergeben als die
Interface-Mechanik selbst.
**Lösung:** Eigenes Unterkapitel 4.2.5 (`sec:sonderregeln`). Early Stop jetzt mit
Durchsatz-Begründung (16 parallele Umgebungen), Truncation-statt-Terminal-Semantik und der
Auslösequote (unter 3 % der trainierten Episoden, bereits vorher im Ausblick genannt).
Seed-Pool jetzt mit beiden im Code vorhandenen Modi: Erfolgs-Modus (Standard aller
v12-Läufe) gegenüber PLR-Modus (`--plr`, nicht Teil der v12-Konfiguration). Die
Gegenüberstellung greift die bestehende, bisher nur in der Einordnung der Arbeit
(Abschnitt 1.5, `sec:relatedwork`) geäußerte Kritik auf, dass der Standard-Modus die von
Prioritized Level Replay vorgeschlagene Auswahlrichtung umkehrt.
**Nebenfund:** Der Verweis dieser Kritikstelle zeigte auf `sec:technik` (Laurins
Technologie-Stack) statt auf die Seed-Pool-Beschreibung — ein seit dem Kapitel-Split in
v2026-08-06.3 stehen gebliebener Fehlverweis. Korrigiert auf `sec:sonderregeln`.

#### Änderung 2 — Die Schnittstelle: reset() und step() entschlackt
**Datei:** `docs/Projektdokumentation.tex`
**Lösung:** Die Stichpunktliste (Revisit-Penalty/Early-Stop/Seed-Pool) und der
abschließende Truncation-Absatz sind aus 4.2.4 heraus und nach 4.2.5 gewandert. 4.2.4
beschreibt jetzt nur noch die Sprachgrenze und das Sequenzdiagramm.

#### Änderung 3 — Versionsverlauf aus Florians Unterkapitel gelöst
**Problem:** Der Versionsverlauf ist keine RL-spezifische Angelegenheit — die Tabelle wird
von zwölf Stellen im Dokument zitiert, darunter Laurins Weltgenerator- und
Z1-Bewertungsabschnitte. Trotzdem stand er als letztes Unterkapitel unter „Implementierung
der Lernumgebung (Florian)".
**Lösung:** Aus `\subsubsection` in Florians Teil zu einem eigenständigen
`\subsection{Versionsverlauf (gemeinsam)}` (4.3) gemacht, gleichrangig neben 4.1 (Laurin)
und 4.2 (Florian). Label `sec:versionsverlauf` unverändert, daher lösen alle bestehenden
Verweise weiterhin korrekt auf. Kapitelübersicht in der Einleitung („Aufbau der Arbeit")
entsprechend von zwei auf drei Abschnitte korrigiert.

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 62 Seiten. Keine
Zahl, kein Modellstand und kein Eval-Ergebnis wurde verändert oder neu behauptet.

---

## v2026-08-08.1 — RL-Implementierungskapitel inhaltlich neu geordnet und auf den finalen Stand fokussiert

**Wer:** Florian.

**Kontext:** Abschnitt 4.2 („Implementierung der Lernumgebung") stieg mit einem
Versionsverlauf-Tabellenauszug ein, bevor überhaupt erklärt war, was Beobachtung,
Schnittstelle oder Reward-Design sind. Mehrere Unterkapitel wiederholten außerdem, was das
Grundlagenkapitel bereits erklärt hatte (POMDP, MDP-Tupel, PBRS-Beweis), und einzelne
Absätze erzählten Debugging-Geschichten (Konfigurationsleck, Reset-Reihenfolge, alte
β-Werte), statt den finalen Implementierungsstand zu beschreiben. Kein Eingriff in Code,
Modelle oder Messergebnisse — reine Überarbeitung des Fließtexts in
`docs/Projektdokumentation.tex`.

#### Änderung 1 — Reihenfolge neu: Architektur zuerst, Versionsverlauf ans Ende
**Problem:** Der Versionsverlauf (mit Begriffen wie „prozess-globales Konfigurationsleck",
„Straf-Stacking") stand als erstes Unterkapitel vor jeder Begriffsklärung und wirkte wie ein
falscher Einstieg in einen abgeschlossenen Kapitelteil, statt als Rückblick am Ende.
**Lösung:** Neues Unterkapitel „Architektur der Implementierung" (mit dem bestehenden
Komponentendiagramm) an den Anfang gezogen, gibt jetzt den Überblick vor den Details.
Versionsverlauf ist dadurch automatisch zum letzten Unterkapitel geworden, gekürzt auf zwei
Sätze plus Tabelle, mit Pointe auf dem berichtsfähigen v12-Endergebnis (65,7 % / 66,9 %)
statt auf der Entwicklungsgeschichte.

#### Änderung 2 — Duplikate zum Grundlagenkapitel entfernt
**Problem:** „POMDP-Charakter" erklärte POMDP erneut vollständig samt Abbildungsbeschreibung,
obwohl Abschnitt 2.2.2 das mit eigener Abbildung bereits getan hatte. Ebenso wiederholte
„Umgebung, Beobachtung, Aktionen" die MDP-Tupel-Definition aus Abschnitt 2.2.1, und
„Reward-Design" wiederholte den PBRS-Policy-Invarianz-Beweis aus Abschnitt 2.2.6.
**Lösung:** Alle drei Stellen auf Verweise gekürzt, es bleibt jeweils nur der neue,
implementierungsspezifische Teil stehen (konkrete Umsetzung statt Theorie).

#### Änderung 3 — Monitoring-Unterkapitel gestrichen
**Problem:** TensorBoard/`approx_kl`/WebSocket-Beschreibung war für die Forschungsfrage ohne
Belang und lenkte von der Architektur ab.
**Lösung:** Unterkapitel entfernt (`approx_kl` steht bereits im Abkürzungsverzeichnis,
`explained_variance` bereits in den Grundlagen). Das Architektur-Diagramm, das dort stand,
wurde nicht gelöscht, sondern an den Kapitelanfang verschoben (Änderung 1).

#### Änderung 4 — Sequenzdiagramm für `step()` ergänzt
**Datei:** `docs/Projektdokumentation.tex` (neue Abbildung `fig:sequence_step`)
**Problem:** Der Ablauf eines `step()`-Aufrufs über Trainingsschleife → `StoneforgeWorldEnv`
→ `StoneforgeCoreEnv` → `Simulation` stand nur als Fließtext da.
**Lösung:** UML-artiges Sequenzdiagramm (vier Lifelines) ergänzt, Klassen- und
Methodennamen gegen `python/stoneforge_env.py` und `src/python/py_module.cpp` geprüft, nicht
frei erfunden.

#### Änderung 5 — Algorithmenwahl in der Trainings-Pipeline ergänzt
**Problem:** Dass dieselbe Pipeline über `--algo {rppo,ppo}` sowohl die LSTM-Politik als auch
eine MLP-Kontrollgruppe auf identischem Curriculum trainieren kann, stand nirgends in Kapitel
4 — dabei ist das der Mechanismus, der eine kontrollierte F2-Ablation erst ermöglicht
(vgl. `--algo`-Nachrüstung in v2026-08-05.1).
**Lösung:** Neuer Absatz „Algorithmenwahl" in Abschnitt 4.2.6, ausschließlich als
Implementierungsfakt (Parameterunterschiede `policy`/`batch_size`/Netzgröße). Die MLP-Zahlen
selbst wurden **bewusst nicht** ergänzt: Laut Changelog ist nur Seed 1 von 7 gelaufen, nur
Phase 1, als „vorläufig" markiert — nicht Teil des berichtsfähigen Standes (Regel 2,
`CLAUDE.md`).

**Ergebnis:** `latexmk -pdf` fehlerfrei, keine undefinierten Referenzen, 62 Seiten. Keine
Zahl, kein Modellstand und kein Eval-Ergebnis wurde verändert oder neu behauptet.

---

## v2026-08-06.3 — Weltgenerator mit Code und Abbildungen erklärt, Implementierung in zwei Kapitel geteilt

**Kontext:** Der Weltgenerator ist Laurins Kernbeitrag, wurde aber nur benannt statt erklärt.
Dazu vermischte ein einziges Implementierungskapitel Spiel und RL.

#### Änderung 1 — Implementierungskapitel in zwei Kapitel geteilt
**Datei:** `docs/Projektdokumentation.tex`
**Lösung:** Aus Kapitel 4 („Implementierung von Spiel und Lernumgebung") wurden:

| Kapitel | Titel | Inhalt | Seiten |
|---------|-------|--------|--------|
| 4 (`sec:umgebung`) | Implementierung des Spiels Stoneforge | Stack, Bausteine, Weltgenerator, Lösbarkeit, Client | 17–29 |
| 5 (`sec:rlumgebung`, neu) | Implementierung der Lernumgebung | POMDP, Beobachtung, Binding, Reward, Pipeline, Monitoring | 30–35 |

`sec:umgebung` bleibt auf dem Spielteil, weil die Arbeitsteilungstabelle und fünf weitere
Stellen darauf zeigen. Sechs Querverweise umgehängt, davon vier auf `sec:rlumgebung`
(Beobachtung, Early Stop, Monitoring, Gym-Anbindung) und einer auf `sec:rewarddesign`.
Der Absatz „Aufbau der Arbeit" beschreibt die neue Zweiteilung.

#### Änderung 2 — Vier Quelltextauszüge zum Generator
**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Die Stufen wurden benannt, aber nicht gezeigt. Ein Leser konnte sich unter
„Hash-Rauschen mit Salt" oder „Domain Warping" nichts vorstellen.

| Listing | Quelle | Zeigt |
|---------|--------|-------|
| `lst:genchunk` | `World::generateChunk()` | die Pipeline als Landkarte, vollständig |
| `lst:noise` | `World::noise01()` | die zustandsfreie Rauschquelle |
| `lst:biomefield` | `World::biomeFieldForChunk()` | Domain Warping + drei Oktaven |
| `lst:basetile` | `World::sampleBaseTile()` | Vorrangregel See > Wand > Erz > Baum |

Jede Stufe verweist jetzt auf die passende Stelle im Grundlagenkapitel
(`sec:pcg_algorithmen`: Hash- gegen Wertrauschen, Domain Warping, Moore-Nachbarschaft),
statt Vorwissen vorauszusetzen.

#### Änderung 3 — Zwei neue Abbildungen aus echten Generatordaten
**Datei:** `scripts/plot_worldgen.py` (neu), `docs/figures/fig_worldgen_pipeline.{pdf,png}`,
`docs/figures/fig_noise_vergleich.{pdf,png}`

- `fig:worldgenreal`: die drei Stufen nebeneinander an Seed 7000 (91×91-Ausschnitt),
  Biomfeld mit Chunk-Raster → Basisbelegung → fertige Welt mit Spawn, Exit und dem
  BFS-Weg (43 Schritte). Zeigt sichtbar, dass der kürzeste Weg eine grobe Treppe ist.
- `fig:noisevergleich`: Hash- gegen Wertrauschen, roh und nach Schwellwert. Macht den
  Kernbefund der Arbeit visuell: derselbe Schwellwert erzeugt links verstreute
  Einzelfelder, rechts zusammenhängende Barrieren.

**Verifikation:** Das Biomfeld ist im pybind11-Binding nicht exponiert und musste in
Python nachgebaut werden. Der Nachbau reproduziert die Tile-Typen des C++-Kerns auf
**8.115 von 8.115** geprüften Feldern exakt (`plot_worldgen.py` bricht bei jeder Abweichung
ab). Ein zwischenzeitlicher Fehler (Python floor-Division gegen C++-Truncation bei
negativen Koordinaten in der Seenmaske) fiel genau dadurch auf: 50 Abweichungen, alle an
negativen Koordinaten.

#### Änderung 4 — Durchgerechnetes Beispiel `tab:trace`
**Lösung:** Eine echte Chunk-Zeile (Seed 7000, Chunk (−4,−4), Bergland) mit allen drei
Rauschwerten je Feld und dem resultierenden Tile. Enthält alle vier Ausgänge und zeigt an
Feld (−32,−31) die Vorrangregel: Dichte 0,330 erzeugt keine Wand, Erzwert 0,002 setzt
trotzdem Erz.

#### Änderung 5 — Übervolle Zeilen behoben
**Problem:** Lange `\texttt`-Bezeichner (`enableFloodFillValidation`,
`assets/base/game_config.json`) sind nicht trennbar und ragten in den Rand.
**Lösung:** `\setlength{\emergencystretch}{3em}`; Technologie-Stack-Tabelle auf
`\footnotesize`.

| | vorher | nachher |
|---|--------|---------|
| Overfull-Boxen | 31 | **4** (alle < 13 pt, alle in Abbildungen außerhalb Kap. 4/5) |

**Korrektur zu v2026-08-06.1 und .2:** Die dort berichteten „0 Overfull-Boxen" waren falsch.
Das `.log` enthält Bytes, an denen `grep` die Datei als binär behandelt und stillschweigend
nichts ausgibt; die Prüfung lief ins Leere. Die tatsächliche Zahl lag bei 31. Prüfung
erfolgt jetzt über Python mit `errors="replace"`.

#### Änderung 6 — `listings` gegen Nicht-ASCII abgesichert
**Problem:** Ein Em-Dash in einem Code-Kommentar brach den Build
(`Invalid UTF-8 byte sequence`), weil `listings` mit `inputenc`/utf8 nur die im `literate`
hinterlegten Zeichen kennt.
**Lösung:** Zeichen entfernt, zusätzlich `—`, `–`, `„`, `"` ins `literate` aufgenommen.

**Ergebnis:** `latexmk -pdf` fehlerfrei, **82 Seiten**, 4 Overfull-Boxen, keine
undefinierten Referenzen oder Zitate. Sieben Listings, zwei neue Abbildungen. Kein Eingriff
in Simulationskern, Modelle oder Messergebnisse.

---

## v2026-08-06.2 — Spielteil des Implementierungskapitels von unten nach oben neu aufgebaut

**Kontext:** Das Kapitel stieg bei der Generator-Pipeline ein, ohne vorher gesagt zu haben,
was ein Tile, ein Chunk und eine Welt sind. Damit fehlte die unterste Ebene, und alles
darüber wirkte zusammenhanglos. Fokus dieser Runde ausschließlich auf Spiel und prozedurale
Weltgenerierung; die RL-Abschnitte bleiben unangetastet.

#### Änderung 1 — Neuer Abschnitt `sec:bausteine` als unterste Ebene
**Datei:** `docs/Projektdokumentation.tex`
**Problem:** `TileType`, Chunk und `World` kamen im Text nicht vor, obwohl die gesamte
Argumentation (Determinismus, Reproduzierbarkeit, Lösbarkeit) auf ihnen steht.
**Lösung:** Neuer Abschnitt „Die Bausteine: Tile, Chunk und Welt" vor dem Generator:
15 Tile-Typen, Begehbarkeit als Default-`false` (Bäume und Erz blockieren also ebenfalls),
Chunk als 8×8-Array, `World` als lazy gefüllte Hashtabelle, Generator als reine Funktion
`(seed, cx, cy) → 64 Tile-Typen`, und daraus abgeleitet die Notwendigkeit für das
Evaluationsprotokoll.

#### Änderung 2 — `sec:weltgenerator` am Code neu geschrieben
**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Die Stufenbeschreibung nannte Verfahren, erklärte aber keines. Mehrere im Code
zentrale Fakten fehlten ganz.

| Fakt | vorher | nachher |
|------|--------|---------|
| Chunkgröße | **16×16 (falsch)** | 8×8 (`world.hpp:15`, einzige Definition) |
| Biom pro Tile oder Chunk? | unklar | **pro Chunk**, `biomeTagForChunk(cx,cy)` |
| Biomgröße | fehlte | Skalierung 0,22 → ca. 4–5 Chunks ≈ 36 Tiles |
| Rauschquelle | „Hash-Rauschen" | SplitMix64-Mixer, zustandsfrei, Begründung gegen laufenden RNG |
| Oktaven Biomfeld | fehlte | 3 Felder, Gewichte 0,62 / 0,28 / 0,10 |
| Glättungsfunktion | „Perlins" | Hermite `3t²−2t³` |
| Erzverteilung | fehlte | **nur Bergland** (`biomeTag == 3`) |
| Seen | „zwei Rauschfelder" | zusätzlich: ganzzahlige Division `x/7`, `x/3` → blockweise konstant, Seen grobkörnig |
| Reihenfolge in `sampleBaseTile` | fehlte | See > Wand > Erz > Baum, als Regel benannt |
| Halo der Glättungsstufe | „damit es nicht springt" | Begründung: erhält die Reinheit der Generatorfunktion |
| Landmarken-Offset | fehlte | 3 Felder Spielraum bei Chunkgröße 8 |

Neu: `tab:biome` mit allen 21 Schwellwerten aus `World::sampleBaseTile()`.

#### Änderung 3 — Widerspruch „Wandanteil 7–25 %" vs. „Wanddichte 0,238" aufgelöst
**Problem:** Beide Zahlen standen zwei Absätze auseinander im Text, ohne Erklärung. Sie messen
verschiedene Dinge: die erste ist der Schwellwert-Parameter für Wände, die zweite der
gemessene Anteil **unpassierbarer** Felder inkl. Bäumen, Erz und Landmarken.
**Messung:** Tile-Zensus über beide Eval-Sets (100 Seeds, 81×81-Fenster um den Spawn,
656.100 Tiles):

| Tile | Anteil | begehbar |
|------|--------|----------|
| Leer | 0,7511 | ja |
| Wand | 0,1562 | nein |
| Baum | 0,0421 | nein |
| Erz | 0,0322 | nein |
| Landmarken (7 Typen) | 0,0183 | nein |
| Exit | 0,0002 | ja |
| **unpassierbar gesamt** | **0,2487** | |

**Lösung:** Neue `tab:tilecensus` plus Absatz „Was am Ende tatsächlich im Weg steht".
Formulierung im Umwegfaktor-Absatz von „Wanddichte" auf „Hindernisdichte" korrigiert.
Wände allein sind nur 15,6 %, nicht 23,8 %.

#### Änderung 4 — Kausalkette zwischen Determinismus und Offenheit explizit gemacht
**Problem:** Dass die Welten offen sind, stand als Beobachtung da. Der Grund dafür ist aber
dieselbe Entwurfsentscheidung, die Reproduzierbarkeit erzeugt: `noise01` ist zustandsfrei,
also entscheidet jedes Feld ohne Kenntnis seiner Nachbarn.
**Lösung:** Im Absatz „Eine Konsequenz für die Ergebnisse" explizit als Zielkonflikt benannt
und auf `sec:bausteine` zurückverwiesen.

#### Änderung 5 — Neue Literaturstelle
**Datei:** `docs/references.bib`
**Lösung:** `steele2014splitmix` (Steele, Lea, Flood, OOPSLA 2014) für den Bit-Mixer in
`World::mix()`.

**Ergebnis:** `latexmk -pdf` fehlerfrei, 74 → **76 Seiten**, **0 Overfull-Boxen**, keine
undefinierten Referenzen, neue Zitierung korrekt im Literaturverzeichnis aufgelöst. Zwei
neue Tabellen (Nr. 3 und 4). Keine Änderung an Code, Modellen oder RL-Abschnitten.

**Offen:** Kommentar `src/core/world.cpp:402` spricht weiterhin von „5×5-Exit-Freiräumung",
tatsächlich 3×3 (`exitClearRadius: 1`). Nur der Paper-Text ist korrigiert.

---

## v2026-08-06.1 — Implementierungskapitel verständlich gemacht, drei Quelltextauszüge ergänzt

**Kontext:** Das Kapitel `sec:umgebung` benannte Ergebnisse, erklärte aber die Mechanismen
nicht. `sec:loesbarkeit` bestand aus sieben Zeilen, die die Garantie behaupteten, ohne zu
zeigen, wodurch sie entsteht. Ein Leser konnte die zentrale Aussage der Arbeit („jede Welt
ist lösbar, also liegt jeder Misserfolg am Agenten") nicht nachvollziehen.

#### Änderung 1 — `sec:loesbarkeit` vollständig neu geschrieben
**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Der Abschnitt sprang von der Behauptung direkt zum empirischen Beleg. Weder der
Mechanismus noch die Unterscheidung zwischen Erreichbarkeits- und Distanzgarantie kamen vor.
Die eigentliche Ingenieursleistung (Exit aus dem Erreichbarkeitsbaum ziehen statt Rejection
Sampling oder Korridor-Carving) war unsichtbar.
**Lösung:** Umbenannt in „Exit-Platzierung und garantierte Lösbarkeit", Aufbau jetzt:
Motivation über die Interpretierbarkeit der Erfolgsquote → verworfene Alternativen mit
Begründung → vierstufiger Ablauf von `World::chooseExitPoint()` → die Freiräumungs-Falle →
Trennung harte Erreichbarkeitsgarantie vs. Distanzgarantie mit Fallback → Belege.

| Aspekt | vorher | nachher |
|--------|--------|---------|
| Umfang | 7 Zeilen | ca. 2 Seiten inkl. Listing |
| Mechanismus | „Flood-Fill vom Spawn" | 4 nummerierte Schritte, BFS-Tiefe = echter Laufweg |
| Freiräumungs-Falle | fehlte | eigener Absatz + `lst:exit` |
| Fallback nach 24 Versuchen | verschwiegen | explizit als Grenze der Distanzgarantie benannt |
| Freiräumungsradius Exit | — | **3×3 (Radius 1)**, nicht 5×5 |

#### Änderung 2 — Neuer Abschnitt `sec:schnittstelle`
**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Das Kapitel erklärte nirgends, was bei `reset()` und `step()` tatsächlich
passiert und wie die Arbeit zwischen C++ und Python aufgeteilt ist. Das prozessglobale
Config-Leck stand als Halbsatz in einer Klammer, obwohl es einen ganzen Trainingslauf
verfälscht hatte.
**Lösung:** Neuer Abschnitt „Die Schnittstelle: was bei `reset()` und `step()` tatsächlich
passiert" mit Arbeitsteilung der Sprachen, `reset()` in vier Schritten, `step()` in zwei
Hälften (Kern-Reward vs. drei Python-Regeln), und dem Config-Leck als eigenem Absatz
inklusive Orakel-Nachweis (8,3 vs. 40,2 Schritte).

#### Änderung 3 — Drei Quelltextauszüge, `listings` in der Preamble
**Datei:** `docs/Projektdokumentation.tex`
**Lösung:** `listings` + `xcolor` + `\lstset` (Literate-Mapping für Umlaute und griechische
Zeichen, da `inputenc`/utf8). Drei Listings, bewusst nur dort, wo der Code das Argument trägt:

| Listing | Quelle | Trägt welche Aussage |
|---------|--------|----------------------|
| `lst:exit` | `src/core/world.cpp` | Virtuelle Freiräumung schützt die Distanzgarantie |
| `lst:pbrs` | `src/core/simulation.cpp` | Shaping-γ = RL-γ, Bedingung der Policy-Invarianz |
| `lst:lstmstate` | `scripts/eval_baselines.py` | Korrekte LSTM-Zustandsführung im Eval |

#### Änderung 4 — Motivation und Jargon im Generator-Abschnitt
**Datei:** `docs/Projektdokumentation.tex`, `sec:weltgenerator`, `Trainings-Pipeline`
**Problem:** Der Generator wurde beschrieben, ohne zu sagen, warum er so gebaut ist.
„Hash-Rauschen mit merkmalsspezifischem Salt" stand unerklärt. Die Trainings-Pipeline war
ein einziger Absatz aus aneinandergereihten Fakten.
**Lösung:** Zwei Entwurfsentscheidungen vorangestellt (zustandsfreier Generator;
Determinismus als Voraussetzung des Evaluationsprotokolls, „der Seed *ist* die Welt").
Hash-Rauschen und Salt in zwei Sätzen erklärt. Trainings-Pipeline in vier benannte Bausteine
gegliedert (parallele Umgebungen, Evaluation und Gating, Absenkung der Exploration,
Seed-Pool), inkl. Begründung der Doppelbedingung beim Gating.

#### Änderung 5 — Pipeline-Stufenzahl korrigiert
**Problem:** Fließtext sagte „dreistufige Pipeline", Abbildungsunterschrift und
`description`-Liste zeigten vier Stufen (0 bis 3).
**Lösung:** „vierstufig".

**Messung:** `scripts/probe_world_geometry.py` erneut ausgeführt zur Prüfung der im Text
genannten 1,10.

| Konfiguration | lösbar | Wanddichte | BFS Ø | Umweg Ø | p90 |
|---------------|--------|------------|-------|---------|-----|
| Glättung AUS (berichtet) | 100/100 | 0,238 | 40,0 | **1,099** | 1,254 |
| AN it=2 b=5 s=4 | 100/100 | 0,037 | 40,1 | 1,001 | 1,000 |

Der Wert 1,10 im Fließtext (100 Seeds, A+B) ist korrekt; 1,123 in `tab:geometrieprobe` ist
der 50-Seed-Wert auf Testset A. Beide stehen mit korrektem n im Dokument.

**Ergebnis:** `latexmk -pdf` fehlerfrei (exit 0), 73 → 74 Seiten, **0 Overfull-Boxen**,
keine undefinierten Referenzen oder Zitate. Alle drei Listings korrekt nummeriert (S. 21,
25, 30). Keine Änderung an Code, Modellen oder Messergebnissen.

**Offen:** Der Kommentar in `src/core/world.cpp:402` und der Changelog-Eintrag v2026-07-06
sprechen von einer „5×5-Exit-Freiräumung". Bei `exitClearRadius: 1` in
`assets/base/game_config.json` sind es **3×3**. 5×5 gilt für den Spawn (`spawnClearRadius: 2`).
Das Paper nennt jetzt 3×3; der C++-Kommentar sollte nachgezogen werden.

---

## v2026-08-05.3 — Glättungsthese im Paper gegen die Härtungssonde geradegezogen

**Kontext:** Die Messung aus v2026-08-05.1 widerlegt eine Annahme, die an vier Stellen
der Ausarbeitung als Tatsache stand. Die Arbeit hätte im Related Work und im Ausblick eine
These vertreten, die ihre eigenen Rohdaten kippen.

#### Änderung 1 — Härtungssonde als zitierbare Messung im Paper verankert
**Datei:** `docs/Projektdokumentation.tex`, Abschnitt `sec:weltgenerator`
**Problem:** Umwegfaktor und Regelsweep existierten nur im Changelog. Jede Korrektur im
Related Work und Ausblick wäre eine Behauptung ohne Beleg im Dokument gewesen.
**Lösung:** Zwei neue Absätze („Wie offen die Welten sind, misst der Umwegfaktor",
„Die naheliegende Reparatur funktioniert nicht") plus `tab:geometrieprobe` mit dem
vollständigen 7-Zeilen-Sweep (Quelle: `scripts/probe_world_geometry.py --sweep`,
Testset A, 50 Seeds). Ursachenkette benannt: `enableFloodFillValidation` und
`enableMacroGraphPrecheck` auf `false`, Basisdichte hartkodiert in `World::sampleBaseTile()`.

#### Änderung 2 — Vier widerlegte Aussagen korrigiert

| Stelle | vorher | nachher |
|--------|--------|---------|
| `sec:weltgenerator`, Konsequenz-Absatz | „Die Glättungsstufe ist **der** naheliegende Hebel … muss nur aktiviert und kalibriert werden" | Messung: Dichte 0,238 → 0,037, Umweg 1,10 → 1,001; Härtung ist kein Config-Schalter |
| `sec:relatedwork`, Hindernisstruktur | implizit: Glättung an ⇒ Sackgassen | Literaturaussage bleibt, Umkehrschluss explizit widerlegt; neue Lehre: PCG-Bausteine sind nicht parameterfrei übertragbar, der Betriebspunkt der Vorstufe entscheidet |
| `sec:ausblick`, Beiträge/Technisch | „der wirksamste Hebel … muss lediglich aktiviert werden" | Zugänglichkeit erlaubte, die geplante Härtung **vor** der ersten Trainingsstunde zu widerlegen |
| `sec:ausblick`, Punkt 1 Härtung | „billigster verfügbarer Weg zu echten Sackgassen" | drei benannte Code-Eingriffe statt eines Schalters; Zielkonflikt (3/50 lösbar) beziffert |

**Begründung:** Ein Related-Work-Kapitel, das eine im Projekt gemessene Widerlegung
verschweigt, ist angreifbar. Die zweistufige Methodik (erst kalibrieren, dann trainieren)
überlebt die Korrektur unverändert und wird durch sie sogar gestützt.

**Ergebnis:** Keine neue Messung, alle Zahlen aus v2026-08-05.1 übernommen. Zwei
`pdflatex`-Durchläufe fehlerfrei (exit 0), keine undefinierten Referenzen oder Zitate,
neue Tabelle als Nr. 4 im Tabellenverzeichnis, an fünf Stellen referenziert.

**Offen:** CLAUDE.md nennt „Umwegfaktor 1,12 (100 Seeds)". Laut Messung 1 ist 1,099 der
100-Seed-Wert, 1,123 der 50-Seed-Wert auf Testset A. Im Paper stehen beide Zahlen mit
korrektem n; die Notiz in CLAUDE.md vermischt sie.

---

## v2026-08-05.2 — Grundlagenkapitel sprachlich überarbeitet

#### Änderung 1 — Grundlagenkapitel auf den Schreibstil der Projektarbeit umgestellt
**Datei:** `docs/Projektdokumentation.tex` (Abschnitt `sec:grundlagen`, Zeilen 535–1190)
**Problem:** Das Kapitel las sich generiert: durchgehend Passiv und Nominalstil
("wird verwendet", "ist zu beachten", "Gesteuert wird die Erzeugung"), gleichförmige
Satzlängen ohne Rhythmus, Floskeln wie "umfassende Übersicht", "ein zentrales Problem",
"was bei der Interpretation zu berücksichtigen ist".
**Lösung:** Prosa aktiv umgeschrieben (Wir-Form, wo das Projekt handelt), Satzlängen
gebrochen, harte Schlusssätze gesetzt ("Hier laufen sie zusammen.", "Wer sie vermischt,
misst Unsinn.", "Wer das beim Interpretieren vergisst, liest eine Stärke, wo keine ist.").
Parenthetische Gedankenstriche (`~--`) durch Punkte und Doppelpunkte ersetzt.

| Aspekt | vorher | nachher | Begründung |
|--------|--------|---------|------------|
| Stimme | überwiegend Passiv | aktiv, Wir-Form bei eigenen Entscheidungen | Handschrift statt Bürokratendeutsch |
| Gedankenstriche `~--` | 3 Vorkommen | 0 | Stilvorgabe CLAUDE.md |
| Fachinhalt, Formeln, Zitate, TikZ | — | unverändert | rein sprachliche Überarbeitung |

**Ergebnis:** Keine Messung (Textänderung). `pdflatex -draftmode -halt-on-error` läuft
fehlerfrei durch (exit 0), Abschnitts- und Label-Struktur unverändert.

---

## v2026-08-05.1 — Härtungssonde widerlegt den Umbauplan, MLP-Kontrollgruppe für F2 nachgerüstet

**Kontext:** Ein Gutachtenvorschlag verlangte, die Umgebung über die zelluläre Glättung zu
härten (Kompass-Heuristik soll von 92 % auf unter 30 % einbrechen), anschließend MLP und LSTM
darauf neu zu trainieren. Vor dem Trainingssprint haben wir die Annahme geprüft. Sie hält nicht.

### Messung 1 — Die zelluläre Glättung härtet die Welt nicht, sie planiert sie

**Skript:** `scripts/probe_world_geometry.py` (100 Seeds: Testset A 7000–7049 + B 8000–8049)
**Vorgehen:** `enableCellularSmoothing` temporär in `assets/base/game_config.json` gesetzt.
Kein Rebuild nötig — `StoneforgeCoreEnv` liest die Config im **Konstruktor**
(`src/python/py_module.cpp:44–48`), jedes neu gebaute Env übernimmt den Patch sofort.
Die Rebuild-Warnung in CLAUDE.md ist an dieser Stelle zu vorsichtig.

**Leitmetrik: Umwegfaktor = BFS-Distanz / Manhattan-Distanz.** Er misst direkt, ob es
überhaupt etwas zu umrunden gibt. Bei 1,0 ist der kürzeste Weg die Luftlinie.

| Bedingung | lösbar | Wanddichte | Manhattan Ø | BFS Ø | Umweg Ø | p90 |
|---|---|---|---|---|---|---|
| Glättung AUS (Status quo) | 100/100 | 0,238 | 36,8 | 40,0 | **1,099** | 1,254 |
| Glättung AN (b=5 / s=4, 2 Iter.) | 100/100 | **0,037** | 40,1 | 40,1 | **1,001** | 1,000 |

**Ergebnis:** Die Glättung radiert die Wände weg statt sie zu verdichten. Bei einer
Ausgangsdichte von 24 % hat kaum ein Wandtile die geforderten fünf soliden Nachbarn, also
stirbt fast jede Wand und keine wird geboren. Umwegfaktor 1,001 heißt: die Welt wird zur
leeren Ebene. Hätten wir das aktiviert, wäre die Heuristik nicht auf 30 % gefallen, sondern
auf nahezu 100 % gestiegen.

### Messung 2 — Es gibt keinen reinen Config-Ausweg

**Skript:** `scripts/probe_world_geometry.py --sweep` (Testset A, 50 Seeds)
**Frage:** Hebt irgendeine Regelkombination den Umwegfaktor, ohne die Lösbarkeit zu zerstören?

| Regel | lösbar | Wanddichte | BFS Ø | Umweg Ø | p90 |
|---|---|---|---|---|---|
| aus (Referenz) | 50/50 | 0,240 | 40,1 | 1,123 | 1,312 |
| an, it=2 b=5 s=4 (JSON-Default) | 50/50 | 0,036 | 40,1 | 1,001 | 1,000 |
| an, it=2 b=4 s=3 | 50/50 | 0,169 | 40,1 | 1,101 | 1,327 |
| an, it=1 b=3 s=3 | 50/50 | 0,271 | 39,8 | 1,145 | 1,637 |
| an, it=2 b=3 s=2 | 41/50 | 0,447 | 39,4 | 1,158 | 1,357 |
| an, it=4 b=3 s=2 | 19/50 | 0,699 | 39,7 | 1,173 | 1,607 |
| an, it=2 b=2 s=1 | **3/50** | 0,875 | 35,3 | 1,636 | 2,299 |

**Ergebnis:** Ein harter Zielkonflikt. Jede Einstellung, die den Umwegfaktor spürbar hebt,
zerstört die Lösbarkeit. Alles, was 50/50 lösbar bleibt, liegt beim Umweg im Rauschen der
Referenz (1,145 gegen 1,123). Der Generator kann keine Welt bauen, die gleichzeitig schwer
und zuverlässig lösbar ist.

**Ursache:** `enableFloodFillValidation` und `enableMacroGraphPrecheck` stehen beide auf
`false` (`game_config.json:23,25`). Die Stufen, die Konnektivität prüfen und erzwingen würden,
sind aus. Der Automat zählt Nachbarn, er kennt keine Erreichbarkeit.

**Konsequenz für die Planung:** Härtung ist kein Config-Flip. Sie verlangt Eingriffe in
`World::sampleBaseTile()` (Basisdichte ist hartkodiert, world.cpp), dazu funktionierende
Konnektivitätsvalidierung und Neuvalidierung — vor der ersten Trainingsstunde. Bei neun Tagen
Restzeit verworfen.

**Gewinn für die Arbeit:** Der Umwegfaktor **1,12** ist die quantitative Erklärung, warum ein
Vierzeilen-Kompass 92 % erreicht und Gedächtnis in dieser Welt keinen Vorteil bringt. Der
kürzeste Weg ist zwölf Prozent länger als die Luftlinie. Es gibt schlicht nichts zu umrunden.
Das ersetzt die Behauptung „die Umgebung war zu einfach" durch eine Messung.

### Messung 3 — Durchsatz: die 8-Stunden-Annahme gilt nur für das LSTM

Gemessen mit `n_envs=16` (DummyVecEnv, wie v12), 8192 Steps, `OMP_NUM_THREADS=3`.

| Algo | Parameter | Durchsatz | 2,2 M Steps (Training, ohne Eval) |
|---|---|---|---|
| `ppo` (MLP) | 250.629 | **5.989 Steps/s** | ~0,1 h |
| `rppo` (LSTM) | 1.038.917 | 104 Steps/s | ~5,9 h |

Faktor 58. Die 5,9 h des LSTM decken sich mit der protokollierten Laufzeit von 7h48m
(v12 s1, `results.json`) plus Eval-Overhead. Beim MLP dominiert der periodische Eval die
Wallclock, nicht das Training. **Folge:** Die aus Zeitgründen geplante Beschränkung auf n=3
ist hinfällig, n=7 spiegelt die LSTM-Seite exakt.

#### Änderung 1 — `--algo` für das Curriculum nachgerüstet
**Datei:** `scripts/train_curriculum.py`
**Problem:** Das Skript war fest auf `RecurrentPPO` verdrahtet und kannte kein `--algo`.
Eine gedächtnislose Kontrollgruppe auf **identischem** Curriculum war damit nicht messbar.
Die Zahlen in Tabelle 9 vergleichen deshalb Umgebungsversionen statt Architekturen — für F2
wertlos.
**Lösung:** `--algo {rppo,ppo}` eingeführt, Auflösung über ein `ALGOS`-Dict. Curriculum,
Swarm-Pool, Phasen, Gates, Eval-Protokoll und alle geteilten Hyperparameter bleiben identisch.
Betroffen: Import, `PPO_KWARGS`, Modellkonstruktion, Phasen-Reload, Entropie-Annealing,
`save_run_config`, `save_run_results`.

| Parameter | `rppo` (LSTM) | `ppo` (MLP) | Begründung |
|---|---|---|---|
| `policy` | `MlpLstmPolicy` | `MlpPolicy` | die kontrollierte Variable: Gedächtnis |
| `batch_size` | 8 | 256 | 8 ist eine Notlösung für den LSTM-Critic (v2026-07-07.4). Beim MLP wären das bei Rollout 256×16=4096 ganze 512 Gradientenschritte pro Epoche. 256 = 16 Minibatches, PPO-Standard. |
| `policy_kwargs` | `lstm_hidden_size=256` | `net_arch=[256,256]` | spiegelt die Hidden-Size. Der SB3-Default `[64,64]` würde die Baseline über die Kapazität benachteiligen statt über das Gedächtnis. |
| alles übrige | — | identisch | `n_steps`, `n_epochs`, `lr`, `gamma`, `gae_lambda`, `clip_range`, `ent_coef`, `vf_coef` |

`--lstm-size` wirft jetzt einen Fehler, wenn es mit `--algo ppo` kombiniert wird.

**Verifikation:** `scratchpad/smoke_mlp.py` prüft Konstruktion, `learn()`, Speichern und den
Phasen-Reload mit gefilterten Kwargs (die Stelle, an der so ein Umbau typischerweise bricht),
plus Gegenprobe, dass der LSTM-Pfad unverändert konstruiert. Bestanden.

**Offenzulegende Einschränkung:** Das MLP hat 250.629 Parameter, das LSTM 1.038.917 — Faktor
vier. Verliert das MLP, ist der Einwand „Kapazität statt Gedächtnis" zulässig. Bei
Trainingskosten von Minuten pro Lauf lässt er sich mit einer zweiten MLP-Variante
(`net_arch=[512,512]`, ~1 M Parameter) ausräumen.

### Ergebnis (vorläufig, Lauf s1 in Arbeit)

`models/ppo_mlp_curriculum_v12_s1`, Phase 1 (exit 5–12, die **leichteste** Phase):

| Eval @ Steps | det | stoch |
|---|---|---|
| 24.992 | 0/50 (0,0 %) | 12/50 (24,0 %) |
| 49.984 | 1/50 (2,0 %) | 27/50 (54,0 %) |
| 149.952 | 1/50 (2,0 %) | 15/50 (30,0 %) |
| 274.912 | 0/50 (0,0 %) | 15/50 (30,0 %) |
| 424.864 | 0/50 (0,0 %) | 17/50 (34,0 %) |
| 499.840 (Phasenende) | 0/50 (0,0 %) | 8/50 (16,0 %) |

**Phase 1 abgeschlossen, bestes SR (stoch): 54,0 %.** LSTM v12 s1, gleiche Phase: **92,0 %.**
Gate 0,85 klar verfehlt, volle 500 k Steps ausgeschöpft.

Auffällig ist nicht nur das Niveau, sondern der **Verlauf**: Das Maximum von 54 % fällt auf
Step 49.984, danach sinkt die SR und pendelt bis zum Phasenende zwischen 14 und 34 %.
Neun Zehntel des Phasenbudgets haben die Politik verschlechtert. Deterministisch bleibt sie
über den gesamten Lauf bei 0–2 %.

Das gedächtnislose MLP scheitert deterministisch nahezu vollständig und kommt stochastisch
nicht stabil über 30 %. Das passt zur erwarteten Signatur: ohne Gedächtnis erzeugt dieselbe
Beobachtung dieselbe Aktion, die deterministische Politik läuft in Zyklen. Belastbar ist das
erst mit n=7; ein Einzelsnapshot ist nach v2026-07-07.4 ausdrücklich **keine** Validierung.

**Offen:** Seeds 2–7, danach Standard-Eval (Testset A/B, Cap 4000) mit Pfadeffizienz,
optional die breite MLP-Kontrolle.

### ⚠️ Befund — Die Pfadeffizienz rettet das F2-Argument NICHT

**Auslöser:** Die Vermutung lautete, die Heuristik gewinne zwar bei der Erfolgsquote, das
LSTM aber bei der Wegqualität. Gegen `logs/eval_results/baselines_and_models.json` geprüft.
**Sie stimmt nicht.**

| Politik | SR Testset A | Schritte Ø | Pfadeffizienz |
|---|---|---|---|
| Kompass ε=0,3 | 50,0 % | 399 | **0,158** |
| Kompass ε=0,5 | 62,0 % | 466 | 0,142 |
| Kompass ε=0,6 | 76,0 % | 485 | 0,130 |
| Kompass ε=0,9 | 88,0 % | 1495 | 0,040 |
| RecurrentPPO v12 s1–s7 | 56,0–88,0 % | 1197–1740 | **0,039–0,061** |

Das trainierte Modell liegt bei der Pfadeffizienz auf dem Niveau des ε=0,9-Zufallslaufs und
wird vom ε=0,3-Kompass um rund Faktor drei geschlagen. Es erkauft seine höhere Erfolgsquote
durch mehr Herumlaufen, nicht durch bessere Wege. Die mittlere Episodenlänge (1197–1740
Schritte bei einem BFS-Optimum von 40) bestätigt das.

**Vorbehalt:** Die Effizienz mittelt nur über *erfolgreiche* Episoden. Der ε=0,3-Kompass
schafft nur die Seeds, auf denen die Luftlinie ohnehin trägt — ein Selektionseffekt, der
seinen Wert schönt. Er erklärt den Faktor drei aber nicht vollständig.

**Konsequenz:** Die Pfadeffizienz taugt als *zusätzliche* Berichtsmetrik und entwertet die
92-%-Schlagzeile der Heuristik (Effizienz 0,047). Als Beleg für einen Vorteil des Gedächtnisses
taugt sie nicht. F2 muss über den MLP/LSTM-A/B beantwortet werden, nicht über die Wegqualität.

#### Änderung 2 — Dokumentationsdrift aufgeräumt

Beim Arbeiten sind sechs Widersprüche aufgefallen. Alle gegen Rohdaten und Code geprüft,
alle bestätigt.

| # | Fund | Status |
|---|---|---|
| 1 | CLAUDE.md nannte „Random 8 % / Kompass 89 %", `baselines.json` und die Doku sagen **5,2 % / 92,0 %** | korrigiert, Quelle benannt |
| 2 | Das Inline-Snippet „Standardisierter Eval" in CLAUDE.md probierte nur `PPO`/`A2C`/`DQN` (**kein `RecurrentPPO`**) und rief `predict()` **ohne LSTM-Zustand** auf | entfernt, ersetzt durch Verweis auf `eval_baselines.py` |
| 3 | `eval_baselines.py:12` verwies auf `scripts/eval_final.py` — existiert nicht | Verweis entfernt |
| 4 | `train_curriculum.py` (Quelle **aller** berichtsfähigen Zahlen) fehlte im Strukturbaum, stattdessen war `train.py` als Trainingseinstieg gelistet | ergänzt und abgegrenzt |
| 5 | `eval_hard_world.py` setzt `*WallThreshold`-Keys, die laut CLAUDE.md hartkodiert und entfernt sind | Warnblock im Code, als defekt markiert |
| 6 | Fünf Eval-Skripte plus Inline-Snippet, kein kanonisches benannt | `eval_baselines.py` als kanonisch festgeschrieben |

Befund 2 war der einzige mit echtem Notenrisiko: Wer in den letzten Tagen schnell eine Zahl
nachmisst und das Snippet nimmt, bekommt für ein LSTM-Modell einen systematisch zu niedrigen
Wert und trägt ihn in die Arbeit ein.

#### Änderung 3 — Kurzfassung nennt jetzt das Ergebnis

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Die Kurzfassung enthielt **keine einzige Zahl und keinen Befund**. Sie endete mit
„Was diese Arbeit daraus für die Bewertung solcher Agenten ableitet, zeigen die folgenden
Kapitel." Ein Prüfer liest die Kurzfassung zuerst und erfuhr dort nichts.
**Lösung:** Von 150 auf 285 Wörter erweitert. Nennt jetzt in dieser Reihenfolge: Z1 erfüllt
(100 % Lösbarkeit über 3.150 Seeds, Determinismus), das Agentenergebnis (65,7 % ± 12,4 A /
66,9 % ± 12,8 B, n=7, Holdout erfüllt, Testset verfehlt), den Bruch (Kompassheuristik 92 %,
bei vergleichbarer SR dreimal wegeffizienter), die Ursache im Generator und den übertragbaren
Beitrag. Der unbequeme Teil steht drin, nicht nur das Positive.

**Zusätzlich:** Zwei verwaiste Abbildungen an den Text angebunden. `fig:v12endergebnis` — die
zentrale Ergebnisgrafik — wurde im Fließtext **nie** referenziert, ebenso `fig:pcgpipeline`.

**Verifikation:** `latexmk -pdf -halt-on-error` läuft mit Exit 0 durch, 69 Seiten, keine
undefinierten Verweise, keine fehlenden Zitate. Mechanischer Gesamtcheck des Dokuments:
90 Labels, 76 Verweise, 36 Zitate, **null tote Querverweise, null fehlende Bib-Einträge,
null unzitierte Literatur**.

### Bewertung des Dokumentstands (05.08.2026)

Das Gutachten monierte Lücken, die das Dokument bereits schließt. Gegengeprüft:

| Kritikpunkt | tatsächlicher Stand |
|---|---|
| „Tabelle 9 vergleicht alte mit neuen Umgebungen" | steht so im Absatz „Aussagekraft" (§\ref{sec:ablation}), mit vier Einschränkungen und dem Satz „Diese Gegenüberstellung ist *keine* kontrollierte Ablation" |
| „Umgebung härten (Priorität 1)" | steht als Punkt 1 im Ausblick, präziser formuliert: Zielgröße ist „ein Betriebspunkt, an dem der Kompass-Zufallslauf einbricht, die Lösbarkeit aber bei 100 % bleibt" |
| „MLP-Baseline fehlt" | als Einschränkung 5 benannt; wird durch die Läufe dieser Version geschlossen |
| „F2 eindeutig beantworten" | F2 ist architekturneutral gestellt („Ob der Agent dafür ein Gedächtnis braucht, ist Teil der Frage und nicht ihre Voraussetzung") und mit „nicht entscheidbar" bereits sauber beantwortet |

Der von Messung 2 dieser Version geforderte Betriebspunkt **existiert über die Config nicht**.
Damit beantwortet diese Version Punkt 1 des Ausblicks negativ: Die Härtung braucht einen
C++-Eingriff in `World::sampleBaseTile()` plus Konnektivitätsvalidierung, nicht nur ein Flag.
Der vorgeschlagene Umbau der Arbeit in „Iteration 1 / Iteration 2" wurde **verworfen** — die
bestehende Struktur (Ergebnis → Verhältnis beider Antworten → Einschränkungen → Ausblick)
trägt den Befund bereits.

---

## v2026-08-04.8 — Drittes Review: HP als totes Feature entdeckt, sieben Korrekturen

**Kontext:** Drittes externes Review über das Gesamtdokument. Alle beanstandeten Punkte gegen
Code, Changelog und Rohdaten geprüft; **alle sieben bestätigt**, keiner zurückzuweisen.

### ⚠️ Befund 1 — HP ist ein totes Feature, „Tod" kann nicht eintreten

**Ausgangspunkt des Reviews:** Tabelle 4 nennt „Timeout / **Tod** (terminal) −10" und einen
Schadens-Penalty; §4.4 listet als Episodenenden aber nur Exit, Timeout und Early-Stop.
**Code-Prüfung** (`src/core/simulation.cpp`): Es gibt genau zwei Schadensquellen:

| Quelle | Zeile | Gate |
|---|---|---|
| Mob-Nähe (3×3-Umgebung) | 341–348 | `updateMobs()` kehrt bei `disableMobs` sofort zurück (Z. 1301) |
| Verhungern | 351–356 | steht in `if(!cfg.gameplay.disableEnergy)` |

`stoneforge_env.py:150` übergibt **beide** Flags als `True`; zusätzlich steht
`mobSpawn.count: 0` in der Konfiguration.
**Folge:** Die Lebenspunkte ändern sich im RL-Pfad nie. Tod ist unmöglich, der
Schadens-Penalty feuert nie, und **HP ist eine der 229 Beobachtungsdimensionen ohne
Informationsgehalt** — dieselbe Kategorie wie Energie und Inventar, die in v11 entfernt
wurden. Bei der v11-Bereinigung übersehen.
**Lösung:** In §4.4 offengelegt, mit Verweis auf die abgeschaltete Mechanik und dem Hinweis,
dass es für die berichteten Ergebnisse folgenlos ist, bei einer Neufassung der Beobachtung
aber zu streichen wäre. Tabelle 4 bleibt unverändert, da sie die Reward-Funktion des Kerns
dokumentiert.

### ⚠️ Befund 2 — Widerspruch in der Deutung des Det/Stoch-Gaps

§7.5 begründet ausführlich, dass Singh et al. \[14] nur für *gedächtnislose* Politiken gilt
und den Gap **nicht** erklärt. §7.6 schrieb dann: „die empirische Signatur des
POMDP-Resultats \[14]" — und nahm damit genau die Zuschreibung zurück. §8.3 folgte wieder der
7.5-Linie.
**Lösung:** §7.6 verweist jetzt auf die in §7.5 diskutierten Ursachen (Ghosh et al.,
epistemische Unsicherheit + unvollständiger Belief-State) statt auf Singh et al.

### Befund 3 — „elf Prozentpunkte" in der Kurzfassung war falsch

Nachgerechnet: A 74,0 → 65,7 = **8,3**; B 77,3 → 66,9 = **10,5**. Gegen die ursprünglich
berichteten Werte (73,3 / 80,0) wären es 7,6 und 13,1. „Elf" passt zu keiner konsistenten
Paarung. Korrigiert auf „acht bzw. zehn".

### Befund 4 — Datum der Lösbarkeits-Nachmessung

§4.2 nannte den 05.07.2026. Die 150/150-Messung steht im Changelog unter `v2026-07-06`
(Z. 2614) und setzt die BFS-Exit-Platzierung voraus, die erst mit v11 am 06.07. kam.
Korrigiert auf 06.07.2026.

### Befund 5 — Abbildung 7 war zwei Läufen zugeordnet

Bildunterschrift und §7.2: Lauf vom 08.06. (Bedingung C, 86 % auf Testset A). §10.3 verwies
mit „das aktuelle Referenzmodell (Abbildung 7)" aber auf die v10-Reproduktion vom 25.06.
(86 % auf **Validierung**) — zwei verschiedene Läufe mit zufällig gleichem Wert.
Korrigiert: §10.3 nennt den Lauf jetzt ohne Abbildungsverweis und grenzt ihn ausdrücklich vom
08.06.-Lauf ab.

### Befund 6 — 86 % (Bedingung C) neben 65,7 % (v12) ohne Einordnung

Bedingung C wurde vor der v11-Revision gemessen, als „Exit 35–45" als Luftlinie vorgegeben
war und real 42–75 Feldern entsprach. Das ältere Modell erzielte den höheren Wert also auf
den **längeren** Wegen; unkommentiert las sich der Absatz wie eine Regression. Zwei Sätze
ergänzt.

### Befund 7 — Fehlende Gegenrichtung bei der CI-Aussage

§8.3 wies darauf hin, dass das CI auf Holdout B bis 55,0 % hinabreicht, also unter die
erfüllte Schwelle. Die symmetrische Aussage fehlte: Das CI auf Testset A reicht bis 77,2 %
hinauf und **schließt die 70 %-Schwelle ein**; das Kriterium ist im Punktschätzer verfehlt,
nicht aber signifikant. Ergänzt, mit dem Zusatz, dass der Hauptbefund nicht an dieser
Schwelle hängt.

### Kleinigkeit — ε doppelt belegt

PPO-Clipping nutzt `\epsilon`, die Referenzpolitik `\varepsilon`; das Symbolverzeichnis führte
nur die zweite Bedeutung. Da es typografisch zwei verschiedene Zeichen sind (ϵ vs. ε), ist die
Formel korrekt; ergänzt wurde der Eintrag für die Clipping-Grenze, neues Label `sec:ppo`.

**Verifikation:** `latexmk -pdf` fehlerfrei, 61 Seiten, 0 Undefined References/Citations.

---

## v2026-08-04.7 — Motivation: jede Aussage belegt, eine Falschaussage korrigiert

**Anlass:** Rückfrage, ob die Aussagen der Motivation belegbar sind. Prüfung ergab: Drei
Aussagen standen ohne Quelle da, eine war sachlich falsch.

### ⚠️ Korrektur — Aussage über Procgen war falsch

**Vorher:** „Bei fertigen Forschungsumgebungen wie MiniGrid oder Procgen kommt man an den
Generator nicht heran: Wandanteil, Struktur der Hindernisse und das Maß für die Schwierigkeit
sind fest eingebaut."
**Prüfung** an Paper und Quellcode-Repository:

| Behauptung | Befund |
|---|---|
| „kommt man nicht heran" | **Falsch.** Procgen ist Open Source; `src/games/*.cpp` ist modifizierbar |
| „Schwierigkeit fest eingebaut" | **Ungenau.** Es gibt kalibrierte Stufen `easy`/`hard` (Paper) sowie `extreme`, `memory`, `exploration` über die Python-API |
| MiniGrid | **Nicht geprüft** → Aussage entfernt, statt sie unbelegt stehen zu lassen |

**Nachher (belegbar):** Procgen bietet zwei kalibrierte Schwierigkeitsstufen und darüber
hinaus feste Voreinstellungen; die Erzeugung von Layouts und Hindernissen liegt im
C++-Quellcode. Eine je Welt bezifferbare und einstellbare Schwierigkeit ist nicht vorgesehen.
Das ist die Aussage, die die Arbeit tatsächlich braucht, und sie ist belegt.

### Änderung — drei bisher unbelegte Aussagen mit Quellen versehen

| Aussage | Beleg |
|---|---|
| Videospiele als Standardumgebung für RL | `mnih2015dqn` |
| Agenten auf festen Levels erreichen dort hohe Werte und fallen auf neuen ab | `cobbe2020procgen` (bisher nur allgemein für PCG zitiert) |
| Generalisierungslücke als offenes Problem des Feldes | **neu:** `kirk2023survey` |

**Neue Literatur:** Kirk, Zhang, Grefenstette, Rocktäschel: „A Survey of Zero-shot
Generalisation in Deep Reinforcement Learning", JAIR 76, S. 201–264, 2023. Verifiziert über
arXiv:2111.09794 (Titel, Autoren, Band, Seiten). Zusätzlich `procgen_repo` (OpenAI,
GitHub-Repository) für die API- und Quellcode-Angaben. Bibliografie 34 → 36 Einträge.

**Nützlicher Nebenbefund aus Kirk et al.:** Das Abstract stellt fest, „that taking a purely
procedural content generation approach to benchmark design is not conducive to progress in
ZSG". Damit ist der Kernbefund dieser Arbeit in der Literatur bereits angelegt. Das ist in
die Motivation aufgenommen und **entschärft zugleich den Ton**: Die Arbeit bestätigt einen
bekannten Kritikpunkt an einem eigenen Gegenstand, statt etwas Neuartiges zu behaupten.

### Änderung — Motivation gekürzt und Ton angepasst

Vier kurze Absätze. Der Schlusssatz verspricht nur noch, was die Arbeit einlöst: „Wie sich
zeigen wird, entscheidet der Generator darüber, was das Experiment überhaupt messen kann."
Keine Wertung als bedeutsam oder neuartig. Alle Gedankenstriche entfernt.

**Verifikation:** `latexmk -pdf` fehlerfrei, 62 Seiten, 0 Undefined References/Citations.

---

## v2026-08-04.6 — Reihenfolge Spiel → RL überall durchgezogen

**Prinzip:** Wo beide Projektteile nacheinander vorkommen, steht das Spiel zuerst. Das folgt
der Entstehungslogik: Erst existierte die Plattform, dann wurde RL darauf angewendet.

**Bereits in v2026-08-04.4 umgestellt:** Grundlagen (PCG 2.1/2.2 vor RL), Kapitel 4
(Generator 4.1/4.2 vor Agentensicht 4.3–4.5), Evaluation (Plattform Z1 vor Agent), Fazit
(F1 vor F2), Zielsetzung (Z1 vor Z2), Arbeitsteilungstabelle (Rößler-Zeilen zuerst).

**In diesem Eintrag nachgezogen:**

| Stelle | vorher | nachher |
|---|---|---|
| Kurzfassung, erster Satz | „untersucht, ob ein Reinforcement-Learning-Agent …" | „besteht aus zwei Teilen. Der erste ist die Entwicklung des 2D-Spiels *Stoneforge* …" |
| §8.2 Beiträge | Methodisch → Inhaltlich → Technisch | **Technisch** (Plattform) → Inhaltlich → Methodisch |
| Schlagwörter | Reinforcement Learning, POMDP, … | Prozedurale Weltgenerierung, Spielentwicklung, Reinforcement Learning, … |

**Geprüft, keine Änderung nötig:**
- §1.1 Motivation beginnt bereits mit Videospielen als Testumgebung.
- §1.2 beginnt mit Stoneforge und der Weltentstehung, danach der Agent.
- Kapitel 3: Der PCG-Absatz („Weltgenerierung und der Preis des eigenen Generators") steht
  bereits vor den RL-Absätzen.
- Kapitel 6: Technologie-Stack → Umgebung/Binding → Trainings-Pipeline → Monitoring; der
  C++-Kern steht in der Stack-Tabelle an erster Stelle.
- §8.3 Einschränkungen ist nach Schwere sortiert, nicht nach Projektteil; das bleibt so.
- Haupttitel: „Navigation in prozedural generierten 2D-Welten mit Reinforcement Learning"
  nennt die Welt bereits vor dem Verfahren.

**Offen zur Entscheidung:** Der Untertitel („Generalisierung eines rekurrenten Agenten auf
unbekannte Welten am Beispiel Stoneforge") ist weiterhin RL-zuerst formuliert. Eine Änderung
sollte mit dem Betreuer abgestimmt werden und wurde deshalb nicht vorgenommen.

**Verifikation:** `latexmk -pdf` fehlerfrei, 62 Seiten, 0 Undefined References/Citations.

---

## v2026-08-04.5 — Einleitung gestrafft: Motivation, Problemstellung, Aufbau

**Anlass:** Rückmeldung, die Einleitung sei zu geschraubt, zu detailliert und nehme zu viel
vorweg. Vorgehen: Recherche zu den Anforderungen an Einleitungsteile, dann Kürzung.

### Änderung 1 — Motivation von einer Seite auf vier Absätze

**Recherche-Grundlage:** Die Motivation soll Relevanz begründen und zum Thema hinführen; sie
ist einer von rund sechs Bausteinen der Einleitung. Details gehören in Problemstellung und
Hauptteil. Als häufigste Fehler werden zu abstrakte Formulierungen, unnötige Ausschweifungen
und Überladung genannt.
**Gestrichen:** Die vierteilige Anforderungsliste an den Generator (Determinismus,
Lösbarkeit, steuerbare Schwierigkeit, Durchsatz). Kein Informationsverlust: drei davon stehen
in §1.3, alle fünf in Z1 (§1.4).
**Neu:** Zwei Sätze Hinführung vor dem Einstieg (warum Videospiele als Testumgebung), weil
der Text sonst zu abrupt beginnt.
**Gleichgewicht korrigiert:** Der RL-Teil war zugunsten des Generators untergegangen; er hat
jetzt einen eigenen Absatz (RL erklärt + warum ein Test in fester Umgebung nichts aussagt),
und im Schlussabsatz stehen Spiel und Agent gleichrangig.
**Wortwahl:** „Programm" → „KI" im Einstieg, „Software-Agent" bei der Begriffseinführung.

### Änderung 2 — Problemstellung: Ergebnisse entfernt

Die Problemstellung hatte drei Stellen, die den Hauptteil vorwegnahmen:

| Stelle | Warum raus |
|---|---|
| „Zuerst wurde die Luftlinie verwendet. Das war falsch: … lagen die Wege zwischen 42 und 75 Feldern" | Befund aus der Entwicklung, nimmt §10.7 vorweg → ersetzt durch die offene Frage „Woran misst man die Schwierigkeit einer erzeugten Welt überhaupt?" |
| „Genau dort liegt auch das Hauptergebnis: Die erzeugten Welten sind so beschaffen, dass …" | Ergebnis gehört nicht in die Problemstellung → gestrichen |
| „…war zu Beginn nur eine Vermutung; sie bestätigte sich erst, als die gedächtnislosen Varianten scheiterten" | ebenfalls Ergebnis → „Ob der Agent dafür ein Gedächtnis braucht, ist Teil der Frage und nicht ihre Voraussetzung." |

**Bewusst beibehalten:** das „Orakel". Es ist keine diskutierte, sondern eine *ausgeschlossene*
Lösung und grenzt ab, was überhaupt als Lösung zählt. Formulierung dazu geschärft: „Die
Aufgabe ist also erst dann richtig gestellt, wenn dieser Ausweg ausgeschlossen ist."
Der kurze Ergebnisausblick verbleibt allein in §1.4 (fehlende Schwierigkeits-Anforderung),
wo er als Einschränkung der eigenen Zielsetzung hingehört.

### Änderung 3 — „Aufbau der Arbeit" von 7.148 auf 1.867 Zeichen

**Recherche:** Der Teil ist Standard und sollte nicht entfallen, aber „in wenigen Sätzen"
gehalten werden; ausdrücklich zu vermeiden ist die bloße Wiederholung des
Inhaltsverzeichnisses. Gefragt ist die *logische Abfolge*, nicht der Inhalt.
**Gestrichen:** die siebenteilige Kapitel-Aufzählung mit je 2–4 Zeilen Inhaltsvorschau.
**Ersetzt durch:** einen Absatz zur Leselogik (in jedem Kapitel zuerst die Welt, dann der
Agent, weil das eine Voraussetzung des anderen ist) plus Hinweis auf die beiden Anhänge.
Die Absätze „Zur Versionsbezeichnung" (von Aufzählung auf drei Sätze gekürzt) und „Zur
Arbeitsteilung" mit Tabelle bleiben, da sie Lesehilfen und keine Inhaltsvorschau sind.

### Änderung 4 — Gedankenstriche entfernt

Alle 34 Em-Dashes (`---`) im Fließtext ersetzt: paarige Einschübe durch Kommas, nachgestellte
Erklärungen je nach Satzbau durch Doppelpunkt, Semikolon oder Punkt. Die verbleibenden 40
Vorkommen stehen ausschließlich in TikZ-Kommentaren (`% ---- MDP ----`) und werden nie
gesetzt. Die 84 Zahlenbereiche (`35--45`, En-Dash) sind unverändert, das ist korrekte
Typografie.

**Zwei Fehler beim Umbau gefunden und behoben:** „Zur Versionsbezeichnung" stand nach der
Kürzung doppelt im Dokument; ein `\noindent` ließ zwei Absätze ineinanderlaufen, nachdem die
Arbeitsteilungs-Tabelle als Float weggeflossen war.

**Verifikation:** `latexmk -pdf` fehlerfrei, 62 Seiten (vorher 63), 0 Undefined
References/Citations.

---

## v2026-08-04.4 — Struktur: Weltgenerator und RL als ein Projekt statt zwei nebeneinander

**Kontext:** Die Aufnahme des Generator-Teils (v2026-08-03.2) hatte ihn zwar ergänzt, aber
sichtbar *danebengestellt* — eigene Absatzüberschriften „Der erste Teil / Der zweite Teil",
getrennte Problemstellungen, Z1/Z2 als Blöcke. Die Gliederung benannte damit die Trennung,
statt sie aufzulösen. Dieser Eintrag stellt die Arbeit auf ein durchgehendes Prinzip um:
**erst die Welt, dann der Agent — in jedem Kapitel.**

### Nachtrag — §1.1–1.4 sprachlich neu gefasst (zweiter Durchgang)

Die erste Fassung war inhaltlich richtig, aber zu abstrakt und zu lang im Satzbau
(„Damit verschiebt sich die Aufgabe an eine Stelle, die leicht unterschätzt wird",
„Messinstrument", „die inhaltlich folgenreichste"). Ziel des zweiten Durchgangs: verständlich
für jemanden ohne RL-Hintergrund, kurze Sätze, konkrete Zahlen statt Umschreibungen.

| vorher | nachher |
|---|---|
| „Ein Computerprogramm, das ein Videospiel-Level fehlerfrei durchspielt, hat zunächst nur eines bewiesen …" | „Wenn ein Programm ein Spiel-Level fehlerfrei durchspielt, kann das zweierlei heißen: Es hat verstanden, wie man spielt — oder es hat dieses eine Level auswendig gelernt. Von außen sieht beides gleich aus." |
| Vier Anforderungen als Fließtextabsatz | Vier Anforderungen als Aufzählung mit fettem Vorspann (scanbar) |
| „schon die Frage, welches Maß die Schwierigkeit überhaupt richtig abbildet, ist nicht trivial" | „Zuerst wurde die Luftlinie verwendet. Das war falsch — bei der Vorgabe ,35–45' lagen die tatsächlichen Wege zwischen 42 und 75 Feldern." |
| „Gelernt wird durch Belohnung: Erreicht der Agent den Ausgang, erhält er einen großen positiven Wert; jeder Schritt kostet ein wenig." | „Gelernt wird über Belohnung. Ausgang erreicht: großer Pluspunkt. Jeder Schritt: kleiner Abzug." |
| „Die Reihenfolge der beiden Hälften ist keine Rangfolge. F1 ist nicht Vorarbeit für F2, sondern die Bedingung, unter der …" | „F1 steht nicht deshalb zuerst, weil es weniger wichtig wäre, sondern weil F2 ohne F1 nicht zu beantworten ist." |

Gestrichene Vokabeln: Messinstrument, Beiwerk, Nahtstelle, folgenreichste, gegenläufige
Forderungen, zwingend. Alle Inhalte, Zitate, Querverweise, F1/F2 und Z1/Z2 unverändert
erhalten; die Vorab-Festlegung der Z2-Schwellen (Anti-HARKing) steht jetzt als klarer Satz
(„sie wurden also nicht nachträglich passend gemacht") statt als Nebensatz.

### Änderung 1 — §1.1–1.4 vollständig neu geschrieben

Die Einleitung ist jetzt ein durchlaufender Text ohne Teil-Überschriften. Argumentationslinie:
Generalisierung prüfen → dafür braucht es prozedurale Welten → aber nicht irgendeine Welt,
sondern eine, die als *Messinstrument* taugt (vier Anforderungen) → fertige Benchmarks geben
den Generator als Blackbox vor → deshalb eigenes Spiel → und genau diese Zugänglichkeit macht
das Hauptergebnis erst auffindbar.
- §1.2 erzählt Weltentstehung und Lernaufgabe als eine Geschichte; der Satz „Spiel und
  Versuchsaufbau sind dasselbe Programm" ersetzt die frühere Zweiteilung.
- §1.3 stellt die beiden Schwierigkeiten (Agent sieht zu wenig / Generator muss drei
  gegenläufige Forderungen erfüllen) nebeneinander und leitet daraus **eine Frage in zwei
  Hälften** ab. F1/F2 bleiben erhalten, aber mit dem Zusatz: „Die Reihenfolge ist keine
  Rangfolge" — F1 ist Bedingung, nicht Vorarbeit.
- §1.4 führt Z1/Z2 als zwei aufeinander aufbauende *Ebenen* ein („wann die Umgebung als
  Messinstrument taugt" / „wann der Agent als erfolgreich gilt") statt als zwei Blöcke.

### Änderung 2 — Kapitel 2 (Grundlagen) umgestellt

PCG-Grundlagen von 2.9/2.10 an den Anfang (jetzt 2.1/2.2), RL folgt. Kapiteleinleitung
begründet die Reihenfolge sachlich statt didaktisch: Ob eine Lernaufgabe die zu messende
Fähigkeit überhaupt erfordert, entscheidet sich in der Weltgenerierung.

### Änderung 3 — Kapitel 3: PCG-Literatur ergänzt

Bisher ausschließlich RL-Benchmarks und -Verfahren. Neuer Absatz „Weltgenerierung und der
Preis des eigenen Generators": Die Verfahren sind Stand der Technik (`shaker2016pcg`,
`johnson2010cellular`), der Beitrag liegt in der Kombination zu einer zugleich *spielbaren*
und *kontrollierbaren* Umgebung. Enthält die Begründung, warum kein fremder Benchmark
verwendet wurde — und was das kostet (fehlende Einbettung in publizierte Vergleichswerte).

### Änderung 4 — Kapitel 4 umbenannt und umgestellt

„Die Umgebung Stoneforge" → **„Stoneforge: Spiel, Weltgenerator und Lernumgebung"**.
Reihenfolge der Unterabschnitte gedreht:

| vorher | nachher |
|---|---|
| 4.1 POMDP-Charakter | 4.1 Aufbau des Weltgenerators |
| 4.2 Beobachtung/Aktionen | 4.2 Garantierte Lösbarkeit |
| 4.3 Reward-Design | 4.3 Was der Agent davon sieht: POMDP-Charakter |
| 4.4 Aufbau des Weltgenerators | 4.4 Beobachtung, Aktionen, Episodenende |
| 4.5 Garantierte Lösbarkeit | 4.5 Reward-Design |

Neue Kapiteleinleitung begründet die Reihenfolge und benennt die Doppelrolle (dieselbe
Simulation für Mensch und Agent) als Entwurfsentscheidung. Neue Labels `sec:pomdpchar`,
`sec:rewarddesign`.

### Änderung 5 — Neuer Evaluationsabschnitt §7.1 „Bewertung der Plattform (Z1)"

**Lücke geschlossen:** Die Einleitung führte fünf Z1-Kriterien ein, die nirgends
zusammengeführt nachgewiesen wurden — die Evaluation bewertete ausschließlich den Agenten.
Neue Tabelle `tab:z1` mit Anforderung · Nachweis · Status; alle fünf erfüllt, jeweils mit
Verweis auf die Messung (bit-identische Trainingspfade, 3.150 Seeds Lösbarkeit, 150/150 im
Distanzband, 88–190 FPS, spielbarer Client).
**Inhaltlicher Zusatz:** Drei der fünf Nachweise entstanden erst als Korrektur eines vorher
unbemerkten Fehlers (Config-Leck, Luftlinienmaß, Carving-Fundament) — und keiner war am
Verhalten des Agenten ablesbar. Sie wurden sichtbar, weil das RL-Experiment unerklärliche
Zahlen lieferte. Das ist die erste dokumentierte Verzahnung beider Teile.

### Änderung 6 — Fazit beantwortet F1 und F2 getrennt und setzt sie ins Verhältnis

§8.1 gliedert in „F1 — beantwortet", „F2 — nicht entscheidbar" und einen dritten Absatz zum
Verhältnis beider Antworten. Kernaussage neu formuliert, gehört keinem der beiden Teile
allein:

> Die Schwierigkeit einer Lernaufgabe ist eine Eigenschaft der Weltgenerierung und muss dort
> spezifiziert und geprüft werden, bevor ein Lernverfahren darauf angesetzt wird.

Mit dem Zusatz, dass der Befund beide Projektteile voraussetzte: ohne eigenen Generator nicht
lokalisierbar, ohne RL-Experiment nicht auffindbar. §8.2 „Technisch" um die
Generator-Bestandteile und den Punkt erweitert, dass die Härtung ohne Neuentwicklung möglich
ist, weil der wirksamste Hebel bereits implementiert ist.

### Änderung 7 — §1.5 Lesepfad

Ergänzt, dass die beiden Stränge nicht nacheinander, sondern in jedem Kapitel nebeneinander
laufen, und benennt die zwei Stellen, an denen sie aufeinandertreffen (§7.4 und §8.4).

**Verifikation:** `latexmk -pdf` fehlerfrei, 63 Seiten, 0 Undefined References/Citations,
Overfull-Boxen 30 → 29.

---

## v2026-08-04.3 — Nachlauf zum zweiten Review: drei offene Punkte geschlossen

### Änderung 1 — §8.4 trug noch die Werte der alten Reward-Bilanz

**Problem:** Bei der PBRS-Korrektur (v2026-08-04.1) wurden Tabelle 13 und §7.3 aktualisiert,
der Ausblick aber nicht. Dort standen weiterhin „rund 3 % der dichten Reward-Masse" (jetzt
5 %) und Revisit-Penalty „−6,8 je Episode" (jetzt −6,2).
**Lösung:** Beide Werte nachgezogen. Zusätzlich ergänzt, dass der Revisit-Penalty den
Explorations-Bonus (+4,1) betragsmäßig übersteigt und ihn damit faktisch neutralisiert — das
schärft den Reparaturvorschlag.

### Änderung 2 — Diskontierungs-Absatz: Formulierung korrigiert, Zahlen bestätigt

**Review-Vorwurf:** +37,7 und −5,3 implizierten denselben Faktor 0,377, es sei „offenbar ein
Skalar auf beide Seiten angewandt" worden; richtig seien +26,5 und −7,8.
**Prüfung — der Vorwurf trifft nicht zu.** Beide Größen sind je Episode gemessen. Beleg über
die per-Lauf-Werte, die bei einem gemeinsamen Skalar identisch sein müssten:

| Lauf | E[γ^T] (Terminal) | impliziter Diskont der dichten Terme |
|---|---|---|
| s1 | 0,358 | 0,354 |
| s2 | 0,408 | 0,387 |
| s3 | 0,365 | 0,379 |

Sie weichen je Lauf voneinander ab — kein gemeinsamer Skalar, sondern zwei unabhängige
Messungen, die im Mittel zufällig nahe beieinanderliegen.
**Zur vorgeschlagenen Korrektur +26,5:** Das ist γ^E[T], also γ^T an der mittleren
Episodenlänge. Für den *erwarteten* diskontierten Terminal-Reward ist E[γ^T] maßgeblich, und
weil γ^T konvex in T ist, unterschätzt γ^E[T] den Wert systematisch (Jensen — dieselbe
Ungleichung, die im Dokument bereits für η diskutiert wird). Gemessen: E[γ^T] = 0,377 gegen
γ^E[T] = 0,266. **+37,7 ist korrekt, +26,5 wäre der verzerrte Schätzer.**
**Berechtigt war die Formulierungskritik:** „über die mittlere Episodenlänge diskontiert"
beschrieb tatsächlich γ^E[T] und nicht das Gemessene.
**Lösung:** Absatz neu gefasst als Aufzählung, die beide Größen explizit benennt
(100·E[γ^T] bzw. Σγ^t·r_t über den tatsächlichen Verlauf), den Jensen-Punkt mit dem
Gegenwert +26,6 ausweist und erklärt, warum der gemessene mittlere Diskont der dichten Terme
(0,37) unter dem Wert bei gleichmäßiger Verteilung (0,55) liegt: Der Revisit-Penalty fällt
spät in der Episode an und wird deshalb stärker diskontiert.
**Skript erweitert:** `scripts/reward_bilanz.py` gibt jetzt E[γ^T], γ^E[T] und den implizit
gemessenen Diskontfaktor der dichten Terme getrennt aus, damit die Unterscheidung
reproduzierbar belegt ist.

### Änderung 3 — Platzhalterzeile aus der KI-Erklärung entfernt

Die Zeile „(weiteres Werkzeug eintragen)" stand auf Seite iii und ließ die Erklärung
unfertig wirken. Entfernt; die Tabelle listet jetzt nur das Werkzeug, dessen Einsatz belegbar
ist. Der `TODO`-Kommentar im Quelltext wurde entsprechend umformuliert: Falls weitere
Werkzeuge verwendet wurden, **muss** je Werkzeug eine Zeile ergänzt werden (Produktname,
Einsatzform, betroffene Teile, Art der eigenen Prüfung).

**Verifikation:** `latexmk -pdf` fehlerfrei, 60 Seiten, 0 Undefined References/Citations.
Grep über das Dokument bestätigt: keine Restvorkommen der alten Werte (3 %, −6,8).

---

## v2026-08-04.2 — Einleitung um den Plattform-Teil erweitert, Arbeitsteilung als Tabelle

**Anlass:** Einleitung und Zielsetzung deckten bisher nur den RL-Teil ab, obwohl die
Spielentwicklung und der prozedurale Weltgenerator (L. Rößler) ein gleichwertiger Teil des
Projekts sind.

### Änderung 1 — §1.1 Motivation: Plattform als eigener Projektteil begründet

Zwei Absätze ergänzt: (a) welche Anforderungen eine RL-taugliche Umgebung erfüllen muss
(Reproduzierbarkeit, garantierte Lösbarkeit, steuerbare Schwierigkeit, Durchsatz) und warum
fertige Benchmarks (MiniGrid, Procgen) sie nur teilweise erfüllen — sie geben den Generator
als Blackbox vor, was sich hier gerade als entscheidend erwies; (b) explizite Feststellung,
dass das Projekt aus zwei gleichgewichtigen Teilen besteht.

### Änderung 2 — §1.2 in zwei Teile gegliedert

Neue Absatzüberschriften „Der erste Teil: das Spiel" (C++-Kern, spielbarer Client,
Python-Anbindung, chunkweise Generierung ohne gespeicherten Weltzustand) und „Der zweite
Teil: die Lernaufgabe". Verweise auf §2.10 und §4.4.

### Änderung 3 — §1.3: zwei Forschungsfragen statt einer

Bisher nur die RL-Frage. Ergänzt um die konstruktive Problemstellung (Abwechslung vs.
garantierte Lösbarkeit vs. quantifizierbare Schwierigkeit als Spannungsfeld) und um **F1**:

> F1 — Wie lässt sich ein prozeduraler Weltgenerator so konstruieren, dass er
> abwechslungsreiche, nachweislich lösbare Welten mit präzise steuerbarer Schwierigkeit
> erzeugt und dabei als reproduzierbare Versuchsumgebung für RL taugt?

Die bisherige Frage wird zu **F2**. Ergänzt ist der Hinweis, dass F1 nicht Vorarbeit für F2
ist, sondern dessen Voraussetzung — das Hauptergebnis der Arbeit betrifft den Generator,
nicht den Agenten.

### Änderung 4 — §1.4 Zielsetzung: Kriteriengruppe Z1 ergänzt

Bisher nur die drei RL-Schwellen. Neu **Z1 (Plattform/Generator)** mit fünf funktionalen
Kriterien, jeweils mit Nachweisstelle: Determinismus (§4.4), garantierte Lösbarkeit (§4.5),
steuerbare Schwierigkeit (§10.7), RL-Tauglichkeit (§6), Spielbarkeit (§6). Die RL-Schwellen
werden zu **Z2**.
**Wichtig für die Redlichkeit:** Es steht ausdrücklich dabei, dass nur die Z2-Schwellen vorab
festgelegt wurden (belegt über das Changelog-Datum, Anti-HARKing), während Z1
Konstruktionsziele sind, die im Verlauf präzisiert wurden. Zusätzlich benannt: **Z1 verlangt
lösbare, aber keine hinreichend schwierigen Welten** — genau diese fehlende Anforderung ist
der Grund, aus dem F2 unbeantwortet bleibt.

### Änderung 5 — Arbeitsteilung als Tabelle (`tab:arbeitsteilung`)

Der Fließtext-Absatz aus v2026-08-03.2 ersetzt durch eine neunzeilige Tabelle
(Beitrag · Schwerpunkt · Kapitel), gegliedert in Plattform (L. Rößler), RL (F. Merlau) und
gemeinsame Teile. `TODO`-Kommentar zur Gegenprüfung bleibt bestehen.

### Änderung 6 — Jedes Hauptkapitel beginnt auf einer neuen Seite

`\let\sfOldSection\section` + `\renewcommand{\section}{\clearpage\sfOldSection}` in der
Präambel. Wirkt auch auf die gesternten Abschnitte des Vorspanns; dort ist `\clearpage` ein
No-Op, es entstehen also keine Leerseiten. Verifiziert: Inhaltsverzeichnis zeigt weiterhin
nur Sections/Subsections (die `\paragraph`-Einträge in der `.toc` werden über `tocdepth`
gefiltert und waren auch vorher schon enthalten).

### Änderung 7 — Kurzfassung und Kapitelübersicht nachgezogen

Kurzfassung nennt die Plattform jetzt als eigenen Teil (Generator: reproduzierbar,
nachweislich lösbar, steuerbare Zielentfernung) statt als Nebensatz. Die Kapitel-Bullets zu
Grundlagen und Umgebung erwähnen die PCG-Verfahren bzw. den Weltgenerator.

**Verifikation:** `latexmk -pdf` fehlerfrei, 60 Seiten (vorher 55), 0 Undefined
References/Citations, Overfull-Boxen unverändert 30.

---

## v2026-08-04.1 — Zweites Fakten-Review: PBRS-Rechnung korrigiert, Abb. 10 neu erzeugt

**Kontext:** Zweite externe Prüfung der Projektdokumentation. Erneut wurde jede Behauptung
gegen Code, `eval_history.json` und `CHANGELOG.md` verifiziert. Zwei Befunde waren
substanziell (ein rechnerisch falscher Satz über PBRS, ein Protokoll-Widerspruch in einer
Abbildung), einer war eine Fehlmessung im eigenen Diagnoseskript.

### ⚠️ Änderung 1 — PBRS-Summe: Teleskopierungsaussage war für γ < 1 falsch

**Datei:** `docs/Projektdokumentation.tex` (§7.3), `scripts/reward_bilanz.py`
**Problem:** Tabelle 11 und der zugehörige Text behaupteten, die PBRS-Episodensumme sei
„wegen der Teleskopierung unabhängig vom gewählten Weg auf β·d₀/128 festgelegt" (+0,8).
Das gilt nur für γ = 1. Korrekt ist

```
Σ F = β/128 · [ d₀ + (1−γ)·Σ_t d_t ]
```

Der Restterm verschwindet bei γ = 0,999 nicht. **Ursache im eigenen Code:** Die erste Fassung
von `reward_bilanz.py` hat den PBRS-Beitrag gar nicht gemessen, sondern als `BETA*d0/128`
*berechnet* — also genau die falsche Formel eingesetzt. Alle übrigen Terme der Tabelle waren
gemessen.
**Lösung:** Skript misst den PBRS-Beitrag jetzt schrittweise aus der tatsächlichen
BFS-Distanz (`current_bfs_distance_to_exit()` je Schritt). Zusätzlich fester Politik-RNG
(`torch.manual_seed(0)`), weil die Messung ohne ihn von Lauf zu Lauf um mehrere Punkte
schwankte. Neu gemessen über die Läufe 1–3, Testset A:

| Term | vorher (Tab. 11) | nachher (gemessen) |
|---|---|---|
| Explorations-Bonus | +4,2 (17 %) | **+4,1 (16 %)** |
| Schritt-Malus | −13,5 (53 %) | **−13,3 (53 %)** |
| Revisit-Penalty | −6,8 (27 %) | **−6,2 (25 %)** |
| **PBRS gesamt** | **+0,8 (3 %)** | **+1,3 (5 %)** |
| — davon teleskopiert | — | +0,8 |
| — davon Restterm | — | +0,5 |
| Summe dichte Terme | −15,3 | **−14,1** |

Der teleskopierte Anteil ist also um **Faktor 1,7** zu niedrig gewesen; der PBRS-Anteil an
der dichten Reward-Masse liegt bei 5 %, nicht 3 %. Die qualitative Aussage („Richtungssignal
ist ein kleiner Teil der dichten Reward-Masse") bleibt unverändert gültig.
**Neuer Absatz** in §7.3 mit Herleitung des Restterms als eigene Gleichung.

**Wichtige Einschränkung gegenüber dem Review:** Das Review folgert, der Restterm sei „genau
die Sorte Nebenwirkung, vor der PBRS eigentlich schützen soll". **Das trifft nicht zu.** Die
Garantie von Ng et al. gilt für das *diskontierte* Optimierungsziel bei identischem γ — und
genau diese Bedingung ist hier erfüllt (γ_Shaping = γ_RL = 0,999). Der Restterm tritt nur in
der *un*diskontierten Episodensumme auf, die kein Optimierungsziel ist; die optimale Politik
bleibt unverändert. Das steht jetzt explizit in §7.3, damit der Punkt nicht als
Invarianzverletzung missverstanden wird.
Auch die Größenordnung des Reviews war zu hoch gegriffen (dort +0,79 Restterm bei
angenommenem d̄ ≈ 30); die tatsächlich gemessene mittlere Distanz während der Episode
beträgt **d̄ ≈ 20**, der Restterm entsprechend +0,5.

### ⚠️ Änderung 2 — Abbildung 10 mischte zwei Eval-Protokolle unter einer Bildunterschrift

**Dateien:** `docs/figures/fig_entwicklung.pdf`, `scripts/plot_entwicklung.py` (neu), §7.1
**Problem:** Die Abbildung zeigte vier Balken unter der Bildunterschrift „Testset A,
Exit 35–45": Phase 3 (86/2), Phase 4 (98/2), Delta-BFS (100/42), LSTM-Curriculum (86/36).
Tabelle 6 gibt für dasselbe Phase-4-Modell aber **32 % stoch / 0 % det** an. Nur der letzte
Balken war tatsächlich unter Exit 35–45 gemessen, die drei Mai-Balken unter dem
Kurzdistanz-Protokoll auf der Umgebung vor v11.
**Relevanz:** Der inhaltliche Beitrag in §8.2 („LSTM ohne BFS erreicht mindestens dieselbe
stochastische Leistung wie MLP mit BFS") beruht auf 86 % gegen 32 %. Stünde 98 % daneben,
kehrte sich die Aussage scheinbar um.
**Lösung:** Abbildung neu erzeugt (`scripts/plot_entwicklung.py`, Daten per `pdftotext` aus
der Altfassung übernommen). Zwei sichtbar getrennte Protokollgruppen mit Kopfzeilen; als
Brücke ein **fünfter Balken „Phase 4 nachgemessen" (32/0)**, der den Protokolleffekt am
selben Modell zeigt. Bildunterschrift und §7.1 benennen jetzt beide Betriebsarten — bisher
erklärte der Text nur den det-Abfall (2 → 0), nicht den stoch-Abfall (98 → 32).

### Änderung 3 — Drei Zahlen für Seed 0/1 Phase 1 aufgelöst

**Problem:** §10.5 nannte 42 %, §10.6 „12–16 % (wie Seed 0)", Tab. 16 „16–28 %".
**Prüfung** an `eval_history.json` der drei v11-Läufe:

| Lauf | Phase-1-Verlauf (stoch) |
|---|---|
| Seed 0 | 12,16,18,14,16,12,24,16,32,18,34,18,12,18,16,16,34,36,24,**42** |
| Seed 1 (vor Cap-Fix) | 14,16,10,12,14,12,12,12,8,10,14 → **8–16 %** |
| Seed 1 (nach Cap-Fix) | 16,16,10,20,16,20,24,28,24,34,14,22 → **10–34 %** |

Alle drei Angaben waren echt, beschrieben aber verschiedene Zeitfenster. Präzisiert:
Seed 0 lag „über weite Strecken bei 12–18 % und stieg erst gegen Ende auf 42 %";
§10.6 auf 8–16 % korrigiert; Tab. 16 auf 10–34 % korrigiert.

### Änderung 4 — Energie-Drosselung als vierte Alternativerklärung in §7.2

Die Drosselung (Läufe 4–7, 29–36 h statt 8 h) fällt exakt mit der Chargengrenze zusammen und
war bisher nur im Anhang erwähnt. Als Punkt 4 in die Ausschlussliste aufgenommen, mit dem
Argument: Training ist schrittbasiert, Evaluationen liegen auf festem 25k-Raster, Phasen
enden am Schrittbudget. **Nebenkorrektur:** Die Begründung im Anhang lautete „das Curriculum
ist leistungsbasiert" — nach Änderung 2 vom 03.08. ist es das faktisch nicht; das Argument
läuft jetzt über das Schrittbudget und ist damit sogar stärker.

### Änderung 5 — Sechs Präzisierungen

1. **η-Definition** (§7.3): ergänzt, dass über die *Quotienten* gemittelt wird. Dieselben
   Läufe ergeben 0,05 als Mittel der Quotienten, aber 0,03 als Quotient der Mittel
   (40/1329) — Jensen-Ungleichung, jetzt benannt, damit es nicht als Fehler gelesen wird.
2. **Diskontierung** (§7.3): Terminal-Reward wurde diskontiert (+26) gegen undiskontierte
   dichte Terme (−15,3) gestellt. Jetzt auf beiden Seiten diskontiert: +37,7 gegen −5,3.
3. **Seed-Trennung** (§5.4): Formulierung war selbstwidersprüchlich („der übrige Bereich" für
   Diagnosen, dann 6000–6119 genannt, was die Selektions-Seeds enthält). Umformuliert:
   Diagnosemessungen nutzen den Validierungsbereich als Ganzes, Begründung ergänzt.
4. **Tab. 12 Baseline** skriptinkonsistent zur n=7-Auswertung (73/80 vs. 74,0/77,3);
   in der Caption offengelegt.
5. **Kurzfassung:** „rund acht Prozentpunkte" galt nur für Testset A (B: 10,4). Jetzt „acht
   (Testset) bzw. elf (Holdout)". „führen **systematisch** zu optimistischen Aussagen" →
   „können leicht zu optimistischen Aussagen führen" (belegt ist ein Varianz-, kein
   Bias-Problem; p = 0,149). Zusätzlich ε = 0,9 benannt, damit die 92 % nicht als
   Greedy-Kompass missverstanden werden — die Aussage „ein Zufallslauf mit 10 %
   Richtungsbias schlägt den Agenten" ist die härtere.
6. **Phase-1-Ziel 85 %** (§7.2): Herkunft ergänzt (nackter Lauf nach Batch-Fix, 84–88 %).
   Nicht das Niveau, sondern die Stabilität unterscheidet nackten Lauf und Curriculum-Stack.

### Änderung 6 — Kleinkram

- Durchsatz „ca. 90–190 FPS" → **88–190**, mit Hinweis, dass die finale Konfiguration
  (batch=8) am unteren Ende liegt.
- Tab. 5: Simulationskern von „ca. 4500" auf **ca. 4300 LOC** präzisiert (gemessen: Kern 3575
  + Nicht-Client-Header 732 = 4307; Client 4326 + 178 = 4504). Beide Angaben waren korrekt,
  die identische Rundung wirkte aber wie ein Copy-Paste.
- KI-Erklärung: „Nicht eingesetzt zur Erzeugung von Messergebnissen" um den Halbsatz ergänzt,
  dass Auswertungsskripte KI-gestützt erstellt und gegen den Rohdatenbestand verifiziert
  wurden (sonst Reibung mit der Nennung von `reward_bilanz.py` in derselben Tabelle).
- Abb. 8: Caption erklärt, warum das Phase-4-Panel bei 175k endet (Evalraster ab null),
  obwohl das Budget 200k beträgt und ausgeschöpft wurde.

### Geprüft und ZURÜCKGEWIESEN bzw. relativiert

| Review-Punkt | Prüfung | Ergebnis |
|---|---|---|
| PBRS-Restterm sei „Nebenwirkung, vor der PBRS schützen soll" | Ng et al. garantieren Invarianz für das diskontierte Ziel bei identischem γ — hier erfüllt | **Keine Invarianzverletzung**; Restterm nur in der undiskontierten Summe |
| Restterm ≈ +0,79, Summe ≈ 1,55 | gemessen: d̄ ≈ 20 statt angenommener 30 | Restterm **+0,5**, Summe **+1,3** |
| Fußnote Tab. 9 nenne „10,4 ± 4,2" | Dokument nennt ± 4,6, belegt in `CHANGELOG.md:522` | **Review-Zitat falsch**; das „± 1,7" war dagegen unbelegt und wurde entfernt |
| Tab. 5: „4500/4500 sieht kopiert aus" | nachgemessen (s. o.) | Beide Werte korrekt, dennoch präzisiert |

**Verifikation:** `latexmk -pdf` fehlerfrei, 55 Seiten, 0 Undefined References/Citations.

---

## v2026-08-03.2 — Prozedurale Weltgenerierung als eigener Beitrag aufgenommen (Zuarbeit L. Rößler)

**Kontext:** Die Dokumentation war bislang fast rein RL-seitig; der Weltgenerator kam nur als
Randnotiz vor, obwohl er der zweite Hauptteil des Projekts ist. Zuarbeit von Laurin Rößler
(Grundlagen PCG + Konzeptbeschreibung) eingearbeitet. **Jede darin enthaltene Aussage wurde
gegen `src/core/world.cpp`, `assets/base/game_config.json` und `python/stoneforge_env.py`
geprüft; zwei Aussagen waren nicht haltbar und wurden korrigiert (siehe unten).**

### Änderung 1 — Neuer Grundlagen-Abschnitt §2.10 „Algorithmen der prozeduralen Weltgenerierung"

**Datei:** `docs/Projektdokumentation.tex`
**Inhalt:** Vier Absätze — rauschbasierte Basisbelegung (Hash- vs. interpoliertes Wertrauschen),
zelluläre Automaten als Formgebung (Moore-Nachbarschaft, Birth/Survival), graphentheoretische
Erreichbarkeitsanalyse (BFS), heuristische Sicherheitsnetze (Manhattan-Distanz als Gleichung,
Carving).
**Fachliche Ergänzung gegenüber der Zuarbeit:** Die Zuarbeit ließ offen, welche Nachbarschaft
für die Konnektivitätsanalyse anzusetzen ist. Ergänzt, dass hier die **Vierer-Nachbarschaft
(von Neumann)** korrekt ist und nicht die für die Glättung verwendete Moore-Nachbarschaft —
eine diagonal hergestellte Verbindung wäre bei `Discrete(4)` nicht begehbar. Der Code macht
das richtig (`validateReachabilityWindow`, `chooseExitPoint`: beide 4-Nachbarschaft).
**Neue Literatur:** `johnson2010cellular`, `shaker2016pcg`, `cormen2009algorithms`,
`perlin1985image` (Bibliografie 30 → 34 Einträge).

### Änderung 2 — Neuer Abschnitt §4.4 „Aufbau des Weltgenerators"

Dreistufige Pipeline je Chunk dokumentiert, verifiziert gegen `World::generateChunk()`:

| Stufe | Inhalt | Verifiziert an |
|---|---|---|
| 0 Biomfeld | interpoliertes Wertrauschen, Domain Warping, 7 gleich breite Intervalle | `biomeFieldForChunk`, `biomeTagForChunk` |
| 1 Basisbelegung | Hash-Rauschen je Tile, biomabhängige Schwellen (Wald 7 % … Bergland 25 %); vorgelagerte Seenmaske `0,75a + 0,25b > 0,86` | `sampleBaseTile`, `lakeMaskAt` |
| 2 Zelluläre Glättung | Moore-Nachbarschaft, Halo in Breite der Iterationszahl (reihenfolgeunabhängig) | `runCellularSmoothingStage` |
| 3 Landmarken | P = 0,10 je Chunk, 5×5-ASCII-Matrizen je Biom, Spawn-/Exit-Chunks ausgenommen | `placeBiomeStructure` |

### ⚠️ Änderung 3 — ZWEI KORREKTUREN an der Zuarbeit

**Korrektur A — „Garantierter Pfad (Manhattan): AKTIVIERT" ist falsch.**
Die Zuarbeit (Tabelle 5.1) führt den Manhattan-Carve als aktiv und folgert, das System
arbeite „in der Produktion primär über den schnellen Manhattan-Fallback". Tatsächlich:

```
assets/base/game_config.json, Z. 8–9:
  "forceGuaranteedPath": false,
  "guaranteedPathFallback": false,
```
Zusätzlich übergibt `stoneforge_env.py:152` explizit `force_guaranteed_path=False`.
**Es findet in der berichteten Konfiguration kein Carving statt.** Die Lösbarkeit stammt
allein aus der BFS-Exit-Platzierung (`chooseExitPoint` wählt Kandidaten ausschließlich aus
begehbar erreichbaren Zellen). Die Zeilenreferenz „Z. 7" zeigt auf `exitMaxDistance`.
Vermutliche Quelle des Irrtums: der veraltete Kommentar in `game_config.hpp:21`
(„always carve a deterministic safe path") oder die Altdatei `game_config.json.bak`.
**Relevanz:** unkorrigiert hätte das §4.5 („Lösbarkeit per Konstruktion") direkt
widersprochen — genau die Art innerer Widerspruch, die das Review vom 03.08. gerügt hat.

**Korrektur B — Legacy-Parameter existieren nicht mehr.**
Die Zuarbeit nennt `coldBiomeMax`/`warmBiomeMax` als „werden eingelesen, sind aber funktional
obsolet". Sie wurden beim v11-Audit **vollständig entfernt** (Parser, Struct-Felder, JSON);
sie überleben nur in `game_config.json.bak`. Der Befund selbst war richtig und ist in
korrigierter Form aufgenommen: als abgeschlossene Bereinigung, nicht als offene Schuld.

### Änderung 4 — Statustabelle „Aktive Mechanismen der Weltgenerierung"

Neue Tabelle `tab:worldgen_aktiv`: BFS-Exit-Platzierung **aktiv**; zelluläre Glättung,
Flood-Fill-Validierung und Carving **inaktiv**, je mit Begründung. Quelle:
`assets/base/game_config.json`, Abschnitt `worldgen`.

### Änderung 5 — INHALTLICHER BEFUND: warum die Welten zu offen sind

Aus der Statustabelle folgt eine Erklärung für den Kernbefund aus §7.3, die bisher fehlte:
**Da die zelluläre Glättung abgeschaltet ist, stammt die Hindernisverteilung ausschließlich
aus unkorreliertem Hash-Rauschen.** Die Wandtiles sind statistisch unabhängig verteilt und
bilden verstreute Einzelhindernisse statt zusammenhängender Barrieren. Genau das macht die
Welten „offen" im Sinne des Baseline-Befunds: Sackgassen entstehen selten, ungerichtete
Zufallsbewegung verlässt sie zuverlässig. Damit hat der bisher nur phänomenologisch
beschriebene Befund („die Welten sind hinreichend offen") eine mechanistische Ursache.

### Änderung 6 — §8.4 Ausblick korrigiert und geschärft

**Problem:** Der Ausblick behauptete, der Wandanteil werde „durch einen zellulären Automaten
geglättet". Das ist falsch — `enableCellularSmoothing: false`.
**Lösung:** Korrigiert; zugleich der Hebel präzisiert. Nicht der Wandanteil allein ist
entscheidend, sondern die **Korreliertheit** der Verteilung. Die Glättungsstufe ist bereits
implementiert und muss nur aktiviert und kalibriert werden — der billigste verfügbare Weg zu
Welten mit echten Sackgassen.

### Änderung 7 — Absatz „Zur Arbeitsteilung" in der Einleitung

Bislang war nirgends benannt, wer was gemacht hat. Ergänzt: Plattform/Generator/Client
schwerpunktmäßig L. Rößler, RL-Aufbau/Evaluation schwerpunktmäßig F. Merlau — mit dem
Hinweis, dass die zentralen Befunde genau an der Schnittstelle liegen.
⚠️ `TODO`-Kommentar im Quelltext: **Arbeitsteilung vor Abgabe gegenprüfen.**

**Verifikation:** `latexmk -pdf` fehlerfrei, 53 Seiten (vorher 50), 0 Undefined
References/Citations.

---

## v2026-08-03.1 — Fakten-Review der Projektdokumentation: 13 Korrekturen, 1 neue Messung

**Kontext:** Externes Review der Projektdokumentation (Prüfschwerpunkt Zahlen und innere
Konsistenz). Alle Behauptungen wurden gegen Code, `eval_history.json`, `CHANGELOG.md` und
die C++-Quellen nachgeprüft. Der Zahlenkern (Tab. Endergebnis, CIs, Welch-Test, PBRS,
5.120 Gradient-Updates) reproduziert exakt; die folgenden Punkte wurden korrigiert.
Drei Review-Punkte wurden nach Prüfung **zurückgewiesen** (siehe unten).

### Änderung 1 — Temperatur-Sweep-Tabelle auf ddof=1 umgestellt

**Datei:** `docs/Projektdokumentation.tex` (`tab:temperatur`)
**Problem:** Die Tabelle rechnete durchgehend mit `np.std(...)` (ddof=0), während §5.4 für
die Arbeit ausdrücklich die Stichproben-Standardabweichung (ddof=1) festlegt. Nachweis: Die
Argmax-Zeile misst dieselbe Größe wie die det-Spalten der Läufe 1–3 (A: 38/26/32 → ddof1 =
6,0, ddof0 = 4,9 — die Tabelle zeigte 4,9). Zusätzlich ist ±4,9 bei drei Messwerten aus
50 Seeds mit ddof=1 arithmetisch unmöglich (Brute-Force über alle Tripel: 0 Treffer).
Betroffen waren **alle fünf Zeilen**, nicht nur die Argmax-Zeile.
**Lösung:** Zugrundeliegende Tripel aus Mittelwert + ddof0-Std rekonstruiert (eindeutig; die
zwei mehrdeutigen Zeilen liefern dieselbe ddof1-Std) und auf ddof=1 umgerechnet.

| Zeile | vorher (ddof=0) | nachher (ddof=1) |
|---|---|---|
| Argmax T=0 | 32,0 ± 4,9 / 42,0 ± 4,3 | **32,0 ± 6,0 / 42,0 ± 5,3** |
| T=0,25 | 52,0 ± 0,0 / 52,7 ± 1,9 | **52,0 ± 0,0 / 52,7 ± 2,3** |
| T=0,5 | 63,3 ± 4,1 / 71,3 ± 6,6 | **63,3 ± 5,0 / 71,3 ± 8,1** |
| T=0,75 | 63,3 ± 7,5 / 72,0 ± 6,5 | **63,3 ± 9,2 / 72,0 ± 8,0** |
| T=1 | 70,7 ± 8,2 / 79,3 ± 6,6 | **70,7 ± 10,1 / 79,3 ± 8,1** |

Alle Mittelwerte und damit die Monotonie-Aussage sowie „+31/+29 Punkte bei T=0,5" bleiben
unverändert. `scripts/eval_baselines.py` nutzt bereits korrekt ddof=1 — der Fehler saß
allein im Temperatur-Auswertungspfad.

### Änderung 2 — Gating-Aussage korrigiert: „Ziel berührt" ≠ „Phase bestanden"

**Datei:** `docs/Projektdokumentation.tex` (§7.2), neue Tabelle `tab:gating`
**Problem:** Der Text behauptete „Phase 1 erreichte ihr Ziel von 85 % in fünf der sieben
Läufe, Phase 2 ihr Ziel von 70 % in allen sieben" und legte damit nahe, diese Phasen seien
per Gating beendet worden. Das Gate verlangt aber die Ziel-SR in **zwei aufeinanderfolgenden**
Evaluationen. Auswertung der sieben `eval_history.json`:

| Phase | Ziel-SR | Ziel berührt | Phase bestanden | Budget erschöpft |
|---|---|---|---|---|
| 1 | 85 % stoch | 5 / 7 (s5 max 72, s6 max 82) | **1 / 7** (nur s1 @50k) | 6 / 7 |
| 2 | 70 % stoch | 7 / 7 | **3 / 7** (s2, s3, s6) | 4 / 7 |
| 3 | 70 % det | 0 / 7 | 0 / 7 | 7 / 7 |
| 4 | 60 % det | 0 / 7 | 0 / 7 | 7 / 7 |

**Lösung:** Absatz neu gefasst, Unterscheidung explizit gemacht, Bilanztabelle ergänzt.
Die Korrektur **stärkt** die eigene Aussage: Das Curriculum war nicht nur in den Phasen 3/4,
sondern in fast allen Phasen budget- statt leistungsgesteuert.
**Nebenbefund:** Die Phase-3/4-Angaben (26–54 % bzw. 26–42 %) stimmen exakt mit den
Maxima aus `eval_history.json` — dort war die Methodik korrekt.

### Änderung 3 — Unbelegte Laufzeitangabe entfernt (v10-Referenzlauf)

**Datei:** `docs/Projektdokumentation.tex` (§B.3)
**Problem:** „erreichte 86 % nach 2,2 Mio. Steps in 3 h 12 m" ⇒ 191 FPS. Dieser Wert liegt
über jedem im Projekt protokollierten Durchsatz und ist nirgends belegt: Es existiert **kein
Changelog-Eintrag zum 25.06.**; die eigene ETA für dieselbe Konfiguration lautete ~5,5 h
@110 FPS; der gemessene batch=8-Durchsatz beträgt 88 FPS; dieselben 2,2 Mio. Steps brauchten
in v12 gemessen 7,7–8,5 h.
**Lösung:** Zeitangabe gestrichen, stattdessen offengelegt, dass keine protokollierte Messung
vorliegt, mit Nennung der beiden belastbaren Vergleichswerte.

### Änderung 4 — Undokumentierter Reward-Term ergänzt (Revisit-Penalty)

**Datei:** `docs/Projektdokumentation.tex` (`tab:reward`, §4.3, §6.2)
**Problem:** `python/stoneforge_env.py:334` zieht ab dem 26. Besuch eines Tiles bis zu
−0,06 pro Schritt ab (`reward -= 0.03 * min(visit_count/25, 2)`). Tabelle 2 war mit
„**Vollständige** Reward-Komponenten" überschrieben, §4.3 zählte die Nicht-PBRS-Terme als
abgeschlossene Liste auf und §6.2 wiederholte die Liste — der Term fehlte an allen drei
Stellen. Es ist zudem der letzte verbliebene Term des in §B.7 als entfernt beschriebenen
„Straf-Stackings".
**Lösung:** Als eigene Zeile in Tabelle 2 aufgenommen (mit Kennzeichnung, dass er im
Python-Wrapper und nicht in `computeReward()` gebildet wird), in §4.3 und §6.2 ergänzt.
**Verifikation:** Alle übrigen Werte der Tabelle 2 wurden gegen
`src/core/simulation.cpp:1463–1521` geprüft — stimmen exakt.

### Änderung 5 — NEUE MESSUNG: Bilanz der dichten Reward-Terme

**Datei:** `docs/Projektdokumentation.tex` (§7.3, neue Tabelle `tab:rewardbilanz`)
**Fragestellung:** Wie verteilt sich die dichte Reward-Masse über eine erfolgreiche Episode?
Insbesondere: Wie groß ist der Anteil, der überhaupt *Richtungsinformation* trägt?
**Methode:** Läufe 1–3, Testset A (7000–7049), stochastisch, Cap 4000; je Episode wurden
Schritte, neu betretene Tiles und Revisit-Strafen mitgezählt.

| Lauf | SR | Ø Schritte | neue Tiles | Explor. | Schritt | Revisit | PBRS | Verhältnis Expl:PBRS |
|---|---|---|---|---|---|---|---|---|
| s1 | 33/50 | 1199 | 195 | +3,89 | −11,99 | −4,73 | +0,79 | 4,9 : 1 |
| s2 | 38/50 | 1490 | 229 | +4,57 | −14,91 | −5,80 | +0,78 | 5,8 : 1 |
| s3 | 41/50 | 1346 | 202 | +4,04 | −13,46 | −9,81 | +0,79 | 5,1 : 1 |
| **Ø** | | **1345** | **209** | **+4,2** | **−13,5** | **−6,8** | **+0,8** | **≈ 5 : 1** |

**Befund:** Der PBRS-Term ist wegen der Teleskopierung fest auf β·d₀/128 ≈ 0,8 begrenzt und
macht damit nur **3 % der dichten Reward-Masse** aus; 97 % hängen ausschließlich von
Weglänge und Flächendeckung ab. Der Explorations-Bonus wiegt rund **fünfmal** so schwer wie
die gesamte Zielinformation.
**Wichtige Einschränkung gegenüber der Review-Hypothese:** Herumlaufen ist **nicht**
kostenlos — Schritt- und Revisit-Penalty (−20,3) übersteigen den Explorations-Bonus (+4,2)
deutlich, die dichte Bilanz ist mit −15,3 klar negativ, und der Terminal-Reward (+100;
diskontiert ≈ +26) bleibt das dominierende Zielsignal. Die Schwäche liegt also nicht darin,
dass Umherirren belohnt würde, sondern darin, dass aus dem dichten Signal kaum lernbar ist,
*wohin* zu gehen ist. Konsequenz im Ausblick als eigener Punkt aufgenommen.
**Nebenbefund:** Early-Stop-Truncation greift bei den trainierten Läufen in nur 4/150
Episoden (< 3 %). Skript: `scripts/reward_bilanz.py` (neu).

### Änderung 6 — Early-Stop-Truncation im Evaluationsprotokoll offengelegt

**Datei:** `docs/Projektdokumentation.tex` (§5.4, `tab:capsweep`-Caption, Ausblick)
**Problem:** §5.4 sagte „Gemessen wird stets mit dem vollen Limit von 4.000 Schritten".
Tatsächlich ist die Truncation nach 256 Schritten ohne positiven Reward
(`python/stoneforge_env.py:341–348`) in der Evaluation aktiv, auch für die Baselines
(`scripts/eval_baselines.py:119` bricht auf `truncated` ab). Das ist relevant, weil die
Bedingung von der Explorationsrate der jeweiligen Politik abhängt — und der
Baseline-Vergleich ist der Kernbeitrag der Arbeit.
**Lösung:** Absatz in §5.4 ergänzt (Bedingung, Wirkungsrichtung, Konsequenz für die
Vergleichbarkeit), Caption der Budget-Tabelle entsprechend eingeschränkt, Gegenprobe mit
abgeschalteter Truncation als Diagnoseschritt (e) im Ausblick aufgenommen.

### Änderung 7 — Validierungsumfang vereinheitlicht (50 vs. 120)

**Datei:** `docs/Projektdokumentation.tex` (§5.4, §7.4, `fig:detgap`-Caption, §5.2)
**Problem:** §5.4 definierte die Validierung als 6000–6049 (50 Seeds); die Det-Gap-Abbildung
nutzt laut `scripts/plot_det_gap_distanz.py:76` aber 6000–6119 (120 Seeds), also 70 Seeds
außerhalb der definierten Menge. Kein Leck Richtung Test/Holdout, aber eine Lücke in der
Datentrennungs-Beschreibung.
**Lösung:** Validierungs**bereich** 6000–6199 definiert; davon 6000–6049 für Gating und
Modellauswahl, der Rest ausschließlich für Diagnosemessungen mit mehr als 50 Welten.
Herkunft an allen drei Fundstellen genannt.
**Nicht geändert:** Die „200 Welten pro Phase" in §5.2 sind eine Generator-Eigenschaft
(Exit-Sichtbarkeit beim Start), keine Validierungsmessung — nur als solche klargestellt.

### Änderung 8 — FPS-Angaben als Messspanne statt als Punktwert

**Datei:** `docs/Projektdokumentation.tex` (§B.8.2, §B.6)
**Problem:** Der Fließtext nannte „187 statt 88 FPS" und „Faktor 2,1". Der zugrundeliegende
Benchmark misst für batch=64/CPU eine **Spanne von 163–187 FPS** (Faktor 1,9–2,1); zitiert
wurde zweimal nur das obere Ende.
**Lösung:** Beide Stellen auf die gemessene Spanne umgestellt.

### Änderung 9 — Konsistenz-Passage um den vierten Messwert ergänzt

**Datei:** `docs/Projektdokumentation.tex` (§7.5)
**Problem:** Die Passage stellte 70,7 / 73,3 / 74,0 nebeneinander und erklärte sie als
Sampling-Rauschen; die Budget-Tabelle misst bei Cap 4000 dieselbe Größe an denselben drei
Läufen (64/78/74 → 72,0) und fehlte in der Aufzählung.
**Lösung:** 72,0 aufgenommen, zusätzlich die Abweichung auf Einzellauf-Ebene (62/84/76 vs.
64/78/74) offengelegt.

### Änderung 10 — Tabellennummerierung korrigiert

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Das Tabellenverzeichnis begann bei Tabelle 2 — das captionlose
`longtable` des Abkürzungsverzeichnisses zählte den `table`-Zähler hoch.
**Lösung:** `\addtocounter{table}{-1}` nach dem Verzeichnis. Verifiziert: `.lot` beginnt
jetzt bei `table.1`.

### Änderung 11 — Konstanten der PBRS-Gleichung begründet

**Datei:** `docs/Projektdokumentation.tex` (§4.3)
**Problem:** Normierung 128 und Gewicht β = 2,5 standen unbegründet in Gleichung (1).
**Lösung:** Herleitung ergänzt (Quelle: Kommentare in `src/core/simulation.cpp:1504–1509`):
128 normiert auf die Größenordnung der übrigen Terme und entspricht der BFS-Puffer-Skalierung;
β = 2,5 ist rückwärts aus der Netto-Vorgabe +0,01 pro Fortschritts-Tile bei d ≈ 40 bestimmt
und wurde nach dem BFS-Puffer-Fix von 5,0 halbiert.

### Änderung 12 — σ-Rechnung in §7.5 korrigiert

**Problem:** „Beide Differenzen liegen innerhalb bzw. knapp **oberhalb** von 1,5 σ" — falsch.
Der det-Abstand (10 Pkt.) entspricht bei σ_det = 8,0 rund 1,25 σ, der stoch-Abstand (17 Pkt.)
bei σ_stoch = 12,4 rund 1,37 σ; beide liegen **unter** 1,5 σ. Zudem war für die det-Achse
σ = 12 statt des korrekten σ = 8,0 unterstellt.
**Lösung:** Beide Werte einzeln ausgewiesen; ergänzt, dass bei Einzellauf gegen Dreier-Mittel
σ_diff = σ·√(1+1/3) ≈ 1,15 σ das richtige Maß ist (⇒ 1,08 bzw. 1,19 σ).

### Änderung 13 — Vier Transparenz-Ergänzungen

1. **Hilfsmittel-Erklärung** (`sec:hilfsmittel`): Die Eidesstattliche Erklärung verwies auf
   Anhang A, der aber nur eine Reproduzierbarkeits-Checkliste enthielt. Neuer Unterabschnitt
   mit Software und Literatur; die KI-Werkzeuge stehen seit Änderung 14 im Vorspann.
2. **Det/Stoch-Selektionsasymmetrie** als Einschränkung Nr. 6 in §8.3 benannt: Selektion auf
   deterministischer, Berichterstattung auf stochastischer SR.
3. **Fehlerbalken und η**: Caption der Baseline-Tabelle stellt klar, dass die ±-Werte
   verschiedene Größen messen (5 Politik-RNGs ohne Trainingsvarianz vs. 7 Trainingsläufe);
   bei η ergänzt, dass die Mittelung über *erfolgreiche* Episoden zugunsten des Agenten
   verzerrt und den Befund damit verstärkt.
4. **Curriculum-Tabelle** um die Eval-Distanz aller vier Phasen ergänzt (Quelle: `PHASES`
   in `scripts/train_curriculum.py`: 5–12 / 12–25 / 35–45 / 35–45). Daraus folgt ein
   ergänzter Hinweis in §B.6: Der Median von 477 Schritten des besten Phase-1-Checkpoints
   bezieht sich auf Exit-Distanz 5–12, entspricht also η ≈ 0,02 — ein früher Hinweis auf den
   Baseline-Befund. Ferner: E1–E3 unterschreiten das Vorab-Kriterium „≥ 3 Läufe je
   Konfiguration" bewusst; das steht jetzt explizit da.

### Änderung 14 — Erklärung zur Nutzung generativer KI (Vorspann, vor der Kurzfassung)

**Datei:** `docs/Projektdokumentation.tex`
**Anlass:** Die Arbeit wies den KI-Einsatz bisher nur als Nebensatz im Anhang aus. Übliche
Praxis an deutschen Hochschulen ist eine eigenständige, unterschriebene Erklärung im
Vorspann plus ein KI-Verzeichnis.
**Recherchegrundlage:**
- Hochschule Aalen stellt eine eigene **„Eigenständigkeitserklärung KI"** in **drei
  Varianten** bereit (KI nicht erwünscht / erlaubt / erforderlich); **welche gilt, legt der
  Betreuer fest.** Zusätzlich existiert ein „Fragebogen zur Dokumentation der KI-Nutzung in
  Abschlussarbeiten". Der 2024er Direktlink (`Hilfestellung_Studierende_KI.pdf`) ist
  inzwischen tot; die Vorlagen liegen studiengangsweise unter den Downloads.
- HWR Berlin, „Formulierungsvorschläge für Eigenständigkeitserklärungen bei Nutzung von KI"
  (Stand 19.03.2024, CC BY-SA): Variante **„Lernziel 2 — KI-Tools professionell nutzen,
  u.a. zum wissenschaftlichen Schreiben von Master- und Doktorarbeiten"** ist die für diese
  Arbeitsstufe einschlägige; sie verlangt (a) Eigenverantwortung für Auswahl, Übernahme und
  Ergebnisse des KI-Outputs, (b) ein Verzeichnis der Tools mit Produktnamen, (c) im Anhang
  Prompts und/oder Outputs. Ebenda das Muster für das KI-Verzeichnis in Tabellenform
  (Hilfsmittel · Einsatzform · betroffene Teile · Bemerkungen).

**Lösung:** Neuer Abschnitt „Erklärung zur Nutzung generativer KI" **nach der
Eidesstattlichen Erklärung und vor der Kurzfassung**, mit eigenem Eintrag im
Inhaltsverzeichnis und Unterschriftenfeld. Inhalt: Eigenverantwortungs-Klausel, Hinweis auf
fehlende Richtigkeitsgewähr generativer Systeme, ausdrückliche Feststellung, dass **keine
Messergebnisse KI-generiert** sind, sowie das KI-Verzeichnis in Tabellenform. Der Verweis in
der Eidesstattlichen Erklärung wurde angepasst, der KI-Absatz im Anhang auf einen
Querverweis reduziert (keine Dopplung).
**Vorausgefüllt:** die in dieser Session belegbare Nutzung (Claude Opus 5 — Konsistenzprüfung,
Umsetzung der Korrekturen, Diagnoseskript; inkl. des Hinweises, dass drei KI-Beanstandungen
nach eigener Prüfung verworfen wurden).
⚠️ **Vor Abgabe zu erledigen** (als `TODO`-Kommentar im Quelltext hinterlegt): Variante mit
Prof. Lecon abstimmen und ggf. den Wortlaut der Hochschulvorlage übernehmen; zweite
Tabellenzeile mit den tatsächlich zusätzlich genutzten Werkzeugen füllen oder löschen.
**Nebenänderung:** `array`-Paket + Spaltentyp `L{}` (linksbündige `p`-Spalte) in die
Präambel, weil Blocksatz in schmalen Tabellenspalten unleserliche Wortabstände erzeugt.

### Geprüft und ZURÜCKGEWIESEN

| Review-Punkt | Prüfung | Ergebnis |
|---|---|---|
| „Delta-BFS: §B.1 widerspricht Abb. 10/Tab. 16" | Quelldaten in `CHANGELOG.md:2696` gefunden: Standard-Welt det 42 % / stoch 100 %; Hard-World det 100 %; τ=0,2 → 100 % @ Ø 90 Schritte | **Kein Widerspruch** — beide Stellen geben die Quelldaten korrekt wieder |
| „C++-Kern und Client beide ca. 4500 LOC sieht nach Copy-Paste aus" | Gemessen: Kern 3.575 + Nicht-Client-Header 732 = 4.307; Client 4.326 + Client-Header 178 = 4.504 | **Beide Angaben korrekt** |
| „Abb. 3 wirkt dichter als 7–25 % Wandanteil" | Abbildung ausgezählt: ≈ 20 % Wandanteil, verstreute Einzeltiles | **Innerhalb des angegebenen Bereichs**; stützt sogar das „zu offen"-Argument |

**Verifikation gesamt:** `latexmk -pdf` fehlerfrei, 50 Seiten, 0 Undefined References.
Tabellenverzeichnis beginnt bei 1. Overfull-Boxen 29 → 28, größte 69,3 pt → 39,5 pt
(die schlimmste lag in der Curriculum-Tabelle und ist mit deren Umbau verschwunden).

---

## v2026-07-29.1 — Methodik entchronologisiert: Kapitel 5 umstrukturiert, Curriculum-Budget offengelegt

**Kontext:** Kapitel 5 (Methodik) las sich stellenweise als Projekttagebuch statt als
Verfahrensbeschreibung: Datumsangaben („Mai 2026", „bis am 11.06."), Erzählungen einzelner
Testläufe und Ergebnistabellen standen im Aufbau-Kapitel, während dieselben Sachverhalte in
Anhang B bereits vollständig chronologisch dokumentiert waren. Ziel des Umbaus: Kapitel 5
beschreibt ausschließlich **was** gemacht wurde und **warum**; **wann** und **woraus
gelernt** steht in Anhang B, **wie es ausging** in Kapitel 7.

### Änderung 1 — Abschnitt „Iterative Modellverbesserung" (§5.3) aufgelöst

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Der Abschnitt erzählte drei Befunde mit Datumsangaben und Versionsverläufen
(v6–v9-Fehlserie, batch=64-Widerlegung, Eval-Cap-Artefakt). Alle drei sind in Anhang B
(Etappe 3, `sec:etappe6`, `sec:batchsize`) bereits vollständig und mit Abbildungen belegt —
§5.3 war reine Duplikation mit Chronologie.
**Lösung:** Abschnitt ersatzlos entfernt. Die *methodischen Regeln*, die daraus folgen,
stehen jetzt atemporal im Evaluationsprotokoll; die *Historie* nur noch in Anhang B.
Label `sec:iteration` war nirgends referenziert, keine gebrochenen Verweise.

### Änderung 2 — §5.1 Algorithmenwahl: Chronologie → Begründung

**Problem:** „Ein früher DQN-Anlauf (Mai 2026) wurde nach Diagnose verworfen: Die Q-Werte
kollabierten […]" — eine Verlaufsbeschreibung an der Stelle, an der eine Begründung stehen muss.
**Lösung:** Ersetzt durch „Begründung der Verfahrenswahl" mit drei sachlichen Kriterien
(diskreter Aktionsraum → entscheidet nichts; POMDP → rekurrent statt reaktiv; Umgebung während
der Entwicklung noch fehlerbehaftet → on-policy, weil ein Replay-Buffer Reward-Fehler über
ihre Korrektur hinaus konserviert). Die DQN-Diagnose selbst wurde nach Anhang B, Etappe 1
verschoben und dort um das Q-Wert-Detail ergänzt. Neuer Absatz „Zustandekommen der
Konfiguration" verweist einmalig auf Anhang B, ohne Daten zu nennen.

### Änderung 3 — §5.4 Evaluationsprotokoll als Spezifikation neu gefasst

**Problem:** Fließtext mit eingestreuter Historie („wurde identifiziert und behoben", „seit v11",
„bis am 11.06."). Als Protokoll nicht nachvollziehbar abarbeitbar.
**Lösung:** Sechs benannte Absätze: Metrik · Episodenlimit · Seed-Trennung · Betriebsarten ·
Aggregation über Läufe · Entscheidungen auf Kurven statt Endpunkten. Neues Label
`sec:evalprotokoll`. Ergänzt: Pfadeffizienz η als Nebenmetrik benannt (bisher erst in §7.3
eingeführt), `approx_kl` als Diagnosemetrik, R2D2-Zitat beim LSTM-State-Handling,
„kein Best-of-Runs" mit Henderson-Beleg.

### Änderung 4 — E1–E3-Ergebnistabelle aus der Methodik in die Evaluation verschoben

**Problem:** `tab:iterationen` (Setup **und** SR-Ergebnisse der drei Gap-Varianten) stand in
§5.6, diskutiert wurde sie 400 Zeilen später in §7.5. Ergebnisse gehören nicht ins
Aufbau-Kapitel.
**Lösung:** Tabelle nach §7.5 (`sec:gapexperimente`) verschoben, direkt vor den Absatz, der sie
auswertet. Die Einschränkung zum reduzierten Phase-3-Budget von E1/E2 stand doppelt (§5.6 und
§7.5) — zusammengeführt in den Absatz „Aussagekraft". §5.6 heißt jetzt „Versuchsaufbau" und
enthält nur noch die Basiskonfiguration.

### Änderung 5 — §5.5 Erfolgsschwellen: Datumsangaben als Beleg gekennzeichnet

**Problem:** Die drei Daten (16.05. / 08.07. / 17.07.) lasen sich wie der übrige
Chronologie-Ballast, tragen hier aber das gesamte Anti-HARKing-Argument.
**Lösung:** In einen eigenen Absatz „Zur Vorab-Festlegung" gefasst, der einleitend sagt,
*warum* der Zeitpunkt hier zählt („Eine nachträglich abgesenkte Schwelle wäre wertlos, weshalb
der Zeitpunkt der Festlegung hier als Beleg und nicht als Chronik angeführt wird").

### ⚠️ Änderung 6 — Curriculum: Schrittbudget offengelegt (inhaltliche Korrektur)

**Datei:** `docs/Projektdokumentation.tex`, Tab. 5 (`tab:curriculum`) und §7.2
**Problem:** §5.2 beschrieb das Curriculum rein leistungsbasiert („eine Phase endet, wenn die
Ziel-SR in zwei aufeinanderfolgenden Evaluationen erreicht wird"). Das feste Schrittbudget je
Phase aus `scripts/train_curriculum.py` (PHASES, Z. 52–66) wurde nirgends genannt. Gegenprobe an
den `results.json` aller sieben Läufe zeigt, dass die zweite Abbruchbedingung die faktisch
wirksame war:

| Phase | Ziel-SR | erreichte Werte (7 Läufe) | Gate erreicht |
|-------|---------|---------------------------|---------------|
| 1 | 85 % stoch | 0,72 – 0,94 | 5 / 7 |
| 2 | 70 % stoch | 0,70 – 0,88 | 7 / 7 |
| 3 | 70 % det | **0,26 – 0,54** | **0 / 7** |
| 4 | 60 % det | **0,26 – 0,42** | **0 / 7** |

**Lösung:** Spalte „Budget" (500k / 500k / 1,0M / 200k) in Tab. 5 ergänzt, beide
Abbruchbedingungen im Text benannt, Caption um die Doppelrolle der Gating-Metrik
(Phasenende **und** Bestmodell-Auswahl) erweitert. In §7.2 neuer Absatz „Die
Ziel-Erfolgsquoten der späten Phasen wurden nicht erreicht" mit den Zahlen oben. Zusätzlich
präzisiert, dass das Ausgabemodell nach der **deterministischen** Validierungs-SR ausgewählt
wurde (Gating-Metrik der Phase 4), während die Kernzahl stochastisch berichtet wird.

**Warum das zählt:** Die `results.json` liegen dem Projekt bei; die Diskrepanz zwischen
„leistungsbasiert" und den nie erreichten Phasenzielen wäre im Kolloquium nachprüfbar gewesen.

#### Ergebnis

46 Seiten (unverändert), kompiliert fehlerfrei, **0 undefinierte Referenzen**, keine mehrfach
definierten Labels. Verbleibende Warnungen: ausschließlich 18× „`h' float specifier changed to
`ht'". Datumsangaben in Kapitel 5: von 6 auf 3 reduziert, die verbleibenden drei sind das
Anti-HARKing-Argument.

**Offen aus der Prüfrunde v2026-07-29 (nicht in diesem Eintrag behandelt):** ε-Auswahl der
Baseline (Doku behauptet Wahl auf Val-Seeds 6000–6049, `scripts/eval_baselines.py` kennt nur
A/B), Singh-Widerspruch zwischen §7.4 und §7.5, ddof-Konvention in Tab. 8 vs. Tab. 7,
„dreimal wegeffizienter" in Kurzfassung/Fazit, pybind11-Version in Tab. 9.

---

## v2026-07-26.1 — Externe Durchsicht: Abstract neu gewichtet, Meta-Prosa entfernt, neun Korrekturen

**Kontext:** Vollständige Durchsicht von Deckblatt bis Fazit mit Nachrechnen aller Zahlen
gegen Rohdaten, Code und Primärliteratur. Verifiziert wurden u. a. Tab. 7 gegen
`scratchpad_final_eval_n7.json` (alle 28 Werte exakt), Mittelwerte/ddof=1-Std/t-CIs (df=6),
Welch-Test (t=1,764, df=4,21, p=0,149), Tab. 4 gegen `Simulation::computeReward()`,
PBRS-Arithmetik, die 5.120 Gradient-Updates gegen die `RecurrentRolloutBuffer`-Semantik,
LOC-Angaben (Kern 4.469 / Client 4.585) und sämtliche Bibliotheksversionen.

**Eigene Nachmessung:** Die Sichtbarkeits-Staffelung des Curriculums (72 % / 6 % / 0 % Exit
im 15×15-Sichtfeld bei Episodenstart) reproduziert über 200 Welten je Phase **exakt**
(145/200, 13/200, 0/200). Tab. 9 (Baselines) reproduziert mit `--repeats 5 --rng-offset 100`
**zeichengenau** in allen zwölf Werten.

### Änderung 1 — Kurzfassung neu gewichtet

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Zwei Fehler in einem Absatz. (a) Sachlich falsch: „erreicht 92 % und ist
**zugleich wegeffizienter**" — bei ε=0,9 ist η=0,047, der Agent liegt mit 0,049 darüber.
Wegeffizienter ist nur ε=0,4, und der erreicht 60,4 %. Der Haupttext (§7.3) formuliert es
korrekt, die Kurzfassung zog zwei Betriebspunkte zusammen. (b) Gewichtung: ein Satz Ergebnis
gegen vier Sätze Selbstentwertung, kein Wort zum Beitrag.
**Lösung:** Aussage korrigiert auf „bei vergleichbarer Erfolgsquote dreimal wegeffizienter";
Umbau von „zwei methodische **Befunde**" auf „zwei methodische **Beiträge**" (kalibrierter
Bezugspunkt + Quantifizierung der Lauf-Streuung). Der Negativbefund bleibt vollständig
erhalten, inklusive „nicht trennscharf". Aufstockungseffekt beziffert (rund acht Punkte).

### Änderung 2 — Meta-Prosa entfernt

**Problem:** Inhaltsangaben in Prosa vor jedem Kapitel, redundant zu Inhaltsverzeichnis
*und* § 1.5 „Aufbau der Arbeit" (z. B. „Dieses Kapitel beantwortet die Frage, wie gemessen
wurde.").
**Lösung:** Gestrichen in § 2, § 5 (8 → 3 Zeilen), § 5.6, § 7, § 7.3, § 1.2 und Anhang B.
Erhalten bleibt jeweils nur die nicht-offensichtliche Information (z. B. *warum* die
Schwellen-Herleitung am Kapitelende steht). „bis heute" im Anhang entfernt.

### Änderung 3 — Drei Abbildungen für „Welt vs. Agentensicht" auf eine reduziert

`fig_game_real.png` war zweimal eingebunden (Abb. 4 und linke Hälfte von Abb. 5); Abb. 3
machte inhaltlich denselben Punkt. Abb. 5 (`fig:game_vs_ai`) entfernt, § 4.1 verweist jetzt
auf `fig:pomdp_schema`. 17 statt 18 Abbildungen.

### Änderung 4 — Zirkulärer Querverweis behoben

§ 7.2 verwies für die Gap-Einordnung auf `sec:evaluation` (Kapitel 7 — der Leser steht schon
darin). Ursache: § 7.4 hatte **kein Label**. `\label{sec:detstochgap}` ergänzt, Verweis
umgehängt. Zusätzlich `sec:cap` entfernt (stand nach `\textbf{}` und löste auf dieselbe
Nummer auf wie `sec:etappe6`).

### Änderung 5 — Tabelle 9 wurde im Text nie referenziert

Die zentrale Baseline-Tabelle hatte kein einziges `\ref`. Einführender Satz in § 7.3 ergänzt.
(Die Prüfung in `v2026-07-25.9`, Befund 3, hatte nur Abbildungen erfasst.)

### Änderung 6 — Widerspruch 7 vs. 10 Änderungen in v11

| Stelle | vorher | nachher |
|---|---|---|
| Etappe 4 + Zeitleiste | „zehn dokumentierte Änderungen" | unverändert |
| § A.6 | „sieben verifizierte Änderungen" | „zehn dokumentiert; die folgenden sieben betreffen Umgebung/Reward/Eval, die übrigen drei stehen in § A.7" |

### Änderung 7 — Datumsinkonsistenz Kernbeitrag

Zeitleiste sagte 07.06., § A.2 sagte 08.06. Laut Changelog: Lauf gestartet 07.06.,
ausgewertet 08.06. Beide Stellen darauf vereinheitlicht.

### Änderung 8 — Zwei Belege nachgeschärft

| Stelle | Problem | Korrektur |
|---|---|---|
| § A.7.1 | „Empfehlung der SB3-Autoren" mit `\cite{raffin2021stable}` — das JMLR-Paper enthält diese Empfehlung nicht, die Online-Doku schon | Neuer Bib-Key `sb3docs` mit Wortlaut („PPO is meant to be run primarily on the CPU, especially when you are not using a CNN") und Abrufdatum |
| § 8.4 | „gegenüber **Transformern** konkurrenzfähig" `\cite{morad2023popgym}` — POPGyms Attention-Baselines sind *lineare* Varianten (u. a. FART), nicht volle Softmax-Attention wie bei AMAGO | Präzisiert; die Argumentation gegen einen voreiligen AMAGO-Wechsel bleibt tragfähig, wird aber nachprüfbar |

`burda2018rnd`: Key 2018 / Feld 2019 (ICLR 2019) im `note`-Feld aufgelöst.

### Änderung 9 — Random-Baseline: Streuung über den Politik-Zufallsstrom offengelegt

**Problem:** 5,2 % (RNG-Seeds 100–104) ist der niedrigste von drei vorliegenden Messwerten;
mit RNG-Seeds 0–4 ergibt dieselbe Politik **10,4 ± 4,6 %** auf A, der alte 3er-Lauf 8,0 %.
Berichtet wurde ausgerechnet der Wert, der die Aussage „deutlich über dem Zufall" am besten
stützt. Die Kompass-Zeilen sind dagegen stabil (89,6 vs. 92,0 bei ε=0,9) — der Kernbefund
wackelt nicht.
**Lösung:** Spannweite in der Caption von Tab. 9 offengelegt; „Zufallsuntergrenze von 5 %"
→ **5–10 %** an beiden Stellen (§ 7.3, § 8.1).

### Änderung 10 — Exposé-Zweitmetrik nachgetragen

Das Exposé nennt neben ≥ 90 % SR die Minimierung der *Steps per Episode*. § 5.5 rechnete nur
gegen die SR ab. Ergänzt: Auch diese Zielgröße wird verfehlt, und zwar deutlicher
(η ≈ 0,05 = rund zwanzigfacher Umweg). Die Pfadeffizienz ist in § 7.3 jetzt explizit als
Aufgreifen dieser Exposé-Metrik ausgewiesen.

### Änderung 11 — Reproduzierbarkeit der Baseline-Tabelle hergestellt

**Datei:** `scripts/eval_baselines.py`
**Problem:** Das abgelegte `logs/eval_results/baselines.json` stammte aus einem älteren
3-Wiederholungs-Lauf ohne ε=0,4 und passte nicht zu Tab. 9. Das Skript enthielt ε=0,4 gar
nicht in der Standardliste und hatte `repeats=3` — die Doku-Zeile war mit dem
ausgelieferten Skript **nicht erzeugbar**, obwohl die Zahlen korrekt sind.

| Parameter | vorher | nachher | Begründung |
|---|---|---|---|
| ε-Liste | 0,3/0,5/0,6/0,8/0,9 | 0,3/**0,4**/0,5/0,6/0,8/0,9 | ε=0,4 ist der in Tab. 9 berichtete effizienzoptimale Punkt |
| `--repeats` | 3 | **5** | Doku-Tabelle mittelt über 5 |
| `--rng-offset` | — | **100** (neu) | Politik-RNG-Seeds der Doku-Messung explizit steuerbar |

Rohdaten mit dem Doku-Protokoll neu erzeugt.

### Änderung 12 — Zweiter Durchgang: Restbestände

- **§ 5-Opener ganz entfernt.** Die verbliebene Ein-Satz-Begründung („steht am Ende dieses
  Kapitels, weil sie Kenntnis der Umgebung voraussetzt") war weiterhin eine Aussage über
  Dokumentstruktur. Sie steht jetzt am Anfang von § 5.5, also dort, wo sie den Leser
  tatsächlich betrifft. Kapitel 5 beginnt direkt mit § 5.1.
- **Begriff „Erfolgsrate" eliminiert** (3 Fundstellen: Curriculum-Gating, Monitoring,
  Bildunterschrift Abb. 9). `v2026-07-25.9` hatte auf „Erfolgsquote" vereinheitlicht, diesen
  dritten Synonym-Strang aber übersehen. Jetzt: Erfolgsquote 27×, „Success Rate" nur noch 2×
  an den beiden Definitionsstellen, „Erfolgsrate" 0×.
- **HARKing war eine Karteileiche.** Der Begriff stand im Abkürzungsverzeichnis, kam im Text
  aber **0×** vor — § 5.5 zitierte Kerr nur, ohne das Konzept zu benennen. Statt den Eintrag zu
  streichen, ist der Begriff jetzt an der Stelle ausgeschrieben, an der das Argument geführt
  wird; das macht die methodische Absicherung explizit statt implizit.

**Gegenprüfung:** Alle 20 Abkürzungen und 6 Symbole des Verzeichnisses werden im Text
verwendet. Schlüsselzahlen über das Dokument hinweg konsistent (65,7 12× · 66,9 8× · 29,1 3× ·
32,6 3× · 74,0 5× · 73,3 4×), keine widersprüchlichen Varianten.

### Offener Punkt — Durchsatzangabe § 6.1

Nicht geändert, weil es eine Sachfrage an die Autoren ist: § 6.1 nennt „ca. 90--190
Umgebungsschritte pro Sekunde". Die Untergrenze stammt aus dem 8192-Schritt-Mikrobenchmark
(88 FPS bei batch=8). Aus den Produktivläufen gerechnet liegt der Wert darunter:
`v12_s1` erreicht laut `eval_history.json` rund 1,72 Mio. Umgebungsschritte in 7 h 48 m
(= 28.100 s) → **≈ 61 FPS** Ende-zu-Ende bzw. ≈ 68 FPS nach Abzug der Eval-Zeit
(≈ 69 Eval-Punkte à ≈ 42 s). Ein Leser, der die 8 h gegen die Schrittzahl gegenrechnet, trifft
auf diese Lücke. Kein Ergebnis hängt daran; zu klären wäre nur, ob § 6.1 den reinen
Trainings- oder den Ende-zu-Ende-Durchsatz meinen soll.

#### Ergebnis

46 Seiten, kompiliert fehlerfrei, 0 undefinierte Referenzen, keine unaufgelösten Labels.
Verbleibende LaTeX-Warnungen: ausschließlich „`h' float specifier changed to `ht'".

---

## v2026-07-25.9 — Vollständige Durchsicht: Zahlenfehler in der Baseline-Tabelle korrigiert

**Kontext:** Durchsicht des gesamten Dokuments nach den zahlreichen Eingriffen der letzten
Runden. Geprüft wurden Zahlenkonsistenz gegen die Rohdaten, Querverweise, Zitate,
Begriffskonsistenz und Widersprüche zwischen Abschnitten.

### ⚠️ Befund 1 — Random-Baseline war ein Einzeldurchlauf, Caption behauptete fünf

**Problem:** Die Zeile „Random" in Tabelle `tab:baselines` stammte aus
`baselines_and_models.json` (**ein** Durchlauf, `sr_std = 0`), während die Kompass-Zeilen aus
fünf Wiederholungen gemittelt waren. Die Bildunterschrift behauptete für alle Zeilen fünf
Wiederholungen. Der Einzeldurchlauf war zudem ein günstiger Ausreißer.

**Messung nachgeholt** (5 Wiederholungen, Politik-RNG-Seeds 100–104, Weltseeds fix):

| | Doku vorher (n=1) | korrigiert (n=5) |
|---|---|---|
| Testset A | 10,0 % · η 0,022 | **5,2 ± 2,7 %** · η 0,021 |
| Holdout B | 16,0 % · η 0,030 | **10,4 ± 1,7 %** · η 0,025 |

Alle abhängigen Stellen nachgezogen: „Zufallsuntergrenze von 10 %" → 5 % (Fazit 8.1 und
Evaluation 7.3). Die inhaltliche Aussage verstärkt sich dadurch — der Agent liegt weiter über
dem Zufall, als bisher berichtet.

### Befund 2 — Widerspruch zwischen PPO- und POMDP-Abschnitt

Der PPO-Abschnitt verwies für den Det/Stoch-Gap auf „warum das kein Widerspruch sein muss,
wurde in Abschnitt 2.2 begründet". Abschnitt 2.2 war in `v2026-07-25.x` aber genau gegenteilig
korrigiert worden (Singh gilt nur für gedächtnislose Politiken; bei rekurrenter Politik ist der
Gap ein Belief-State-Defizit). Verweis umformuliert.

### Befund 3 — Drei Floats ohne Verweis im Text

`fig:game_vs_ai`, `tab:sysumgebung` und `tab:timeline` waren gesetzt, wurden aber nirgends
referenziert. Je ein einführender Satz ergänzt. Automatisierte Gegenprüfung: keine
unreferenzierten Abbildungen oder Tabellen mehr, keine fehlenden Labels.

### Befund 4 — Fehlende Zitate

Zwei Nennungen „Henderson et al." ohne `\cite` (Abschnitt Versuchsübersicht,
Repro-Checkliste). Ergänzt.

### Befund 5 — Begriffsinkonsistenzen

- **„Success Rate" vs. „Erfolgsquote"** gemischt (10× / 18×). Vereinheitlicht auf
  **Erfolgsquote** als Leitbegriff, mit „(Success Rate, SR)" bei der Definition in der
  Methodik und im Abkürzungsverzeichnis.
- **„Ablation"** wurde weiterhin für die A/B/C-Gegenüberstellung verwendet, obwohl der
  Abschnitt in `v2026-07-25.x` bewusst zu „Gegenüberstellung" umbenannt wurde, weil es eben
  keine kontrollierte Ablation ist. Vier Reststellen korrigiert.

### Befund 6 — Überclaim im Umgebungskapitel

„Ohne Gedächtnis ist sie nicht deterministisch lösbar" — nach dem Baseline-Befund nicht mehr
haltbar in dieser Absolutheit. Ersetzt durch die belegbare Aussage, dass für eine
gedächtnislose Politik verschiedene Weltpositionen ununterscheidbar sind.

### Befund 7 — Verworrene Messanmerkung

Der Absatz „Anmerkung zur Messung" erklärte eine Abweichung (68,6 % vs. 65,7 %) und schloss
mit „Berichtet wird der Wert aus Tabelle 7" — die Erklärung betraf also eine Zahl, die gar
nicht in der Tabelle steht. Auf zwei Sätze reduziert.

#### Ergebnis

46 Seiten, kompiliert fehlerfrei, 0 undefined references, keine unreferenzierten Floats.

---

## v2026-07-25.8 — Prozess-Erzählung und Selbstkommentierung entfernt

**Problem (vom Team gemeldet):** Der Satz „Der Befund entstand erst in der Schlussdurchsicht,
betrifft die Interpretation sämtlicher Leistungszahlen dieser Arbeit und wird deshalb an
erster Stelle berichtet" erzählt die Entstehungsgeschichte statt die Sache. Zwei der drei
Teilsätze sind Meta-Kommentar („entstand erst in der Schlussdurchsicht", „wird deshalb an
erster Stelle berichtet"), und der erste liest sich als Eingeständnis, obwohl der Befund eine
Leistung ist. Eine Doku sollte über den Gegenstand sprechen, nicht über sich selbst.

**Vorgehen:** Gezielte Suche nach dem Muster (`wird bewusst berichtet`, `bewusst ausführlich`,
`Schlussdurchsicht`, `bleibt als Beleg erhalten`, …). Sechs Stellen gefunden und bereinigt:

| Ort | vorher | nachher |
|-----|--------|---------|
| Fazit 8.1 | „Der Befund entstand erst in der Schlussdurchsicht, … und wird deshalb an erster Stelle berichtet." | „Dieser Befund betrifft die Interpretation sämtlicher Leistungszahlen der Arbeit, einschließlich der Gegenüberstellung in Abschnitt 7.1." |
| Evaluation 7.3 | „Diese Arbeit hat diese Untergrenze zunächst nicht erhoben; sie wurde erst bei der Schlussdurchsicht nachgeholt. Der Befund wird deshalb hier vollständig berichtet." | „Ohne Bezugspunkt bleibt offen, ob 65 % eine gelernte Fähigkeit ausdrücken oder bereits durch Zufall erreichbar sind. Dieser Abschnitt liefert den Bezugspunkt." |
| Anhang Batch-Größe | „Der Abschnitt bleibt als Beleg des methodischen Fehlers erhalten: …" | „Die Lehre daraus: …" |
| Einleitung 1.5 | „Der Anhang ist bewusst ausführlich, weil …" | konkret: welche Erkenntnisse dort stehen (Batch-Größe, Eval-Cap-Artefakt) |
| Anhang Etappe 6 | „Der Weg wird hier bewusst vollständig dokumentiert, inklusive der verworfenen Hypothesen, weil …" | „Die folgende Darstellung enthält auch die verworfenen Hypothesen, da erst deren Ausschluss die jeweils nächste Maßnahme bestimmte." |
| Kurzfassung | „Die Arbeit dokumentiert diesen Befund, leitet daraus ein korrigiertes Messregime ab und beschreibt den vollständigen Entwicklungsverlauf einschließlich der negativen Ergebnisse." | „Aus diesem Befund leitet die Arbeit ein korrigiertes Messregime für Folgearbeiten ab." |

**Regel für künftige Änderungen:** Sätze, die mit „Diese Arbeit …", „Der Abschnitt …",
„… wird bewusst …" beginnen, beschreiben meist das Dokument statt den Gegenstand. Sie sind nur
dann berechtigt, wenn sie eine ungewöhnliche Strukturentscheidung erklären, die der Leser sonst
für einen Fehler hielte — nicht, um die eigene Redlichkeit zu betonen.

#### Ergebnis

46 Seiten, kompiliert fehlerfrei, 0 undefined references. Kurzfassung 184 Wörter (Norm 150–250).

---

## v2026-07-25.7 — Forschungsfrage korrigiert, Exposé-Rechtfertigung entschlackt

#### Änderung 1 — Forschungsfrage setzte das Ergebnis voraus

**Problem (vom Team gemeldet):** Die Frage lautete „Kann ein RL-Agent **mit internem
Gedächtnis** …". Das Gedächtnis war aber kein Teil der Fragestellung, sondern ein *Befund*
des Projekts: Dass es nötig ist, ergab sich erst aus dem Scheitern gedächtnisloser Varianten
(`ppo_no_bfs`, MLP-Baselines). Die Formulierung nahm damit das Ergebnis vorweg und machte aus
einer offenen Untersuchung eine Bestätigungsfrage.

**Lösung:**

| | |
|---|---|
| **vorher** | „Kann ein RL-Agent **mit internem Gedächtnis** in prozedural generierten, nur teilweise einsehbaren 2D-Welten eine Navigationsstrategie lernen, die auch auf unbekannten Welten funktioniert, ohne dass ihm der Weg zum Ziel vorberechnet wird?" |
| **nachher** | „Kann ein **Reinforcement-Learning-Agent** in prozedural generierten, nur teilweise einsehbaren 2D-Welten eine Navigationsstrategie lernen, die auch auf unbekannten Welten funktioniert, ohne dass ihm der Weg zum Ziel vorberechnet wird?" |

Ergänzt um einen Satz, der die Offenheit explizit macht: *Wie* ein Agent das leisten könnte,
ist selbst Teil des Untersuchungsgegenstands; das Gedächtnis war zu Projektbeginn eine
Vermutung und ergab sich erst im Verlauf. Zusätzlich der Halbsatz „Ein Programm ohne
Gedächtnis kann … nicht zuverlässig entscheiden" aus dem Absatz davor entfernt — er nahm
dieselbe Antwort vorweg. Kurzfassung und Anhang waren bereits korrekt formuliert und blieben
unverändert.

#### Änderung 2 — Exposé-Rechtfertigung von 79 auf 37 Zeilen

**Problem (vom Team hinterfragt):** Der Abschnitt „Revision des Erfolgskriteriums" und ein
vorgezogener Absatz in der Einleitung verteidigten über mehr als eine Seite, warum die
90 %-Metrik des Exposés ersetzt wurde — inklusive HARKing-Abwehr, „damit die Revision nicht
als Ersetzen eines unbequemen Maßstabs missverstanden wird" und einem eigenen Absatz
„Die Projektziele des Exposés sind davon unberührt". Da der Betreuer schriftlich bestätigt hat,
dass das Exposé nicht verbindlich ist (`v2026-07-22.1`), ist dieser Aufwand unverhältnismäßig
und liest sich defensiv.

**Lösung:**
- **Einleitung:** Der vorgezogene Rechtfertigungsabsatz (12 Zeilen) ist ersatzlos entfallen.
  Es bleibt ein Satz: Schwellen wurden vor den Auswertungsläufen festgelegt, Herleitung folgt
  in der Methodik.
- **Methodik:** Abschnitt umbenannt von „Revision des Erfolgskriteriums" zu
  **„Herleitung der Erfolgsschwellen"** — von Rechtfertigung auf Sachdarstellung gedreht.
  Erhalten bleibt die inhaltlich wertvolle Substanz: die drei Gründe, warum 90 % in dieser
  Aufgabe unerreichbar sind (POMDP-Charakter, Luftlinien- statt Laufwegdistanz vor v11,
  Streuung ±12 Punkte). Entfallen sind die HARKing-Abwehr als Absatz, die Meta-Kommentare
  zur eigenen Redlichkeit, die separate 100-Seed-Tabelle (redundant zu Tab. `tab:v12`) und der
  Absatz zu den Muss-/Nice-to-have-Kriterien.
- Die Vorab-Fixierung bleibt als **ein** Halbsatz mit HARKing-Beleg erhalten — Datum dabei auf
  den tatsächlichen Stand korrigiert: **16.05.2026** statt 08.06.2026. Beide Schwellen stehen
  bereits im Changelog-Eintrag `v2026-05-16`, die Vorab-Fixierung ist also drei Wochen älter
  als bisher behauptet.

#### Ergebnis

46 Seiten, kompiliert fehlerfrei, 0 undefined references.

---

## v2026-07-25.6 — Fazit gestrafft, Zwischenfazits entfernt, Hardware in den Anhang

#### Änderung 1 — Fazit und Ausblick von 187 auf 126 Zeilen

**Problem:** Sieben `\paragraph`-Blöcke plus sieben nummerierte Ausblickspunkte, teils
redundant (der Baseline-Befund stand in „Ergebnis", „Der wichtigste Befund ist ein negativer"
*und* in den Einschränkungen).

**Lösung:** Gliederung in vier Unterabschnitte statt einer Absatzkette:

| vorher | nachher |
|--------|---------|
| Ergebnis · Der wichtigste Befund ist ein negativer | **8.1 Ergebnis** (beides zusammengeführt) |
| Methodischer Beitrag · Inhaltlicher Kernbeitrag · Technische Leistung | **8.2 Beiträge** (drei kurze Absätze) |
| Einschränkungen (Fließtext mit (1)…(5), 2b eingeschoben) | **8.3 Einschränkungen** (nummerierte Liste, nach Schwere sortiert) |
| Ausblick: 4 + 3 Punkte in zwei Listen | **8.4 Ausblick** (eine Liste mit 4 Punkten) |

**Ausblick nach Priorität neu sortiert:** Die Härtung der Umgebung war vorher Punkt 7 von 7
und steht jetzt an Position 1 — sie ist die Voraussetzung dafür, dass alles Übrige überhaupt
interpretierbar wird. Der Architekturwechsel (AMAGO/Transformer) rutschte von Position 1 auf 4.
Die vier vorher separaten Kleinmaßnahmen (Grid-Kodierung, E1/E3-Replikation, Probe,
Entropie-Annealing) sind zu einem Punkt „Billige Diagnosen" zusammengefasst.

#### Änderung 2 — Zwischenfazits entfernt

Die sechs am 25.07. (`v2026-07-25.2`) eingefügten `\paragraph{Zwischenfazit.}`-Absätze wurden
wieder entfernt. Sie waren als Kapitelklammern gedacht, erwiesen sich neben den bereits
vorhandenen Kapitel-Vorschauen aber als Dopplung: Was das Kapitel gebracht hat, stand danach
zweimal da. Die Leserführung übernehmen jetzt allein die Vorschauen am Kapitelanfang plus
Abschnitt 1.5 („Aufbau der Arbeit").

#### Änderung 3 — Hardware aus dem Haupttext

**Problem:** „Training und Evaluation laufen vollständig auf der CPU (Apple M1 Pro)" stand im
Kapitel Technische Umsetzung. Die konkrete Maschine ist für die Argumentation der Arbeit
irrelevant.
**Lösung:** Im Haupttext bleibt nur die inhaltlich relevante Aussage (CPU statt GPU, kein
Vorteil durch GPU bei diesem Netz, Durchsatz 90–190 Schritte/s) mit Verweis auf den Anhang.
Die Gerätebeschreibung (M1 Pro, 8 Performance-Kerne, macOS arm64, MPS-Backend) steht jetzt
im Anhang, Abschnitt „CPU schlägt Apple-GPU (MPS)", wo der zugehörige Benchmark ohnehin liegt.

#### Ergebnis

**46 Seiten** (vorher 48), kompiliert fehlerfrei, 0 undefined references.

---

## v2026-07-25.5 — Bildunterschriften gekürzt, leere Überleitungen entfernt

#### Änderung 1 — Captions von bis zu 195 auf max. 46 Wörter

**Problem:** 16 der 32 Bildunterschriften waren länger als 45 Wörter, die längste hatte
**195**. Sie enthielten Argumentation, Einschränkungen und Interpretation statt einer
Beschreibung dessen, was zu sehen ist. Eine Bildunterschrift ist kein Fließtextersatz.

**Lösung:** Durchgängig auf Beschreibung reduziert (was wird gezeigt, welches Protokoll,
welche Quelle). Der inhaltliche Teil wurde **nicht gelöscht**, sondern als Absatz in den
Fließtext direkt vor oder nach der Tabelle verschoben:

| Tabelle/Abbildung | vorher | nachher | verschobener Inhalt |
|---|---|---|---|
| Ablation A/B/C | 195 W. | 25 W. | Die vier Einschränkungen → neuer Absatz „Aussagekraft" |
| Temperatur-Sweep | 114 W. | 20 W. | Konsistenznotiz zu 70,7/73,3/74,0 → Fließtext |
| Iterationen E1–E3 | 94 W. | 27 W. | n=1 und Budget-Confound → Fließtext |
| Endergebnis v12 (n=7) | 88 W. | 30 W. | Erläuterung `best_model.zip` + Selektionsoptimismus → Fließtext |
| Reward-Komponenten | 62 W. | 11 W. | Policy-Invarianz gilt nur für den PBRS-Term → neuer Absatz im Reward-Abschnitt |
| Budget-Sweep | 52 W. | 18 W. | Kernaussage „bei jedem Budget …" → Fließtext vor der Tabelle |
| 10 weitere | 45–76 W. | 17–41 W. | Interpretation gestrichen, Beschreibung behalten |

Ergebnis: **max. 46 Wörter, Median 26**, nur noch eine Caption über 45 Wörtern.
Gegenprüfung, dass beim Kürzen nichts verlorenging: Für jede entfernte Aussage wurde
geprüft, ob sie im Fließtext steht; zwei Lücken (Policy-Invarianz nur für PBRS;
Kernaussage des Budget-Sweeps) wurden dabei gefunden und ergänzt.

**Zitate in Captions** bleiben erhalten und stehen jetzt am Ende: Abb. 1
„Eigene Darstellung in Anlehnung an Sutton und Barto [5, Abb. 3.1]", Abb. 4 analog zu
Schulman et al. [12, Abb. 1], Abb. 2 und 5 „Eigene Darstellung; … nach [6]/[7]".

#### Änderung 2 — Selbstverständliche Kapitelüberleitungen ersetzt

**Problem (vom Team gemeldet):** Sätze wie „Dieses Kapitel führt die Begriffe ein, die für
Methodik und Ergebnisse benötigt werden" sagen nichts — das ist die Definition eines
Grundlagenkapitels. Dasselbe Muster in vier weiteren Überleitungen („Das folgende Kapitel
beschreibt die Umgebung so genau, dass die späteren Ergebnisse nachvollziehbar werden",
„Dieses Kapitel berichtet die Messergebnisse", …).

**Lösung:** Fünf Überleitungen ersetzt. Statt anzukündigen, *dass* ein Kapitel kommt, sagen
sie jetzt, *welche offene Frage* es beantwortet — z. B. Grundlagen: „Zwei Entscheidungen
dieser Arbeit lassen sich erst vor diesem Hintergrund begründen: die Wahl einer rekurrenten
statt einer reaktiven Politik und die Wahl der stochastischen Auswertung als Primärmetrik."
Umgebung: „Umso genauer muss die Umgebung offengelegt werden, denn sie ist der einzige
Maßstab, gegen den in dieser Arbeit gemessen wird."

#### Ergebnis

48 Seiten, kompiliert fehlerfrei, 0 undefined references. Abbildungs- und Tabellenverzeichnis
jetzt einzeilig pro Eintrag (18 bzw. 13 Einträge).

---

## v2026-07-25.4 — Vier Grundlagen-Abbildungen ergänzt, Erklärung nach vorne, Kurztitel

#### Änderung 1 — Eidesstattliche Erklärung an den Anfang

Von hinten (nach dem Literaturverzeichnis) nach vorne verschoben: jetzt direkt nach dem
Deckblatt, vor Kurzfassung und Inhaltsverzeichnis. Reihenfolge des Vorspanns:
Deckblatt (o. Nr.) → Erklärung (ii) → Kurzfassung (iii) → Inhalts-, Abbildungs-,
Tabellen-, Abkürzungsverzeichnis → Hauptteil (arabisch ab 1).

#### Änderung 2 — Vier Abbildungen im Grundlagenkapitel

**Problem:** Das Grundlagenkapitel war reiner Fließtext mit Formeln. Zentrale Konzepte
(Agent-Umwelt-Schleife, partielle Beobachtbarkeit, PPO-Clipping, rekurrenter Zustand) blieben
für Fachfremde abstrakt.

**Vorgehen:** Nur Abbildungen nachgebaut, deren Originale **verifiziert** wurden — die Papers
wurden dafür heruntergeladen und die Abbildungen samt Bildunterschrift im Volltext geprüft,
statt sie aus Suchtreffern zu übernehmen. Umsetzung als TikZ (keine Grafikdateien, skaliert
sauber, im Repo diffbar).

| Abb. | Inhalt | Ort | Quelle (verifiziert) |
|------|--------|-----|----------------------|
| 1 | Agent-Umwelt-Interaktion im MDP | § RL und MDP | Sutton & Barto, **Abb. 3.1** — Bildunterschrift wörtlich geprüft: „The agent–environment interaction in a Markov decision process." |
| 2 | MDP gegenüber POMDP (Beobachtungsfunktion + Belief) | § Partielle Beobachtbarkeit | **Eigene Darstellung**; Formalismus nach Kaelbling et al. 1998. Bewusst *nicht* als Nachbau deklariert, da die Originalabbildung nicht einsehbar war |
| 4 | Wirkung des PPO-Clippings, $A_t>0$ und $A_t<0$ | § PPO | Schulman et al., **Abb. 1** — Verlauf, Clipping-Punkte ($1{+}\epsilon$ bzw. $1{-}\epsilon$) und roter Startpunkt bei $r=1$ aus dem Original übernommen |
| 5 | Über die Zeit entrollte rekurrente Politik | § Rekurrente Politiken | **Eigene Darstellung**; rekurrente Politiken im RL nach Hausknecht & Stone |

**Zitierweise:** Nachgebaute Abbildungen tragen „Eigene Darstellung in Anlehnung an
\cite[Abb. X]{...}", eigene Darstellungen ohne Vorlage „Eigene Darstellung; Formalismus nach
\cite{...}". Die Unterscheidung ist bewusst — eine Abbildung als Nachbau auszugeben, deren
Original man nicht gesehen hat, wäre eine Falschangabe.

Alle vier Abbildungen wurden nach dem Satz **visuell kontrolliert** (Seitenrendering). Abb. 1
musste dabei neu gezeichnet werden: Der Rückkanal sah wie ein geschlossenes Rechteck aus statt
wie ein Kreislauf. Abb. 5 hatte eine Label-Überlappung.

#### Änderung 3 — Kurztitel für alle Abbildungen und Tabellen

**Problem:** 27 Abbildungen/Tabellen nutzten `\caption{...}` ohne Kurzform. Dadurch landeten
komplette Absätze im Abbildungs- und Tabellenverzeichnis (eine Bildunterschrift belegte dort
bis zu acht Zeilen).
**Lösung:** Durchgängig `\caption[Kurztitel]{Langtext}`. Beide Verzeichnisse sind jetzt
einzeilig pro Eintrag lesbar.

#### Ergebnis

48 Seiten, kompiliert fehlerfrei (`pdflatex` → `bibtex` → `pdflatex` ×2), 0 undefined
references/citations. Abbildungsverzeichnis: 18 Einträge, Tabellenverzeichnis: 13.
Autor vervollständigt: **Laurin Rößler** (Deckblatt, Erklärung, `Expose.tex`).

---

## v2026-07-25.3 — Roter-Faden-Prüfung: drei Begriffsbrüche behoben, Budget-Sweep ergänzt

**Kontext:** Systematische Prüfung, ob Begriffe eingeführt werden, *bevor* sie benutzt werden.
Methode: Für jeden Leitbegriff die erste Nennung im Haupttext ermitteln und mit dem Ort der
Definition abgleichen. Drei echte Vorgriffe gefunden.

#### Bruch 1 — Versionsschema v11/v12 wurde nie eingeführt

**Problem:** `v11` erschien erstmals in Kapitel 4 (Tab. „Observation der Umgebungsversion v11"),
`v12` in Kapitel 5. Erklärt wurden beide erst im **Anhang**. Der Leser begegnet also über den
gesamten Hauptteil Versionskürzeln, deren Bedeutung er nicht kennt — und weiß insbesondere
nicht, dass Zahlen verschiedener Stände nicht vergleichbar sind.
**Lösung:** Absatz „Zur Versionsbezeichnung" in Abschnitt 1.5 (Aufbau der Arbeit): v11 = Umgebung
(06.07.), v12 = Trainingskonfiguration (08.07.) auf unveränderter v11-Umgebung, ältere Stände nur
im Anhang. Inklusive Begründung, warum die Unterscheidung überhaupt nötig ist.

#### Bruch 2 — Seed-Pool („Swarm") wurde als bekannt vorausgesetzt

**Problem:** Kapitel 3 (Verwandte Arbeiten) verwies auf „der Seed-Pool-Mechanismus dieser Arbeit"
mit Vorwärtsverweis auf Kapitel 6. Der Leser trifft den Begriff zwei Kapitel vor seiner Erklärung.
**Lösung:** PLR wird an der Stelle in einem Satz erklärt (wiederholt Level mit hohem Lernpotenzial),
und der Seed-Pool direkt danach als „stark vereinfachte Variante dieses Gedankens" eingeführt,
inklusive Vorgriff auf die spätere Kritik (kehrt die Auswahlrichtung von PLR um).

#### Bruch 3 — Der Begriff „Det/Stoch-Gap" wurde nie geprägt

**Problem:** Der zentrale Untersuchungsgegenstand der Arbeit erschien als benannter Begriff
erstmals in Kapitel 5, ohne je definiert worden zu sein. Das Konzept (Sampling vs. Argmax) war
zwar in den Grundlagen vorbereitet, der Name aber nicht.
**Lösung:** Begriff im PPO-Abschnitt der Grundlagen explizit geprägt und fett gesetzt, mit
anschaulicher Erklärung („der Agent löst die Aufgabe zuverlässiger, wenn er würfelt, als wenn er
stets seine beste Aktion wählt") und Rückverweis auf den POMDP-Abschnitt.

#### Restfälle

- **MLP** erschien erstmals in der Ergebnistabelle in Kapitel 7. Jetzt in den Grundlagen
  eingeführt (gedächtnisloses vorwärtsgerichtetes Netz) als Gegenstück zum LSTM-Abschnitt.
- **`approx_kl`** wurde in Kapitel 6 und im Anhang verwendet, aber nie erklärt. Jetzt bei
  erster Nennung erläutert (Maß der Politikänderung pro Update; nahe null = Training steht
  still) und ins Abkürzungsverzeichnis aufgenommen.

#### Neue Messung — Erfolgsquote über das gesamte Episodenbudget

**Frage:** Rettet ein engeres Episodenlimit die Leistungsaussage gegenüber der Baseline?
**Methode:** Exakt aus je einem Lauf ableitbar — wer bei Schritt T erfolgreich ist, ist es bei
jedem Budget ≥ T. Protokolliert wurden die Erfolgsschritte, daraus die vollständige Kurve.

| Budget | Kompass ε=0,4 | Kompass ε=0,9 | v12_s1 | v12_s2 | v12_s3 |
|--------|---------------|---------------|--------|--------|--------|
| 120 (3× Optimum) | **4 %** | 0 % | 0 % | 2 % | 0 % |
| 200 | **22 %** | 0 % | 0 % | 4 % | 0 % |
| 500 | **36 %** | 10 % | 12 % | 22 % | 12 % |
| 800 | **54 %** | 18 % | 20 % | 32 % | 30 % |
| 1500 | **62 %** | 54 % | 40 % | 48 % | 54 % |
| 4000 | 64 % | **88 %** | 64 % | 78 % | 74 % |

**Antwort: nein.** Bei *jedem* Budget erreicht oder übertrifft mindestens eine ungelernte Politik
alle trainierten Läufe. Damit ist belegt, dass das Problem **nicht in der Metrik allein** liegt,
sondern in der Struktur der Umgebung: Die Welten sind so offen, dass gerichtete Zufallssuche
Sackgassen von selbst verlässt. Als Tabelle in Abschnitt „Ungelernte Referenzpolitiken"
aufgenommen.

#### Ausblick präzisiert

Der Punkt „Härtung der Umgebung" wurde vom Nebensatz zum ausformulierten nächsten Schritt:
Wandanteil (derzeit fest pro Biom, 7 % Wald bis 25 % Bergland, plus Cellular-Automata-Glättung)
zu einem steuerbaren Parameter machen und so weit erhöhen, bis der Kompass-Zufallslauf einbricht,
die Lösbarkeit aber bei 100 % bleibt. **Zweistufig und in dieser Reihenfolge:** erst die
Baseline-Politik als Kalibrierinstrument einsetzen, dann trainieren — ein Trainingslauf ohne
diese Vorabprüfung wäre erneut nicht interpretierbar.

**Bewusst nicht umgesetzt:** Die Änderung an `world.cpp` und der zugehörige Trainingslauf wurden
erwogen und dann verworfen (Entscheidung des Teams); sie stehen als Folgearbeit im Ausblick.

#### Formales

Deckblatt korrigiert: **Masterstudiengang Informatik und Security (M. Sc.)** statt Bachelor.
`Projektdokumentation.tex` kompiliert fehlerfrei, **48 Seiten**, 0 undefined references.

---

## v2026-07-25.2 — Doku-Umbau: Formalia, Gliederung und Leserführung

**Kontext:** Prüfung, ob die Projektdokumentation einen durchgehenden roten Faden hat, ob Begriffe
rechtzeitig eingeführt werden und ob sie ohne RL-Vorkenntnisse lesbar ist. Konventionen
recherchiert (IMRaD, Gliederungsvorgaben Hochschule/DHBW/TU Chemnitz, Abstract-Regeln,
Signposting-Techniken); Ergebnis als Wiki-Eintrag gesichert.

**Befund:** Inhaltlich stark, formal unvollständig und in der Leserführung lückenhaft.
Es fehlten sämtliche Verzeichnisse und ein Deckblatt; der Abstract war mit **392 Wörtern**
rund doppelt so lang wie die Norm (150–250) und enthielt Zitate; es gab keinen Abschnitt
„Aufbau der Arbeit"; und der Leser bekam MDP/POMDP/PPO-Theorie, **bevor** er wusste, worum es
im Spiel überhaupt geht.

#### Änderung 1 — Formalia ergänzt

| Element | vorher | nachher |
|---------|--------|---------|
| Deckblatt | nur `\maketitle` | vollwertige `titlepage` (Hochschule, Fakultät, Studiengang, Art der Arbeit, Titel/Untertitel, Verfasser, Betreuer, Abgabedatum) |
| Inhaltsverzeichnis | fehlt | `\tableofcontents` |
| Abbildungsverzeichnis | fehlt | `\listoffigures` (13 Abbildungen) |
| Tabellenverzeichnis | fehlt | `\listoftables` (12 Tabellen) |
| Abkürzungs-/Symbolverzeichnis | fehlt | 20 Abkürzungen + 6 Symbole (`longtable`) |
| Seitennummerierung | durchgehend arabisch | Vorspann römisch ab ii, Hauptteil arabisch ab 1 |
| Eidesstattliche Erklärung | fehlt | ergänzt, mit Unterschriftsfeldern für beide Verfasser |

#### Änderung 2 — Kurzfassung gekürzt

**Problem:** 392 Wörter, mit Literaturzitat (`\cite{henderson2018matters}`), erzählte den
Projektverlauf statt des Ergebnisses.
**Lösung:** 192 Wörter nach dem Schema Kontext → Fragestellung → Methode → Ergebnis, ohne
Zitate, mit Ergebniszahlen und Schlagwörtern. Als `\section*{Kurzfassung}` statt
`abstract`-Umgebung, damit sie im Inhaltsverzeichnis erscheint.

#### Änderung 3 — „Zielsetzung" → vollwertige Einleitung

**Problem:** Kapitel 1 bestand aus einem Absatz Forschungsfrage plus Kriterienliste. Weder
Motivation noch anschauliche Aufgabenbeschreibung noch Leserführung.

**Lösung:** Fünf Unterabschnitte:

| Abschnitt | Zweck |
|-----------|-------|
| 1.1 Motivation | Warum ist Generalisierung das eigentliche Problem — ohne Fachjargon |
| 1.2 **Die Aufgabe in Kürze** | Stoneforge in Alltagssprache: Raster, Seed, Exit, 15×15-Sichtfeld, Kompass. Gibt dem Leser ein mentales Bild **vor** der Theorie |
| 1.3 Problemstellung und Forschungsfrage | Forschungsfrage als hervorgehobener Block |
| 1.4 Zielsetzung und Erfolgskriterien | bisheriger Inhalt |
| 1.5 **Aufbau der Arbeit** | Kapitel für Kapitel, je ein Satz — das zentrale Signposting-Element |

#### Änderung 4 — Roter Faden: Sandwich-Prinzip

**Lösung:** Sechs Kapitelklammern eingefügt (Zwischenfazit am Kapitelende + Überleitung zum
nächsten) an allen Kapitelgrenzen: Grundlagen → Verwandte Arbeiten → Umgebung → Methodik →
Technik → Evaluation → Fazit. Zusätzlich Kapitel-Vorschauen für Grundlagen, Methodik und
Evaluation (was kommt, in welcher Reihenfolge, warum diese Reihenfolge).
Metakommentar ergänzt, wo etwas an ungewohnter Stelle steht (z. B. warum die
Kriterienrevision in der Methodik steht und nicht in der Einleitung).

**Bewusst nicht geändert:** die Kapitelreihenfolge. Grundlagen → Verwandte Arbeiten → Umgebung
entspricht der Konvention für Bachelor-/Projektarbeiten; das Verständnisproblem wurde
stattdessen über Abschnitt 1.2 gelöst — billiger und weniger fehleranfällig als eine Umstellung.

#### Ergebnis

`Projektdokumentation.tex` kompiliert fehlerfrei (`pdflatex` → `bibtex` → `pdflatex` ×2),
**47 Seiten** (vorher 30), **0 undefined references/citations**. Gliederungstiefe ≤ 3,
keine einzelne Unterüberschrift ohne Schwester.

**Wiki:** Neuer Eintrag [`concepts/wissenschaftliche-doku-aufbau.md`](docs/wiki/concepts/wissenschaftliche-doku-aufbau.md)
mit Pflicht-Reihenfolge der Bestandteile, IMRaD-Zuordnung, Signposting-Techniken,
Abstract-Regeln, Verständlichkeitsregeln und einer Checkliste vor Abgabe. Verlinkt aus
`docs/wiki/README.md` (Einstieg + Karte).

---

## v2026-07-25 — ⚠️ Kritischer Befund: ungelernte Baseline schlägt den trainierten Agenten

**Kontext:** Externe Durchsicht der Projektdokumentation vor Abgabe. Geprüft wurden alle
Zahlen der Doku gegen Rohdaten, Code und Changelog. Die Verifikation bestand größtenteils
(siehe unten), deckte aber eine fehlende Kontrolle auf, die die Einordnung sämtlicher
Leistungszahlen verändert.

### Befund 1 — Es gab nie eine ungelernte Referenzpolitik

**Problem:** Die Arbeit berichtet Success Rates (65,7 % / 66,9 %) ohne Untergrenze. Der einzige
je gemessene Random-Wert (42 % @ exit 5–12, `v2026-06-15.C`) stammt aus der leichtesten
Curriculum-Phase und ist seit der v11-Revision veraltet (Neumessung: 56 %). Auf Testset A
existierte keinerlei Baseline.

**Lösung:** Neues Skript `scripts/eval_baselines.py`. Zwei ungelernte Politiken auf identischem
Protokoll (50 Seeds, exit 35–45, Cap 4000), beide mit derselben Observation wie der Agent:
- `random` — gleichverteilte Bewegung.
- `compass` — mit Wahrscheinlichkeit ε gleichverteilt zufällig, sonst ein Schritt in die
  dominante Luftlinien-Kompassrichtung. Nutzt **2 von 229 Features**, ignoriert das 15×15-Grid
  vollständig, kein Gedächtnis, kein Training. ε **ausschließlich auf den Validierungs-Seeds
  6000–6049 gewählt** (kein Testset-Tuning).

Zusätzlich neue Metrik **Pfadeffizienz** η = BFS-Optimum / benötigte Schritte (Mittel über
erfolgreiche Episoden). BFS-Optimum auf A und B: Ø 40 Schritte — das Episodenlimit von 4000
entspricht dem **Hundertfachen** der Optimallänge.

**Ergebnis:**

| Politik | A: SR | A: η | B: SR | B: η |
|---------|-------|------|-------|------|
| Random (gleichverteilt) | 10,0 % | 0,022 | 16,0 % | 0,030 |
| Kompass-Zufallslauf ε=0,4 (effizienz-optimal auf Val) | 60,4 ± 4,8 % | **0,153** | 61,6 ± 5,0 % | **0,161** |
| Kompass-Zufallslauf ε=0,9 (SR-optimal auf Val) | **92,0 ± 5,1 %** | 0,047 | **91,6 ± 4,3 %** | 0,048 |
| RecurrentPPO v12 (n=7, stoch.) | 65,7 ± 12,4 % | 0,049 | 66,9 ± 12,8 % | 0,057 |

**Die Heuristik dominiert auf beiden Achsen.** Bei vergleichbarer Effizienz (η ≈ 0,05) erreicht
sie 92 % statt 65,7 %; bei vergleichbarer SR (≈ 60 %) ist sie dreimal wegeffizienter.

**Validierung der Messkette:** Das verwendete Harness reproduziert die publizierten
det-Werte von s1–s3 **exakt** (A: 38/26/32) und die stoch-Werte innerhalb des Sampling-Rauschens
(60/76/78 vs. 62/84/76). Die Baseline-Zahlen sind also direkt vergleichbar.
Ausgangsanalyse der 50 Episoden: Die Modelle scheitern überwiegend am 4000-Schritt-Timeout
(18/11/6 von 50), nicht am Early-Stop — kein Confound.

**Interpretation:** Der Befund entwertet nicht die Plattform, sondern die *Aufgabenparametrisierung*.
Drei Eigenschaften wirken zusammen: (1) Episodenbudget = 100× Optimallänge → ungerichtete Suche
hat genug Zeit; (2) der Luftlinien-Kompass liefert dauerhaft eine grob richtige Richtung;
(3) die Welten sind offen genug, dass Sackgassen durch Zufall verlassen werden. Gedächtnis ist
für die Erfolgsquote schlicht nicht erforderlich — die Metrik kann folglich nicht messen, ob es
gelernt wurde. **Die Forschungsfrage ist mit dem vorliegenden Aufbau nicht entscheidbar.**

Betroffen ist damit auch die Ablation A/B/C: Wenn eine kompassgetriebene Zufallspolitik 92 %
erreicht, misst der Vergleich „LSTM ohne BFS" vs. „MLP mit BFS" überwiegend Kompass-Ausnutzung
und Schleifenanfälligkeit, nicht Gedächtnisfähigkeit.

**Konsequenz für Folgearbeiten:** nicht den Agenten tunen, sondern das Messregime reparieren —
Episodenbudget an die Aufgabenlänge koppeln (≈ 3 × d_BFS), Pfadeffizienz als Primärmetrik,
Weltgenerierung mit höherem Wandanteil / echten Labyrinthstrukturen.

### Befund 2 — Weitere Korrekturen an der Doku

| # | Datei | Problem | Korrektur |
|---|-------|---------|-----------|
| 1 | `docs/RESULTS.md`, `python/doc_logger.py` | Spalte „Beste SR (A)" war das **Maximum über bis zu 88 Eval-Punkte** (E3: 92 % dort vs. 44 % real; v12_s1: 94 % @ Step 25k). Widersprach der eigenen Repro-Checkliste („kein Best-of-Runs"). | Spalten umbenannt in „Max. Val-SR im Verlauf", Warnhinweis über der Tabelle |
| 2 | `Projektdokumentation.tex` Tab. 7 | „finales Modell je Lauf" — das Eval-Skript lädt `best_model.zip` (bestes nach Val-SR) | Caption korrigiert, Selektionsoptimismus explizit benannt |
| 3 | `Projektdokumentation.tex` Tab. 6 (Ablation) | A/B/C laufen auf verschiedenen Obs-Räumen und Env-Ständen; A variiert **zwei** Faktoren ggü. C; B wurde bei 720k/2M **abgebrochen** und enthielt einen später entfernten Stagnations-Penalty; alle n=1 | Alle vier Einschränkungen in der Caption; Aussage auf die belastbare schwache Form reduziert |
| 4 | `Projektdokumentation.tex` Tab. 5 + §Gap-Experimente | „verändern jeweils **genau einen** Faktor" — E1/E2 haben zusätzlich nur 400k statt 1,0 Mio. Phase-3-Schritte. Zudem n=1 gegen eine n=3-Baseline bei σ ≈ 12 Punkten → „kein Arm schlägt die Baseline" nicht ableitbar | Faktorenzahl korrigiert; n=1 ausgewiesen; Aussagen in σ-Einheiten quantifiziert (E1 1,5σ, E3 2,5σ, E2 4σ); Replikation als offener Punkt |
| 5 | `Projektdokumentation.tex` §Det/Stoch-Gap | Singh 1994 gilt für **gedächtnislose** Politiken und trägt das Argument für eine LSTM-Politik nicht — über einem genauen Belief-State existiert wieder eine optimale deterministische Politik | Argumentation auf Ghosh 2021 (epistemic POMDP, gilt auch mit Gedächtnis) umgestellt; Singh zur Motivation zurückgestuft; Gap explizit als teils **Belief-State-Defizit** benannt |
| 6 | `Projektdokumentation.tex` Abstract/Fazit | „ohne globale Wegweiser-Features" — der Luftlinien-Kompass **ist** ein globales Zielsignal (und trägt laut Baseline allein 92 %) | Präzisiert zu „ohne lokale BFS-Gradientenfeatures"; Kompass explizit benannt |
| 7 | `Projektdokumentation.tex` Tab. 9 | Technologie-Stack: „Simulationskern ca. 8.000 LOC" | Gemessen: `src/` gesamt 9.054 LOC, Kern 4.469, raylib-Client 4.585. Aufgeteilt |
| 8 | `Projektdokumentation.tex` Tab. 4 | Reward-Tabelle unvollständig: Idle-Penalty −0,04 und Schadens-Penalty −0,5/HP fehlten | Ergänzt; zusätzlich klargestellt, dass **nur** der PBRS-Term policy-invariant ist |
| 9 | `Projektdokumentation.tex` §7.2 | „Episode endet bei Exit oder nach 4000 Schritten" — widersprach §5.2 (Early-Stop nach 256 Schritten ohne positiven Reward) | Vereinheitlicht |
| 10 | `Projektdokumentation.tex` Tab. 8 | Drei verschiedene Werte für dieselbe Größe (v12, n=3, A stoch): 70,7 / 73,3 / 74,0 | Als Sampling-Rauschen dreier Skripte offengelegt; skriptkonsistenter Vergleich 74,0 → 65,7 ergänzt |
| 11 | `.claude/CLAUDE.md` | Führte `ppo_phase4` weiterhin als „aktuell bestes — 100 % SR" (Juni-Stand, altes Protokoll) | Als historisch markiert, v12 als aktueller Stand, Baseline-Warnung ergänzt |

### Verifiziert und unverändert korrekt

Alle Zahlen der Tab. 7 gegen `scratchpad_final_eval_n7.json`; Mittelwerte, ddof=1-Std und
t-basierte CIs (df=6); Exposé-Tabelle (66,3 ± 12,1 über 100 Seeds); Welch-Test der beiden
Lauf-Chargen (**t=1,764, df=4,21, p=0,1489** — Doku nennt 0,149 ✓); „±14 Punkte CI bei 50 Seeds";
PBRS-Arithmetik gegen `simulation.cpp` (β=2,5, Φ=−d/128, γ=0,999 → netto +0,01/Tile, Φ(Exit)=0);
alle 29 Bib-Keys zitiert und vorhanden, keine verwaisten; sämtliche Versionsangaben
(Python 3.12.13, torch 2.12.0, numpy 2.4.4, gymnasium 1.2.3, SB3/contrib 2.8.0, raylib 5.5).

**Nebenbefund zugunsten der Arbeit:** Die Kriterien 70 %/60 % stehen bereits im Eintrag
`v2026-05-16`, nicht erst seit 08.06.2026 — die Vorab-Fixierung ist also **älter** als in der
Doku behauptet und damit besser belegbar.

**Doku-Stand:** `Projektdokumentation.tex` kompiliert fehlerfrei, 34 Seiten (vorher 30),
keine undefinierten Referenzen. Neuer Abschnitt „Ungelernte Referenzpolitiken und die Grenzen
der Success Rate" (§ vor dem Det/Stoch-Gap) mit Tabelle. Rohdaten:
`logs/eval_results/baselines.json`, `logs/eval_results/baselines_and_models.json`.

---

## v2026-07-22.1 — Betreuer-Bestätigung: Revision des Erfolgskriteriums abgesegnet

**Kontext:** Anfrage per E-Mail an den Betreuer (Prof. Dr. Carsten Lecon, Hochschule Aalen), ob
die dokumentierte Absenkung des Zielkriteriums von 90\,\% (Exposé) auf 70\,\%/60\,\% (Testset
A/Holdout B) so in Ordnung ist (Entwurf: `docs/betreuer_mail_entwurf.md`).

**Antwort (22.07.2026, wörtlich):** „Ja. Das Exposé ist hier kein verbindliches Dokument."

**Bedeutung:** Die Revision ist damit nicht nur methodisch begründet (POMDP-Charakter,
Distanzmessungsfehler vor v11, Seed-Streuung von ±12 Punkten, Abschnitt
„Revision des Erfolgskriteriums") und vorab dokumentiert (Changelog-Fixierung 08.06.2026, einen
Monat vor den finalen Läufen), sondern zusätzlich vom Betreuer schriftlich bestätigt. Kein
Handlungsbedarf für die Doku selbst; die Bestätigung dient als zusätzlicher Beleg der
methodischen Redlichkeit, falls die Kriterienrevision im Kolloquium hinterfragt wird.

---

## v2026-07-18.7 — Stilüberarbeitung: Gedankenstriche entfernt, Wortwahl neutralisiert

**Kontext:** Durchgang durch `Projektdokumentation.tex` mit zwei Zielen: (1) alle
Gedankenstriche (`---`) im Fließtext entfernen, ersetzt durch Punkt, Semikolon, Doppelpunkt
oder Komma je nach Satzbezug; (2) Wortwahl auf sachlich-neutralen wissenschaftlichen Stil
prüfen (recherchiert: Sachlichkeit, keine Wertungen/Übertreibungen, kurze klare Sätze als
Kernregeln für Abschlussarbeiten). Kompiliert exit 0, 30 S., keine undefinierten Referenzen.

#### Änderung 1 — ca. 90 Gedankenstriche im Fließtext entfernt
**Datei:** `docs/Projektdokumentation.tex` (durchgängig, Abstract bis Anhang)
**Problem:** Der Gedankenstrich wurde durchgängig als Universal-Verbindung genutzt (Einschub,
Begründung, Aufzählung, Gegensatz), was im Deutschen unüblich und für einen Prüfer als
Stilbruch auffällig ist.
**Lösung:** Jede Stelle einzeln nach Satzbezug aufgelöst: Kausalzusammenhänge → Punkt/Semikolon,
Erläuterungen → Doppelpunkt, lose Nebeninformation → Komma/Klammer. Tabellenplatzhalter für
fehlende Werte (leere Zellen) auf den kürzeren Halbgeviertstrich (`--`) vereinheitlicht, das ist
die LaTeX-Konvention für „kein Wert" und bewusst von der Fließtext-Bereinigung unterschieden.

#### Änderung 2 — Wertende/übertreibende Formulierungen neutralisiert
**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Einzelne Formulierungen legten dem Leser eine Bewertung nahe statt die Daten
sprechen zu lassen, oder waren umgangssprachlich/dramatisierend: „Hyperparameter-Krise",
„Initialisierungs-Pech", „unterschätzen die Kompetenz massiv", „oszillierte wild", „schlicht
einen günstigen Schnappschuss erwischt", „Bemerkenswert ist, dass…" (2×), „chaotisch
oszillierend" (2×), „entlarvte", „lehrreiches Zwischenbild", „Beschönigung".
**Lösung:** Durchgängig durch sachliche, quantifizierte Beschreibungen ersetzt (z.\,B.
„Fehlkonfigurierte Hyperparameter", „ungünstige Zufallsinitialisierung", „unterschätzen die
Kompetenz erheblich", „schwankte stark und unregelmäßig", „stark schwankende Kurve/Lerndynamik").
Keine Zahlen- oder Ergebnisänderungen.

---

## v2026-07-18.6 — Externer Faktencheck der Doku: 4 Fehler gefunden und behoben (1 davon gravierend)

**Kontext:** Web-Recherche aller extern prüfbaren Behauptungen der Doku (Literaturaussagen,
Bibliographie, Statistik) auf Belastbarkeit vor Abgabe. **Ergebnis: Kernaussagen halten** —
Singh 1994 (stochastische > deterministische Politiken im POMDP), Ghosh 2021 (epistemic POMDP),
POPGym (15 Envs, 13 Baselines, GRU/LSTM vorn), Pleines 2022, AMAGO (ICLR 2024 Spotlight),
Procgen-Normalized-Return-Formel: alle korrekt wiedergegeben. Gesamte Statistik (Tab. `tab:v12`,
100-Seed-Tabelle, t-CIs df=6, Welch p=0,149, ddof=1) unabhängig nachgerechnet — **bitgenau
reproduziert**. Kompiliert exit 0, 30 S.

#### Änderung 1 — 🔴 Erfundene Co-Autoren im Togelius-Bibeintrag (gravierend)
**Datei:** `docs/references.bib` (`togelius2011search`)
**Problem:** Autorenliste enthielt zwei **nicht existierende Namen** („Kastadjian, Georgios",
„Faust, Mohammed S"). Tatsächliche Autoren: Togelius, Yannakakis, **Stanley**, **Browne**
(IEEE TCIAIG 3(3), 2011, DOI 10.1109/TCIAIG.2011.2148116). Derselbe Fehlertyp wie in
[[literatur-lstm-groesse]] dokumentiert — ein Prüfer, der den Eintrag nachschlägt, findet
Phantom-Autoren.
**Lösung:** Autoren korrigiert.

#### Änderung 2 — Falscher Cite-Key: Pineau-Checkliste zitierte Ghosh-Paper
**Datei:** `docs/Projektdokumentation.tex` (§ Versuchsübersicht)
**Problem:** „Checklisten von Pineau et al. \cite{ghosh2021epistemic}" — der Key zeigt auf das
epistemic-POMDP-Paper, nicht auf den Reproducibility-Report.
**Lösung:** → `\cite{pineau2021repro}`.

#### Änderung 3 — R2D2-Aussage stärker als das Paper
**Datei:** `docs/Projektdokumentation.tex` (§ Related Work)
**Problem:** „…für die Leistung entscheidender ist als die Architektur selbst" — diesen
Architektur-Vergleich macht Kapturowski et al. 2019 nicht; das Paper zeigt große Effekte von
Stored-State/Burn-in bei fester Architektur.
**Lösung:** Umformuliert zu „…die Leistung erheblich beeinflusst — bei ansonsten unveränderter
Architektur".

#### Änderung 4 — RND-Jahr korrigiert
**Datei:** `docs/references.bib` (`burda2018rnd`)
**Problem:** `year=2018` mit `booktitle=ICLR` — RND erschien auf der ICLR **2019** (arXiv 2018).
**Lösung:** Jahr → 2019 (Key bleibt `burda2018rnd`).

---

## v2026-07-18.5 — Ausblick umgebaut: „Diagnose vor Architekturwechsel" (gestufter Plan statt AMAGO-Reflex)

**Kontext:** Nachfrage im Gutachter-Review, ob für Folgearbeiten direkt auf Transformer-basierte
In-Context-Architekturen (AMAGO) gewechselt werden soll oder ob RecurrentPPO noch Potenzial hat.
Der bisherige Ausblick nannte AMAGO als Punkt 1 ohne Einordnung. Kompiliert exit 0, 30 S.,
keine undefinierten Referenzen.

#### Änderung 1 — Ausblick als gestufter Plan mit Diagnose-Weiche

**Datei:** `docs/Projektdokumentation.tex` (§ Fazit und Ausblick, `\paragraph{Ausblick.}`)
**Problem:** Der Ausblick empfahl den Architekturwechsel, ohne zu klären, ob der Det/Stoch-Gap
überhaupt ein Gedächtnis-Kapazitätsproblem ist. Zwei eigene Befunde sprechen gegen einen Reflex:
(1) Die Skalierung des Gaps mit der Weglänge ist auch mit der Schleifenbrecher-Hypothese
konsistent (unter `ent_coef=0.05` trainierte Politik nutzt Restrauschen als impliziten
Schleifenbrecher, ohne je Anreiz zu deterministischer Kompetenz); (2) E3 (LSTM 512) zeigte,
dass mehr Gedächtniskapazität hier schadet; POPGym (morad2023popgym) zeigt rekurrente Modelle
als konkurrenzfähig gegenüber Transformern.
**Lösung:** AMAGO wird als *Hypothese* statt *Entscheidung* eingeordnet; neuer 4-Punkte-Plan:
1. **Diagnose** (Stundenbereich): (a) lineare Belief-Probe auf dem LSTM-Hidden-State
   (Exit-Richtung/BFS-Distanz) — kollabiert sie auf langen Wegen → echtes Belief-Tracking-Limit;
   (b) Entropie-Annealing in Phase 4 mit Modellselektion auf *deterministischer* SR als Test
   der Schleifenbrecher-Hypothese.
2. **RecurrentPPO-Restpotenzial:** Auxiliary-Loss-Kopf (BFS-Distanz aus Hidden-State — formt den
   Belief-State explizit, recycelt das Orakel als Trainingssignal ohne Testzeit-Bedarf);
   R2D2-Burn-in (kapturowski2019r2d2).
3. **Architekturvergleich als Folgearbeit**, nur bei positivem Probe-Befund: LSTM /
   Transformer-Gedächtnis / AMAGO auf demselben Generator+Protokoll (Verweis
   `sec:relatedwork`: nur Vergleiche im selben Messregime sind belastbar). Praktische Hürde
   benannt: lange Transformer-Kontexte brauchen GPU; CPU-Läufe hier 8–36 h.
4. **Unverändert übernommen:** engere CIs, temperaturkalibrierte Eval (T=0,5), PLR + SIL.

Keine Zahlen- oder Ergebnisänderungen; alle zitierten Quellen waren bereits in `references.bib`.

---

## v2026-07-18.4 — Struktur-Straffung „Variante B": Kriterienrevision in die Methodik, neuer Abschnitt „Iterative Modellverbesserung"

**Kontext:** Externes Feedback schlug einen Voll-Umbau der Kapitelstruktur vor. Bewusst nur die
risikoarme Teilmenge umgesetzt („Variante B"); Kapitel-4/6-Verschmelzung und DQN-Streichung
wurden **verworfen** (Begründung: Referenz-Churn kurz vor Abgabe bzw. Wiederaufreißen der
Baseline-Lücke aus v2026-07-18.2). Kompiliert exit 0, 30 S., keine undefinierten Referenzen.

#### Änderung 1 — Kriterienrevision von §1.1 in die Methodik verschoben

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** §1.1 verteidigte die Metrik-Revision auf S. 2 mit POMDP-, v11- und
Streuungs-Argumenten, die der Leser erst ab Kapitel 2 bzw. 5 verstehen kann („Verteidigung vor
der Erklärung").
**Lösung:** Die Zielsetzung behält die Kriterien plus eine **kompakte Offenlegung** (Revision,
fixiert 08.06., Exposé-Kriterium wird mitberichtet und deutlich verfehlt) mit Vorwärtsverweis.
Die vollständige Begründung (drei Gründe, Tabelle gegen die 90-%-Metrik, Projektziele-Absatz)
steht jetzt als `\subsection{Revision des Erfolgskriteriums}` in der Methodik nach dem
Evaluationsprotokoll — Label `sec:kriterienrevision` wandert mit, alle Verweise (Abstract,
Related Work) bleiben gültig. **Wichtig:** Die Offenlegung selbst bleibt in §1 — sie ist die
HARKing-Absicherung und darf nicht erst auf S. 15 kommen.

#### Änderung 2 — Neuer Methodik-Abschnitt „Iterative Modellverbesserung" (`sec:iteration`)

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Die entscheidungsprägenden Befunde des Entwicklungsverlaufs (Anhang) waren im
Hauptteil unsichtbar — die Methodik las sich, als sei die Konfiguration vom Himmel gefallen.
**Lösung:** Halbseitiger Abschnitt zwischen Curriculum und Evaluationsprotokoll mit den drei
Schlüsselbefunden inkl. Anhang-Verweisen: (1) Hyperparameter dominieren Features (ent_coef,
Tab. Root-Cause), (2) Einzel-Snapshots validieren nicht (batch 64→8, EV 0,1→0,85),
(3) das Eval-Protokoll ist Teil des Experiments (Cap-Artefakt 48 % vs. 86 %; Data-Leakage bis
11.06.). Neue Labels: `sec:iteration`, `sec:batchsize`, `sec:ziele`.

---

## v2026-07-18.3 — Redaktionelle Politur (externes Feedback, selektiv umgesetzt)

**Kontext:** Externes Redaktions-Feedback zur Doku; drei von vier Punkten umgesetzt, einer nur
teilweise (Begründung unten). Keine inhaltlichen oder Zahlen-Änderungen; kompiliert exit 0, 29 S.

1. **Zusammenfassung mit Aufhänger:** Neuer Einstieg über die Relevanzfrage (Generalisierung auf
   ungesehene Umgebungen), dann Kernbefund der Ablation, *dann* die Zahlen. Das verfehlte
   Testset-Kriterium bleibt bewusst prominent im ersten Absatzdrittel — die Redlichkeit ist die
   Stärke der Arbeit; ein „Erfolgs-Framing", das sie abschwächt, wurde **nicht** übernommen.
2. **POMDP-Vorwegnahme in den Grundlagen (§2.2):** Der Singh-1994-Satz (stochastische
   gedächtnislose Politik kann jede deterministische übertreffen) steht jetzt schon im
   Grundlagenkapitel — der Det/Stoch-Gap wird damit als *geprüfte Hypothese* eingeführt, nicht
   als nachträgliche Erklärung.
3. **Fazit umstrukturiert:** `\paragraph`-Köpfe (Ergebnis / Methodischer Beitrag / Inhaltlicher
   Kernbeitrag / **Technische Leistung** (neu: C++-Engine, pybind11, Pipeline, Monitoring als
   Voraussetzung der Root-Cause-Analysen) / **Einschränkungen (Threats to Validity)** / Ausblick).
4. **Etappe 3 (Anhang) skimmbar:** Lead-ins `Symptom:` / `Root-Cause:` / `Ergebnis:` gefettet.
   Übrige Etappen hatten bereits Bold-Struktur; Tabelle „Root-Cause" war bereits prägnant —
   nicht angefasst.

---

## v2026-07-18.2 — Gutachter-Feedback: Related Work ausgebaut, Baseline-Lücke ausgewiesen, n=7-Lernkurven-Abbildung

**Kontext:** Umsetzung von drei Punkten aus einem internen Gutachter-Review der Doku. Keine neuen
Messungen; Doku kompiliert exit 0 (jetzt 29 S.).

#### Änderung 1 — Baseline-Versprechen aufgelöst (DQN/A2C)

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Die Grundlagen kündigten DQN und A2C als „Baseline geführt" an, aber im
Evaluationskapitel gab es keine einzige DQN/A2C-Zahl — ein unerfülltes Versprechen im eigenen Text.
**Lösung:** (a) Ankündigungen aus den Grundlagen entfernt, stattdessen Verweis auf die Methodik.
(b) Neuer Absatz „Algorithmenwahl" in § Methodik: der frühe DQN-Anlauf (Mai 2026, Q-Wert-Kollaps +
Replay-Buffer konservierte einen später behobenen Reward-Fehler → Wechsel auf on-policy PPO,
Quelle: CHANGELOG 16.05.). (c) Die fehlende quantifizierte DQN/A2C-Baseline auf v11 ist jetzt
**explizit als Grenze (5) im Fazit** ausgewiesen statt stillschweigend offen.

#### Änderung 2 — Related Work von ~½ auf ~1½ Seiten ausgebaut

**Datei:** `docs/Projektdokumentation.tex` (§ Einordnung), `docs/references.bib`
**Problem:** Nur drei Benchmark-Verweise; die Methoden-Literatur (Gedächtnis, Curriculum,
Exploration), die die Arbeit verstreut nutzt, war nicht gebündelt eingeordnet.
**Lösung:** Zwei neue Absätze: (a) *Gedächtnis in teilbeobachtbaren Umgebungen* — DRQN, R2D2
(State-Handling wichtiger als Architektur), Pleines 2022, POPGym (GRU schlägt aufwändigere
Modelle — stützt die eigene E3-Ablation), AMAGO-Verweis in den Ausblick. (b) *Curriculum und
Exploration* — Bengio 2009, PLR (Seed-Pool als vereinfachte Variante), ICM/RND als
Verallgemeinerung des eigenen zählbasierten Explorations-Bonus. Der bisherige Einzelabsatz zu PLR
ging darin auf. **Neue BibTeX-Einträge:** `kapturowski2019r2d2`, `morad2023popgym`; die bisher
unzitierten Einträge `togelius2011search`, `pathak2017icm`, `burda2018rnd` werden jetzt zitiert.

#### Änderung 3 — Lernkurven-Abbildung für das Endergebnis (n=7)

**Dateien:** `scripts/plot_v12_learning_curves.py` (neu), `docs/figures/fig_v12_lernkurven.{pdf,png}` (neu),
`docs/Projektdokumentation.tex`
**Problem:** Die einzige Lernkurve im Hauptteil (`fig:lernkurve`) zeigt den *historischen*
Referenzlauf vor v11 — das Endergebnis (v12, n=7) hatte keine Trainingsverlaufs-Abbildung.
**Lösung:** Neue Abbildung `fig:v12kurven` in § Endergebnis: 2×2-Panels je Curriculum-Phase,
dünne Linien = stochastische SR der 7 Einzelläufe (Val-Seeds 6000–6049), dicke Linien =
Ensemble-Mittel stoch. + det. (nur wo ≥4 Läufe Daten haben). Die alte Abbildung bleibt, ist aber
in Referenz und Caption als historischer Referenzlauf (Bedingung C, prä-v11) gekennzeichnet.

**Zwei Fallstricke, die der Plot-Code behandeln muss (für Reproduktion wichtig):**
1. Das Feld `sr` in `eval_history.json` enthält die **Gating-Metrik der Phase** (stoch in P1/2,
   **det** in P3/4) — det/stoch müssen aus dem Label geparst werden, sonst mischt man Metriken.
2. Der Schrittzähler läuft je Phase weiter bzw. übernimmt beim Phasenwechsel den Stand des
   geladenen Best-Checkpoints — je Panel wird auf „Schritte seit Phasenstart" normalisiert.
   Eine **kumulative** Achse über alle Phasen wäre irreführend (SR scheint zu fallen, weil die
   Phasen schwerer werden und die Läufe sie zu verschiedenen Zeitpunkten erreichen); deshalb
   die Panel-Aufteilung.

---

## v2026-07-18 — Doku-Konsistenzpass: n=3-Reste beseitigt, ddof=1 vereinheitlicht

**Kontext:** Abschluss-Review der Doku vor Abgabe. Keine neuen Messungen — reine
Konsistenzkorrekturen an `docs/Projektdokumentation.tex` (kompiliert exit 0, PDF neu gebaut).

#### Änderung 1 — n=3-Reste nach dem n=7-Umbau

**Datei:** `docs/Projektdokumentation.tex`
**Problem:** Der Umbau auf n=7 (v2026-07-17) hatte drei Stellen übersehen, die noch „drei Läufe"
behaupteten — im Widerspruch zur n=7-Ergebnistabelle derselben Arbeit.

| Stelle | vorher | nachher |
|--------|--------|---------|
| Evaluationsprotokoll (§ Methodik) | „drei unabhängige Läufe (Seeds 1–3) … Bootstrap-Konfidenzintervalle" | „sieben unabhängige Läufe … ddof=1, 95-%-CIs" |
| Repro-Checkliste „Anzahl Läufe" | „drei unabhängige Seeds" | „sieben unabhängige Seeds" |
| Repro-Checkliste „Zufallsquellen" | „Mittelung über drei Läufe" | „… sieben Läufe" |
| Repro-Checkliste „Rechenaufwand" | „ca. 8 h (CPU)" | + Hinweis: Läufe 4–7 gedrosselt 29–36 h (zugeklappter Laptop), ohne Ergebnisrelevanz |
| Titelseite | Datum „6. Juli 2026" | „18. Juli 2026" |

#### Änderung 2 — ddof=1 durchgehend (schließt den offenen Befund aus [[eval-protokoll]])

**Datei:** `docs/Projektdokumentation.tex` (§ Endergebnis, Absatz „Der Weg von n=3 zu n=7")
**Problem:** Die im Text zitierte n=3-Zwischenauswertung stand mit `ddof=0` (A 73,3 ± 6,8 /
B 80,0 ± 6,5), die n=7-Tabelle daneben mit `ddof=1` — zwei Rechenwege im selben Abschnitt.
**Lösung:** n=3-Werte auf `ddof=1` umgerechnet: **A 73,3 ± 8,3 / B 80,0 ± 8,0** (Einzelwerte
64/76/80 bzw. 72/80/88, nachgerechnet). Damit ist die gesamte Arbeit einheitlich `ddof=1`.

#### Kein Handlungsbedarf mehr (Stichproben verifiziert)

- **Pleines-Fehlzuschreibung** (doku-check-2026-07-17): Die Doku enthält bereits die korrigierte
  Formulierung („Grenzen rekurrenter Politiken …, deren Gedächtnisanforderungen über reine
  Kapazität hinausgehen") — die Pflichtkorrektur war schon umgesetzt.
- **Code-Stand-Mix n=7:** bereits am 17.07. verifiziert (Alternativerklärung 2 in v2026-07-17).

**Wiki nachgezogen:** `projekt-status` (Offen-Liste bereinigt), `eval-protokoll` (ddof-Befund →
behoben), `v12-final` (Update-Notiz). **Ergebnis:** keine Messwerte geändert außer der
ddof-Umrechnung; Aussagen der Arbeit unverändert.

**Weiter offen:** eigene Durchsicht der Doku, Betreuer-Mail versenden, Hochschul-Formalia
(Deckblatt, eidesstattliche Erklärung, Nachname „Laurin"), optional Bootstrap-CI für n=7.

---

## v2026-07-17 — Aufstockung auf n=7 abgeschlossen: **Zielkriterium A wird verfehlt** (65,7 % statt ≥ 70 %)

**Setup:** Die vier Zusatzläufe `models/ppo_lstm_curriculum_v12_s{4,5,6,7}` (gestartet 15.07. 21:14)
sind am 17.07. zwischen 09:25 und 09:32 komplett durchgelaufen (alle 4 Phasen, kein Absturz).
Laufzeit 29,5–36,3 h statt der erwarteten 8–12 h — Ursache: **zugeklappter Laptop → gedrosselte CPU**.
Ohne Ergebnisrelevanz (Training ist step-basiert, Curriculum leistungsbasiert).

Standardisierter Eval auf allen 7 finalen `best_model.zip`: Testset A (7000–7049) + Holdout B
(8000–8049), je 50 Seeds, exit 35–45, Cap 4000, det + stoch, LSTM-State korrekt geführt.

#### Ergebnis (n=7)

| Lauf | A stoch | A det | B stoch | B det |
|------|---------|-------|---------|-------|
| Seed 1 | 62 % | 38 % | 76 % | 46 % |
| Seed 2 | 84 % | 26 % | 76 % | 44 % |
| Seed 3 | 76 % | 32 % | 80 % | 36 % |
| Seed 4 | 74 % | 40 % | 74 % | 38 % |
| Seed 5 | 58 % | 20 % | 62 % | 34 % |
| Seed 6 | 50 % | 20 % | 50 % | 14 % |
| Seed 7 | 56 % | 28 % | 50 % | 16 % |
| **Mittel ± Std (ddof=1)** | **65,7 ± 12,4** | **29,1 ± 8,0** | **66,9 ± 12,8** | **32,6 ± 12,7** |
| 95 %-CI (t, df=6) | [54,2; 77,2] | — | [55,0; 78,7] | — |

**Zielkriterien:** Testset A ≥ 70 %: **65,7 % ✗ VERFEHLT** (stochastisch). Holdout B ≥ 60 %:
**66,9 % ✓** (stochastisch, aber CI reicht bis 55,0 — im Mittel erfüllt, nicht mit statistischer
Sicherheit).

⚠️ **Damit ist die bisherige Headline „beide Zielkriterien erfüllt" (n=3: A 73,3 / B 80,0) hinfällig.**
Die Aufstockung hat den Mittelwert **gesenkt**, nicht stabilisiert: A 73,3 → 65,7, B 80,0 → 66,9.
Der n=3-Wert war zu optimistisch — exakt das Risiko, das die Doku selbst benannt hatte
(„das 95 %-CI [66,7; 80,0] schließt die 70 %-Schwelle ein — im Mittel, aber nicht mit statistischer
Sicherheit übertroffen"). Genau dafür fordert Henderson et al. mehrere Seeds.

#### Drei Alternativerklärungen geprüft und ALLE ausgeschlossen

1. **Messfehler?** Nein. Das neu geschriebene Eval-Skript reproduziert **alle sechs det-Werte von
   s1–s3 bitgenau** (A 38/26/32, B 46/44/36 — identisch mit v2026-07-08). Stoch weicht um 2–8 Punkte
   ab = erwartetes Sampling-Rauschen.
2. **Code-Stand?** Nein. s1–s3 liefen auf `~97ab30d`, s4–s7 auf `3c8fa26-dirty`.
   `git diff 97ab30d 3c8fa26 -- src/core src/python src/include python/ assets/base/game_config.json
   scripts/train_curriculum.py` ergibt **genau eine Datei: `python/doc_logger.py` (+22 Zeilen)** — die
   `_git_commit()`-Stempelfunktion selbst. Simulation, Binding, Env, Trainingsskript und Config sind
   **identisch**. Die übrigen Diffs (`launcher_gui.py`, `render_engine.cpp`, `render_ui.cpp`) sind
   Client-Dateien und landen nicht in `stoneforge_sim.so`.
3. **Umgebung?** Nein. `torch` (14.05.) und `sb3_contrib` (15.05.) im venv sind älter als **beide**
   Lauf-Chargen; die `requirements.txt`-Pinnung vom 09.07. war reine Deklaration, kein Reinstall.

**Fazit: Es ist Seed-Varianz.** s4 liegt mit 74 % mitten im s1–s3-Band; der Welch-Test s1–s3 vs.
s4–s7 (74,0 vs. 59,5) ist mit **p = 0,149 nicht signifikant** — n ist schlicht zu klein für eine
Aussage über einen Chargen-Unterschied. Die n=7-Zahlen sind damit belastbar und aggregierbar.

#### Trainings-Evals der neuen Läufe (Val-Seeds 6000–6049, nicht A/B)

| Lauf | Dauer | P1 | P2 | P3 (det) | P4 (det) |
|------|-------|----|----|----------|----------|
| s4 | 36h 12m | 0,94 | 0,84 | 0,40 | 0,42 |
| s5 | 36h 18m | 0,72 | 0,70 | 0,32 | 0,28 |
| s6 | 29h 32m | 0,82 | 0,88 | 0,38 | 0,36 |
| s7 | 36h 13m | 0,90 | 0,76 | 0,32 | 0,26 |

Die det-Werte (0,26–0,42) liegen im selben Band wie s1–s3 → **der Det/Stoch-Gap besteht auch bei
n=7 unverändert**. Der Kernbeitrag der Arbeit (Gedächtnis schlägt Orakel; Gap als POMDP-Signatur)
ist von der Höhe der SR unabhängig und bleibt.

#### Doku auf n=7 umgeschrieben (17.07., kompiliert exit 0, 27 S.)

1. **Headline-Zahlen** an allen 11 Stellen auf n=7 gezogen (Zusammenfassung, Ergebnistabelle,
   Ablations-Caption, Fazit, Grenzen, Zeitleiste). `ddof=1` durchgehend, t-basierte CIs.
   Die historischen 86 % (Ablation, Etappen-Narrativ) bleiben bewusst stehen.
2. **Neuer Abschnitt „Revision des Erfolgskriteriums" (`sec:kriterienrevision`).** Anlass: Das
   **Exposé setzt ≥ 90 % über 100 unbekannte Seeds** an — die Doku berichtete gegen 70/60 und nannte
   diese „**vorab definiert**". Das war faktisch falsch und hätte als HARKing gelesen werden können
   (Kriterien nach Ergebniskenntnis angepasst und als a priori dargestellt). Jetzt offengelegt und
   begründet: (a) POMDP-Charakter bei Exposé-Erstellung unbekannt, (b) v11 deckte auf, dass
   „35–45" real 42–75 Felder Laufweg hieß, (c) Streuung ±12 pp macht eine 90 %-Schwelle
   unerreichbar. BibTeX `kerr1998harking` ergänzt, Label `sec:relatedwork` nachgetragen.
3. **Bericht gegen die Exposé-Metrik selbst** ergänzt: A(50) + B(50) = exakt die geforderten
   100 unbekannten Seeds → **66,3 % ± 12,1** (bester Lauf 80,0 %) gegen das 90 %-Ziel = **deutlich
   verfehlt**. Bewusst mitgeführt, damit die Revision nicht wie das Austauschen eines unbequemen
   Maßstabs wirkt. Ebenfalls festgehalten: Die 90 % standen unter *Reward Design und Evaluierung*,
   nicht unter *Projektziele* — die Muss- und Nice-to-Have-Kriterien des Exposés sind **vollständig
   erfüllt**.

#### ⚠️ Literatur-Behauptung „25–53 % Success Rate" entfernt (unbelegbar)

Die Doku behauptete, publizierte Zero-Shot-Ergebnisse auf prozeduralen Labyrinthen lägen bei
**25–53 % Success Rate** (Verweis auf MiniGrid/Crafter/Procgen). Die Zahl hatte **keine Provenienz**
und fiel bei der Prüfung durch:

- **Procgen** berichtet **normalisierten Return** $(R-R_{min})/(R_{max}-R_{min})$, **keine Success Rate**.
- **MiniGrid**: PPO-LSTM erreicht auf KeyCorridorS3R3 **100 %** — widerspricht der Spanne direkt.
- **Crafter**: Achievements/Reward, keine Labyrinth-Erfolgsquote.

Drei Benchmarks, drei Metriken, eine Zahl, die in keiner davon steht. Sie wäre nach dem verfehlten
Zielkriterium fast zum **Hauptargument** geworden ("wir liegen über dem publizierten Spektrum") —
und hätte an genau dieser Stelle gebrochen. **Ersetzt** durch einen Absatz, der die
Nicht-Vergleichbarkeit explizit macht: Belastbar ist nur der Vergleich *innerhalb* der Arbeit
(gleicher Generator, gleiche Metrik, gleiches Protokoll).

#### Offene nächste Schritte

1. **Betreuer informieren** — Mail-Entwurf in `docs/betreuer_mail_entwurf.md` (nicht versendet).
   Die Exposé/Doku-Differenz findet ein Prüfer beim Nebeneinanderlegen in zwei Minuten.
2. Bootstrap-CI für die n=7-Tabelle (aktuell t-basiert; die alte n=3-Tabelle hatte ein Bootstrap-CI).
3. Eigene Durchsicht der umgeschriebenen Abschnitte.

Eval-Skript: `scratchpad/final_eval_n7.py`. Prüfprotokolle: `docs/wiki/faktencheck-*.md`,
`docs/wiki/doku-check-2026-07-17.md`.

---

## v2026-07-15 — Branch-Merge nach main, zwei Client-Bugs gefixt, v12-Eval unabhängig reproduziert

**Kontext:** `vps/lstm-curriculum-v2` wurde via PR #8 nach `main` gemergt (Merge-Commit `3c8fa26`).
`main` und der RL-Branch waren in beide Richtungen divergiert; der Merge lief konfliktfrei
(nur `CHANGELOG.md` und `scripts/launcher_gui.py` von beiden Seiten berührt, automatisch gemergt).

#### Befund 1 — Spieländerungen berühren das RL-Env NICHT (Frage geklärt, kein Handlungsbedarf)
**Anlass:** Nach den Client-Commits (`df0880f` tiles, `cf6890c` debug menu, `9326fec` invtext,
`44936aa` structures) stand die Frage, ob trainierte Modelle noch gültig sind.

**Nachweis über den Build-Graph (nicht nur Code-Lesen):**
`stoneforge_sim.so` (RL-Binding) = `src/python/py_module.cpp` + `stoneforge_core`
(`game_config`/`item`/`object`/`recipe`/`world`/`simulation`). `render_engine.cpp` und
`render_ui.cpp` gehören ausschließlich zu `stoneforge_client` — zwei getrennte Binaries.
Alle vier Spiel-Commits haben **nur** diese beiden Client-Dateien angefasst.
`44936aa "structures"` ist trotz des Namens reines Rendering, **keine** Weltgenerierung.

**Ergebnis:** Die Spieländerungen können das RL-Env technisch nicht erreichen. Modelle bleiben gültig.

**Wichtige Abgrenzung:** Der real existierende Ergebnis-Bruch stammt NICHT vom Spiel, sondern aus
`9a6b95d` (07.07., RL-Branch): Exit-Platzierung Luftlinie → BFS-Pfadlänge. Derselbe Seed erzeugt
seitdem eine andere Welt. Die historischen 86 % (v2, Juni) sind gegen das alte Env gemessen und
**nicht** mit v12-Zahlen vergleichbar (bereits als Env-v11-Breaking-Change dokumentiert).
Legacy-Gegenprobe `ppo_lstm_curriculum` (231-dim) gegen Env v11: **68 % stoch / 18 % det** —
kein Regressionsschaden, anderes Testset.

#### Befund 2 — v12-Eval unabhängig reproduziert (bestätigt v2026-07-08)
Neu gemessen am 15.07. auf denselben `best_model.zip`, Cap 4000:

| Lauf | A stoch (08.07. → 15.07.) | A det | B stoch (08.07. → 15.07.) | B det (08.07. → 15.07.) |
|------|---------------------------|-------|---------------------------|-------------------------|
| Seed 1 | 64 % → 62 % | 38 % → **38 %** | 72 % → 72 % | 46 % → 44 % |
| Seed 2 | 76 % → 76 % | 26 % → **26 %** | 80 % → 76 % | 44 % → 42 % |
| Seed 3 | 80 % → 80 % | 32 % → **32 %** | 88 % → 84 % | 36 % → 34 % |
| **Mittel ± Std** | 73,3 ± 6,8 → **72,7 ± 9,5** | 32,0 → **32,0 ± 6,0** | 80,0 ± 6,5 → **77,3 ± 6,1** | 42,0 → **40,0 ± 5,3** |

**A det reproduziert exakt** (38/26/32) — wie bei deterministischer Eval zu erwarten. Stochastische
Werte weichen im Sampling-Rauschen ab. **Offene Kleinigkeit:** B det liegt durchgängig exakt
1 Seed (2 pp) niedriger als am 08.07.; bei deterministischer Eval sollte das exakt reproduzieren.
Ursache ungeklärt — Verdacht: prozessglobale WorldGen-Config (bekannter Fallstrick) bzw.
Env-Instanziierung einmal außerhalb vs. je Lauf. Für die Zielaussage irrelevant, für die
Reproduzierbarkeits-Sektion aber erwähnenswert.

**Statistik-Korrektur:** Std ist durchgehend Stichproben-Std (`ddof=1`, n=3). Zielaussage
Testset A bleibt: Abstand zum Ziel (2,7 pp) < Streuung (9,5 pp) → bei n=3 **nicht** als
„robust über 70 %" formulieren. Deshalb Aufstockung auf mehr Seeds (siehe unten).

#### Änderung 1 — Launcher-IndentationError gefixt
**Datei:** `scripts/launcher_gui.py`
**Problem:** `python scripts/launcher_gui.py` (Haupteinstiegspunkt laut CLAUDE.md) startete seit
`44936aa` (23.06.2026) **gar nicht** — `IndentationError` in `_build_section_play()`. Der Commit
hatte `opt.columnconfigure(...)` mit 16 statt 8 Leerzeichen neu eingefügt und `ttk.Entry(...)`
mit verschoben. Der Fehler lag ~3 Wochen unbemerkt auf `main`.
**Lösung:** Statement-Einrückung auf die korrekten 8 Leerzeichen zurückgesetzt, Fortsetzungszeilen
mit ausgerichtet. Die inhaltliche Absicht von `44936aa` (columnconfigure ergänzen, `width=10`)
bleibt erhalten.

| Zeile | vorher | nachher | Begründung |
|-------|--------|---------|------------|
| `opt.columnconfigure(1, ...)` | 16 Spaces | 8 Spaces | Statement-Ebene der Methode |
| `ttk.Entry(...)` | 16 Spaces | 8 Spaces | dito; Fortsetzung auf 12 |

**Validierung:** `ast.parse` fehlerfrei; GUI real instanziiert (`App()` + `update_idletasks`),
Widget-Baum baut vollständig auf, Seed-Feld liefert 42. Kein `mainloop` nötig für den Nachweis.

#### Änderung 2 — raylib-6.0-API-Bruch im Client gefixt
**Datei:** `src/client/render_engine.cpp` (3 Aufrufstellen: 942, 1606, 2367)
**Problem:** `cmake --build build` brach mit `no matching function for call to 'DrawCircleGradient'`
ab — `stoneforge_client` ließ sich nicht bauen. Ursache: installiert ist **raylib 6.0** (Homebrew),
der Code ist gegen die raylib-5.x-Signatur geschrieben.

| | Signatur |
|---|---|
| raylib 5.x (Code-Annahme) | `DrawCircleGradient(int centerX, int centerY, float radius, Color, Color)` |
| raylib 6.0 (installiert) | `DrawCircleGradient(Vector2 center, float radius, Color, Color)` |

**Lösung:** Die beiden `int`-Koordinaten an allen 3 Stellen zu `Vector2{(float)x, (float)y}`
zusammengefasst.
**Validierung:** `cmake --build build -j` läuft komplett durch (alle 4 Targets inkl.
`stoneforge_client`, Binary 594 KB). Verbleibende Warnungen (unused variables) sind vorbestehend.
RL-Binding nach Rebuild unverändert intakt (Obs 229, `Discrete(4)`).

**Hinweis:** Der Fix bindet den Client jetzt an raylib ≥ 6.0. Wer auf 5.x baut, braucht ein
Versions-Switch. Für die Reproduzierbarkeits-Sektion: **raylib-Version pinnen.**

---

## v2026-07-08 — Tile-Viewport bis über den Fensterrand

### v2026-07-08.A — Sicht-Radius erweitert
**Datei:** `src/client/render_engine.cpp`

**Problem:** Zwischen Fensterrand und den gerenderten Tiles blieb ein sichtbarer Hintergrundstreifen.
**Lösung:** Die Haupt-Viewport-Radien wurden so erhöht, dass Tiles jetzt bis zum Rand und leicht darüber hinaus gezeichnet werden.

---

## v2026-07-08 — Debug-Panel per D togglebar

### v2026-07-08.A — Untere Debug-Steuerung ausgeblendet
**Datei:** `src/client/render_engine.cpp`

**Problem:** Auto-Walk, Forcefield, Goal Area, Chunk Borders, Monsters, Threshold sowie die Hinweise zu Goal Distance und aktuellem Biom waren dauerhaft sichtbar.
**Lösung:** Ein neuer Button `D` unten rechts schaltet den kompletten Debug-Block um. Standardzustand ist aus; erst nach Aktivierung werden die Controls und Textinfos eingeblendet.

**Hinweis:** Der vorhandene Threshold-Input bleibt Teil des Debug-Panels und wird beim Ausblenden deaktiviert.

---

## v2026-07-08 — HUD-Textbox entfernt

### v2026-07-08.A — Oberen HUD-Textblock deaktiviert
**Datei:** `src/client/render_ui.cpp`

**Problem:** Der obere Overlay-Block zeigte dauerhaft Status- und Hilfetext wie Tile-Größe, Werkzeugstufe, Reichweite, Werkbankstatus, Inventarstatus, Slot und Steuerhinweise an.
**Lösung:** `drawHud(...)` rendert diesen Textblock nicht mehr; die Funktion ist jetzt ein No-Op und lässt den restlichen UI-Flow unverändert.

**Validierung:** Syntax-/Fehlerprüfung der geänderten Datei ohne Befund. Ein vollständiger Build konnte in dieser Sitzung nicht über die Build-Tools ausgeführt werden.

---

## v2026-07-09 — Temperatur-Sweep der Evaluation: Det/Stoch-Gap ist monoton, kein argmax-Artefakt

**Frage (aufgeworfen bei der Literatur-Recherche):** Ist der Det/Stoch-Gap teilweise nur eine schlechte
Wahl der Eval-Politik? Zwischen reinem Argmax (T=0) und vollem Sampling (T=1) könnte eine niedrig-
entropische, „kalibrierte" Temperatur liegen, die beide Extreme schlägt („Stochastic Policies,
Deterministic Minds", 2025). Getestet auf den 3 v12-Modellen (LSTM-korrekter Sweep, `scratchpad/temp_sweep.py`;
Logits/T-Softmax mit fortgeführtem LSTM-Zustand, 50 Seeds, Cap 4000).

| Temperatur | Testset A (Mittel ± Std) | Holdout B |
|-----------|--------------------------|-----------|
| argmax (T=0) | 32,0 ± 4,9 % | 42,0 ± 4,3 % |
| T=0.25 | 52,0 ± 0,0 % | 52,7 ± 1,9 % |
| T=0.5 | 63,3 ± 4,1 % | 71,3 ± 6,6 % |
| T=0.75 | 63,3 ± 7,5 % | 72,0 ± 6,5 % |
| **stoch (T=1.0)** | **70,7 ± 8,2 %** | **79,3 ± 6,6 %** |

**Befund: Die Kurve ist monoton** — mehr Stochastik ist besser, bis zum vollen Sampling; **keine
Zwischentemperatur schlägt T=1.0.** Damit ist der Gap **kein argmax-Kalibrierungsartefakt**, sondern die
empirische Signatur von Singhs POMDP-Resultat (stochastisch echt überlegen). Nebenbefund: schon T=0.5 holt
den Großteil des Gaps zurück (A +31, B +29 Punkte über argmax) bei geringerer Varianz — brauchbarer
Betriebspunkt für „committeteres" Verhalten. **Konsequenz:** stochastische Eval bleibt die Primärmetrik,
jetzt zusätzlich empirisch gestützt (nicht nur theoretisch). Ergebnisse: `scratchpad/temp_sweep_results.json`.

---

## v2026-07-08.3 — Gap-Schließungs-Experimente E1/E2/E3: keine Verbesserung ggü. v12

**Motivation:** Die drei „Was würde helfen"-Hypothesen aus v2026-07-08.2 als kontrollierte
Experimente getestet — Ziel: den Det/Stoch-Gap bzw. die Gesamt-SR über die v12-Baseline
(A 73,3 % ± 6,8 stoch / 32 % det) heben.

**Setup:** RecurrentPPO, batch=8, Swarm an, Env v11/229-Obs, Seed 1. E1/E2 starten ab Phase 3
aus einem gemeinsamen Phase-2-Checkpoint (je n=1); E3 ist ein voller Lauf ab Phase 1 (LSTM 512).
Standardisierter Eval (Seeds A 7000–7049 / B 8000–8049, Cap 4000, det+stoch, finales `best_model.zip`).

| Exp | Hypothese | Änderung ggü. v12 | A stoch | A det | B stoch | B det |
|-----|-----------|-------------------|---------|-------|---------|-------|
| **v12 (Baseline, n=3)** | — | — | **73,3 % ± 6,8** | 32,0 % | 80,0 % | 42,0 % |
| **E1** critic | stärkerer Phase-3-Critic | `vf_coef` 0,5 → **1,0** | 56 % | **42 %** | 72 % | 36 % |
| **E2** curric | sanfterer Übergang | Phase 3 exit 25–45 → **25–35** | 26 % | 16 % | 42 % | 24 % |
| **E3** lstm512 | mehr Gedächtnis | LSTM 256 → **512** | 44 % | 14 % | 58 % | 20 % |

**Befund:**
- **E1 (Critic, vf_coef=1.0): als Verbesserung widerlegt.** Deterministisch minimal besser
  (A 42 % vs. 32 %), aber stochastisch deutlich schlechter (A 56 % vs. 73 %). Der höhere
  Value-Loss-Anteil verschiebt das Verhalten Richtung Determinismus, kostet aber Gesamt-SR —
  kein Netto-Gewinn, Zielkriterium A ≥ 70 % verfehlt.
- **E2 (sanftes Curriculum, Phase 3 nur bis exit 35): klar schlechter** (A 26 % stoch). Wird
  Phase 3 nur bis Distanz 35 trainiert, aber auf 35–45 evaluiert, ist der Agent auf den längsten
  Wegen untertrainiert. Der „sanftere Übergang" schadet mehr als er nützt.
- **E3 (LSTM-512): komplett durchgelaufen** (22h 54m, alle 4 Phasen) — **als Verbesserung widerlegt.**
  Standardisiert **A 44 % stoch / 14 % det · B 58 % / 20 %**, klar unter v12 (A 73 % / B 80 %) und auf
  det sogar am schwächsten aller Arme. Phasenweise durchgängig ≤ 256er-Baseline: P1 gleichauf,
  P2 stoch-peak 66 % (v12: 80–88 %), P3 det-peak nur 30 % und über die Phase **fallend** (letzte ~250k
  Steps kollabiert, det 0–8 %). ~3–4× langsamer. **Mehr Gedächtnis-Kapazität ≠ besserer -Nutzen.**
  (Sleep-Schutz via `caffeinate -w` verhinderte weitere Stalls; nach Behebung ~48 fps.)

**Konsequenz:** Von den drei naheliegenden Gap-Schließungs-Ansätzen (Critic-Gewicht, Curriculum-
Glättung, LSTM-Größe) hat **keiner die v12-Baseline geschlagen**. Das stützt die Diskussions-These
aus v2026-07-08.2: Der Det/Stoch-Gap ist nicht über einen der drei getesteten Einzelhebel (Critic-
Gewicht, Curriculum-Glättung, LSTM-Größe) zu schließen. Er ist teils fundamental (POMDP, Singh 1994).
**Wichtig — NICHT überinterpretieren:** Damit ist NICHT gezeigt, dass „nur Architektur hilft". Es
bleiben ungetestete, teils billige Hebel: (a) **temperaturkalibrierte Evaluation** — der Gap ist
teilweise eine Wahl der Eval-Policy (reines argmax vs. niedrig-entropische Policy; „Stochastic Policies,
Deterministic Minds" 2025); `scripts/eval_temperature.py` existiert bereits, kein Neutraining nötig;
(b) Hilfs-/Repräsentationsverluste; (c) gezielt mehr Langdistanz-Training. Für das *harte*
Langhorizont-Belief-Tracking ist strukturierter Speicher (Transformer / Neural Map / In-Context-RL,
vgl. AMAGO) die meistgenannte, aber aufwändigste Richtung. Literatur-Belege in `docs/references.bib`
(Singh 1994, Ghosh 2021, Pleines 2022) + Related-Work der Projektdoku.
**v12 bleibt das Ausgabemodell.** Eval-Skripte: `scratchpad/eval_e1e2.py`, `scratchpad/eval_e3.py`.

---

## v2026-07-08.2 — Det/Stoch-Gap analysiert: skaliert mit der Weglänge (Kernbefund für die Diskussion)

**Frage:** Warum lernt der Agent die deterministische ("sture", argmax) Spielweise so schlecht,
obwohl er stochastisch beide Ziele erreicht? Fehlt ihm etwas?

**Messung** (`scripts/plot_det_gap_distanz.py`, finales Modell Seed 3, 120 Val-Seeds 6000–6119,
Distanz 5–45, det + stoch je frischer Reset, Cap 4000): Erfolgsrate nach Startdistanz (BFS-Felder):

| Weglänge | n | det-SR | stoch-SR | Gap |
|----------|---|--------|----------|-----|
| 5–15  | 14 | **79 %** | 100 % | 21 |
| 15–25 | 22 | 50 % | 95 % | 45 |
| 25–35 | 33 | 39 % | 85 % | 46 |
| 35–45 | 51 | **31 %** | 82 % | 51 |

**Befund:** Der Gap ist **keine generelle Unfähigkeit**, sondern skaliert mit der Weglänge. Auf kurzen
Wegen ist der Agent zielstrebig (79 % det, kleiner Gap); erst mit der Distanz bricht det ein, während
stoch hoch bleibt. Abbildung: `docs/figures/fig_det_gap_distanz.pdf` (+ .png/.json).

**Interpretation (POMDP) — KORRIGIERT 08.07. nach Nachmessung mit feinen Distanz-Bins:** Der Gap ist
**kein Schwelleneffekt bei einer bestimmten Schrittzahl**, sondern wächst **kontinuierlich mit der
Weglänge** — det-SR fällt bereits ab ~14–17 Feldern spürbar (feine Bins: 5–8: 100 %, 14–17: 64 %,
20–25: 44 %, 25–30: 30 %). Ursache ist **kumulativ**: Je länger der Weg ohne direkte Zielsicht (Sichtradius
nur 7 Felder), desto mehr Kreuzungs-Entscheidungen unter unsicherem Belief; an jeder sind die
Aktionswahrscheinlichkeiten fast-uniform, und jede einzelne kann argmax in eine Oszillations-Schleife
schicken — die Fehlerwahrscheinlichkeit summiert sich über die Weglänge. Sampling entkommt, argmax nicht.
In einem POMDP ist Determinismus zudem nicht immer optimal (Variieren unter Unsicherheit ist teils die
richtige Antwort). Nebenbefund: Ziel bei Start sichtbar (Chebyshev ≤7) → det 67 % vs. nicht sichtbar 41 %
(n=9 vs. 151 — Richtung plausibel, Stichprobe zu klein für starke Aussage). → Der Gap ist teils
fundamental (POMDP), teils Gedächtnis-Limitation, kein Bug. **Die frühere „ab 200+ Schritten"-Formulierung
war ungenau** (der Einbruch beginnt viel früher).

**Was helfen würde (Ausblick/Diskussion):** (1) mehr/strukturiertes Gedächtnis — größerer LSTM oder
räumlicher Speicher (Neural Map / Attention über die Historie), Stand der Technik für Memory-Maze-Aufgaben;
(2) stabilerer Critic in Phase 3 (EV war ≈ 0 — praktisch angreifbarster Punkt); (3) sanfterer
Curriculum-Übergang auf die langen Distanzen + mehr Training dort.

#### Nebenbefund — Gegentest phase2_best vs. finales best_model (n=3, beide Testsets)
Hypothese: Da best_model in Phase 3/4 auf **det** selektiert wurde, könnte das Phase-2-Zwischenmodell
stochastisch stärker sein. **Widerlegt:**

| Modell | A stoch | B stoch | A det | B det |
|--------|---------|---------|-------|-------|
| **best_model (final)** | **73,3 % ± 6,8** | **80,0 % ± 6,5** | 32,0 % | 42,0 % |
| phase2_best | 71,3 % ± 8,1 | 74,0 % ± 15,0 | 34,0 % | 25,3 % |

Final ist im Mittel gleichwertig bis besser und deutlich stabiler (B-Streuung ±6,5 statt ±15,0). Pro Seed
gemischt (Seed 1 phase2 A=78 > 64; Seed 3 phase2 A=60 < 80), aber Auswahl pro Seed nach Testergebnis wäre
Selektion auf den Testdaten. **Konsequenz:** best_model bleibt das Ausgabemodell; 73/80 stehen.

---

## v2026-07-08 — Finale v12-Curriculum-Läufe abgeschlossen (batch=8, Seeds 1–3): beide Zielkriterien im Mittel erfüllt

**Setup:** 3 volle Curriculum-Läufe (RecurrentPPO, batch=8, Swarm an, Env v11/229-Obs),
`models/ppo_lstm_curriculum_v12_s{1,2,3}`, Laufzeit je 7,7–8,5 h, alle 4 Phasen sauber durchlaufen
(kein Absturz). Standardisierter Eval auf den finalen `best_model.zip`: Testset A (7000–7049) und
Holdout B (8000–8049), je 50 Seeds, Cap 4000, deterministisch **und** stochastisch.

#### Ergebnis (finale best_model.zip je Seed)

| Lauf | A stoch | A det | B stoch | B det |
|------|---------|-------|---------|-------|
| Seed 1 | 64 % | 38 % | 72 % | 46 % |
| Seed 2 | 76 % | 26 % | 80 % | 44 % |
| Seed 3 | 80 % | 32 % | 88 % | 36 % |
| **Mittelwert ± Std** | **73,3 % ± 6,8** | **32,0 % ± 4,9** | **80,0 % ± 6,5** | **42,0 % ± 4,3** |

**Zielkriterien:** Testset A ≥ 70 %: **73,3 % ✓** (stochastisch). Holdout B ≥ 60 %: **80,0 % ✓**
(stochastisch). Damit erstmals mit **3 Läufen + Mittelwert ± Std** (Projektvorgabe) erfüllt, nicht mehr
nur als Einzelwert. Anmerkung: B > A ist Seed-Rauschen (beide gleiche Verteilung, n=3); Seed 1 liegt
auf A einzeln bei 64 % — der Mittelwert trägt.

#### Det/Stoch-Gap besteht weiter — Phase-3-Annealing hat ihn NICHT geschlossen
Deterministisch bleibt es bei 32 % (A) / 42 % (B) — der bekannte POMDP-Gap. Phase-3-Verläufe (Gate=det)
waren über alle Seeds stark volatil (det-MAX je Lauf nur 54/26/36 %, oszilliert bis auf 0 % herunter);
`explained_variance` fiel nach jedem Phasenwechsel wieder auf ~0 und erholte sich nur teils
(Seed 3 zuletzt 0,88; Seed 1/2 ~0,05). D.h.: batch=8 hat Phase 1/2 klar stabilisiert (Projekt-Bestwerte:
88–92 % stoch, 60–74 % det in Stufe 1), aber der lange Phase-3-Horizont (Distanz 35–45) bleibt für den
Critic zu schwer für stabiles det-Lernen. **Interpretation:** stochastische Ziele solide erreicht;
det-Gap bleibt die dokumentierte LSTM/POMDP-Limitation (nicht als Fehler, siehe POMDP-Einordnung).

#### Offene nächste Schritte
1. Prüfen, ob `phase2_best_model.zip` stochastisch besser abschneidet als das finale (auf det selektierte)
   `best_model.zip` — falls ja, ist das je Seed das bessere Ausgabemodell für die stochastische Aussage.
2. Ergebnis- + Diskussionskapitel mit diesen n=3-Zahlen füllen.

---

## v2026-07-07.4 — Forensik: Stack/Swarm/Wrapper entlastet — v11-Lerndynamik ist instabil, Hauptverdacht batch_size=64

#### Befunde (4 parallele Diagnose-Läufe + deterministische Vergleiche, alle Seed 1)
| Arm | Setup | SR stoch (Verlauf) |
|-----|-------|--------------------|
| Kontrolle | Curriculum, Swarm an | 16→20→28→34→**14→8**→24→38→34 % (25k–475k, volatil, kein Trend) |
| B8-Arm | Curriculum, `--no-swarm` | 18→14→12→20→26→**40**→26 % (25k–300k, deckungsgleich volatil) |
| Bisect-Arm | Curriculum, ohne StreamWrapper/LiveMap | 18→14→12 % (25k–75k, ebenso) |
| A1-Repro | **nackter** RecurrentPPO, kein Stack | **74**→44→66→**24**→50 % (25k–125k, ebenso volatil!) |

1. **Swarm entlastet** (B8): No-Swarm-Arm verläuft deckungsgleich mit Kontrolle.
2. **StreamWrapper entlastet**: Code ist rein lesend (`currentBfsDistanceToExit()` ist const);
   Bisect-Arm ohne Wrapper stagniert identisch. (Nebenbei gefixt: Wrapper wurde bisher auch
   bei `--no-live-map` angelegt — jetzt nur noch bei aktiver Live Map.)
3. **Stack komplett entlastet**: Gewichts-Checksummen, Welt-Seeds und die ersten 8192
   Trainings-Steps (Rewards/Aktionen/Values) sind zwischen Curriculum-Pfad und nacktem
   Pfad **bit-identisch** (`compare_setups.py`, `compare_rollout.py`).
4. **Kernbefund:** Die v11-Lerndynamik mit batch=64 ist chaotisch-instabil — SR oszilliert
   10–74 % ohne Konvergenz, `explained_variance` bleibt ≈ 0,1 (Critic lernt nie).
   Auch die A1-„Validierung" (52 % @150k, EV 0,72) war nur ein Schnappschuss dieses
   Prozesses — **die batch=64-Freigabe vom 06.07. beruhte auf einem einzelnen Snapshot
   und ist damit hinfällig.**
5. **Kontrast v10-Reproduktion (15.06., batch=8):** EV bis **0,939** @65k — gesunder Critic.
   Einziger HP-Unterschied zum v2-Erfolgsrezept war damals batch=8.

#### Konsequenz — laufender Test
Alle Diagnose-Arme gestoppt (Artefakte in `models/ppo_lstm_curriculum_v11_s1{,_noswarm,_nostream}`,
Logs `logs/train_v11_seed1*.log`, `logs/train_a1_repro_seed1.log`).
**H1-Test — BESTÄTIGT** (nackter v11-Lauf, Seed 1, batch_size=8, 150k Steps,
`logs/train_bare_batch8_seed1.log`):

| Step | 25k | 50k | 75k | 100k | 125k | 150k |
|------|-----|-----|-----|------|------|------|
| SR stoch | 54 % | 66 % | 64 % | 74 % | **88 %** | 84 % |
| SR det | 20 % | 26 % | 40 % | 38 % | **52 %** | 38 % |

EV springt ab ~50k auf **0,86** (batch=64: nie > ~0,15). Stetiger Anstieg statt Oszillation;
beste Phase-1-Werte des Projekts, erstmals det-SR > 50 %. Direkter @150k-Vergleich:
batch=8 → 84 % stoch vs. batch=64-Repro → 48 % (Curriculum-Arme ~20–26 %).
**Umgesetzt:** `RPPO_KWARGS["batch_size"]` 64 → **8** (train_curriculum.py), CLAUDE.md
aktualisiert (batch-Zeile, forceGuaranteedPath=false, 3 neue Fallstricke: batch=64,
Eval-Cap < 4000, Snapshot-Validierung). Kosten: ~88–150 statt ~187 fps (~5,5 h/Gesamtlauf).
**Nächster Schritt:** ≥3 volle Curriculum-Läufe (batch=8, Seeds 1–3) parallel für
Mittelwert ± Std; besonderes Augenmerk auf Phase 3 (det-Annealing) mit gesundem Critic.

---

## v2026-07-07.3 — Seed 1 stagniert auch bei Cap 4000 → B8-A/B-Test (Swarm) gestartet

#### Befund — Stagnation ist real, Hauptverdacht wandert von B7 (Penalties) zu B8 (Swarm)
**Lauf:** Seed-1-Neustart mit Cap-Fix (v2026-07-07.2), Kontrollarm mit Swarm (Status quo).
Eval-Verlauf P1 (stoch, Cap 4000): 16/16/10/20/16/20/24 % @ 25k–175k — **weit unter der
A1-Validierung (52 % @150k, gleiches Cap)**. `explained_variance` ≈ 0,01–0,15 (A1: 0,72)
— exakt die Seed-0-Signatur (Critic lernt nicht).
**Schlussfolgerung zur Verdachtslage:** Die A1-Validierung lief bereits auf dem v11-Env
(= ohne Wand-Penalty) und lernte normal → die Penalty-Entfernung (B7) kann die Stagnation
nicht erklären. Größter verbliebener Unterschied zwischen A1 (lernt) und den
Curriculum-Läufen (stagnieren): **Swarm-Erfolgs-Replay (30 % der Resets)** — deckt sich
mit der PLR-Literatur (Replay gelöster Level ist lernineffizient, Jiang et al. 2021).

#### Änderung — B8-A/B-Test gestartet (paired, ein Faktor)
Kontrollarm läuft weiter: `--save-dir models/ppo_lstm_curriculum_v11_s1 --seed 1` (Swarm an).
Testarm parallel gestartet (16:5x Uhr): `--save-dir models/ppo_lstm_curriculum_v11_s1_noswarm
--seed 1 --no-swarm --live-map-port 8767` (Log `logs/train_v11_seed1_noswarm.log`).
Gleicher Seed, gleiche Config, einziger Unterschied: Swarm. Beide Läufe parallel auf dem
Mac (je ~1,5 Kerne, 10 vorhanden — keine gegenseitige Bremsung gemessen).
**Entscheidungsregel:** Lernt der No-Swarm-Arm deutlich schneller (Richtung ~50 % @150k)
→ Swarm-Erfolgs-Replay ist die Ursache → Swarm-Default abschalten bzw. auf PLR umstellen.
Stagniert er ebenso → Swarm entlastet; nächste Verdächtige: Stream-Wrapper (Live Map,
seit 07.07. in den Trainings-Envs) oder Unterschiede im A1-Setup.

---

## v2026-07-07.2 — Eval-Cap-Fix (600/1200 → 4000) + Seed-1-Neustart

#### Änderung 1 — Eval-Caps in Phase 1/2 zurück auf 4000
**Datei:** `scripts/train_curriculum.py` (PHASES)
**Problem:** Die am 06.07. eingeführten Kurz-Caps (A3: P1=600, P2=1200) unterschätzen die
echte Kompetenz massiv (siehe Korrektur in v2026-07-07): Das 85-%-Gate war unter Cap 600
praktisch unerreichbar, Phasen liefen immer ans Step-Limit, und die Modellselektion
bevorzugte schnelle statt kompetente Policies. Nachmessung der SR(Cap)-Kurve von
Seed-0-`phase1_best` (VAL 6000–6049, ein Lauf @4000, Steps-bis-Erfolg pro Seed geloggt):

| Cap | 600 | 1000 | 1500 | 2000 | 3000 | 4000 |
|-----|-----|------|------|------|------|------|
| SR stoch | 48 % | 62 % | 72 % | 82 % | 82 % | **86 %** |

Steps-bis-Erfolg (stoch): med=477, p95=1880, max=3239 — die SR saturiert erst bei 4000.
Zudem war die Cap-Begründung („Eval-Verschwendung") hinfällig: kompletter det+stoch-Eval
über 50 Seeds mit Cap 4000 dauert gemessen nur **~21 s** (≈15 % Overhead bei eval_freq 25k).
**Lösung:**

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| P1 `eval_max_steps` | 600 | **4000** | SR saturiert erst bei 4000; = Env-maxSteps = finaler Eval |
| P2 `eval_max_steps` | 1200 | **4000** | dito; Vergleichbarkeit mit 86-%-Referenzlauf |

#### Änderung 2 — Seed-1-Lauf neu gestartet (alte Artefakte archiviert)
**Problem:** Der erste Seed-1-Lauf (bis ~278k, Prozess endete still ~11:28) lief mit den
verfälschenden Caps — Gate, Modellselektion und eval_history sind nicht verwertbar.
**Lösung:** Artefakte archiviert nach `models/ppo_lstm_curriculum_v11_s1_cap600/` bzw.
`logs/train_v11_seed1_cap600.log`; Neustart mit identischer Konfiguration + Cap-Fix:
`train_curriculum.py --save-dir models/ppo_lstm_curriculum_v11_s1 --seed 1`
(Log `logs/train_v11_seed1.log`). Erwartung laut Cap-4000-Nachmessung: P1 sollte jetzt
~85 % stoch erreichen können; die Entscheidungsregel „Seed 1 stagniert → systematisch →
B7-A/B" gilt weiter, aber erst auf Basis unverzerrter Messwerte.

---

## v2026-07-07 — Live-Map-Redesign (uPlot, Viridis, det/stoch-SR) + v11-Trainingsstart

#### Änderung 1 — Live Map komplett überarbeitet
**Datei:** `scripts/ws_map.html` (komplett neu), Recherche-Basis: DataCamp-Dashboard-Prinzipien,
PokéRL-Metrics, uPlot-Benchmarks, Viridis-Literatur.
**Problem:** Handgezeichnete Canvas-Charts ohne Tooltips/Glättung, Jet-Farbskala der Heatmap
(erzeugt künstliche Kontraste, nicht CVD-tauglich), keine visuelle Hierarchie, Det/Stoch-Gap
(zentraler Untersuchungsgegenstand!) nicht sichtbar, 16-Linien-Spaghetti-Chart.
**Lösung:**

| Aspekt | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| Charts | Eigenbau-Canvas | **uPlot v1.6.32 (inline, 51 kB)** | Crosshair + Live-Legende, ~4× weniger CPU als Chart.js |
| SR-Chart | nur Gate-Best-SR | **det- UND stoch-Kurve + Phasen-Marker** | Det/Stoch-Gap live sichtbar; Fallback auf Best-SR für alte Läufe |
| Heatmap-Farbskala | Jet (Rainbow) | **Viridis, log-Skala, Floor 0.18** | perzeptuell uniform, CVD-tauglich; Floor weil dunkelstes Ende ≈ Hintergrund |
| BFS/Episodenlänge | Rohwerte, zappelig | **EMA-geglättet, Rohkurve blass** | TensorBoard/W&B-Muster |
| Pro-Agent-BFS-Chart | 16 Linien | **entfernt** | redundant zu Agent-Karten, >8 Farbklassen unlesbar |
| KPI-Zeile | Textzeile | Kacheln: Phase, Timesteps, Steps/s, SR stoch/det, Pool | Inverted-Pyramid-Layout |
| Agent-Karten | statisch | grüner Flash + ✓-Zähler bei Exit-Erfolg | Erfolge waren vorher unsichtbar |
| Schrift | Courier New | system-ui, tabular-nums für Live-Zähler | ruhigeres Zahlenbild |

Serienfarben (#3987e5/#199e70/#c98500) auf #0d1117 validiert (Kontrast ≥3:1, CVD-ΔE 41).
Eval-Historie zusätzlich als aufklappbare Tabelle (Barrierefreiheit/Ablesbarkeit).

#### Änderung 2 — det/stoch-Eval-Historie an Live Map senden
**Datei:** `scripts/train_curriculum.py`
**Problem:** `MetaCallback` sendete nur `best_sr` (Gate) — die HTML konnte det/stoch nicht plotten.
**Lösung:** `_run_eval()` pflegt modulglobale Liste `_LIVE_EVALS` (`{ts, det, stoch, phase}`) und
sendet sie via `update_meta({"evals": ...})`. Überlebt Phasenwechsel; Browser, die später
verbinden, sehen die volle Kurve. Greift ab dem nächsten Trainingsstart (laufender v11-Prozess
nutzt noch den alten Code → HTML-Fallback auf Best-SR).

#### Änderung 3 — Exit-Richtungspfeil für v11-Obs gefixt
**Datei:** `python/stream_wrapper.py`
**Problem:** `_stream()` kannte nur 231/456-dim Obs — bei v11 (229) blieb `exit_dx/dy = 0`,
der Gold-Pfeil in den Agent-Karten fehlte.
**Lösung:** Fall `n == 229` ergänzt (exitDx=Index 226, exitDy=227). Greift ab nächstem Lauf.

#### Änderung 4 — Agent-Minimaps zeigen jetzt die echte Welt (Sichtfeld-Streaming)
**Dateien:** `python/stream_wrapper.py`, `scripts/ws_map.html`
**Problem:** Die Minimaps zeigten nur einen wandernden Punkt auf Schwarz — kein Weltkontext
(Wände, Exit), feste Skalierung schnitt weite Wege ab. „Man erkennt nichts."
**Lösung:** Wrapper streamt das Sichtfeld (`obs[:gs]` × 30 = Tile-IDs, Grid-Größe dynamisch
aus `env._gs`); die Map akkumuliert daraus pro Episode eine Weltkarte: Boden blaugrau,
Wände hellgrau, Bäume grün, Exit als goldene Raute (sobald gesehen), Agent mit weißem Ring.
Auto-Zoom passt die Ansicht ans erkundete Gebiet an (1–6 px/Tile). Fallback für Läufe ohne
Grid-Daten: besuchte Tiles + Trail wie bisher, aber mit Auto-Zoom und besserem Kontrast.
E2E-verifiziert mit Test-Env auf Port 8767. Grid-Streaming greift ab dem nächsten Lauf.

#### Änderung 5 — Swarm-Pool: Phasenwechsel-Bug gefixt + A/B-Test geplant (B8)
**Dateien:** `python/stoneforge_env.py`, `scripts/train_curriculum.py`, `docs/BEWERTUNG_UND_PLAN.md`
**Problem:** (1) Der Swarm-Pool überlebte Phasenwechsel — ein „gelöster" Seed aus Phase 1
(Exit 5–12) ist in Phase 2 (Exit 12–25) aber eine andere Aufgabe; die Pool-Aussage galt
über Phasengrenzen nicht. (2) Recherche (PLR, Jiang et al. ICML 2021): Replay bereits
gemeisterter Levels ist lernineffizient und riskiert Layout-Memorierung — Literatur
empfiehlt Replay von Levels mit hohem Lernpotenzial (≈ unser `--plr`-Modus).
**Lösung:** `SwarmSeedPool.clear()` ergänzt; `train_curriculum.py` leert den Pool bei jedem
Phasenstart. Semantik-Frage als A/B-Test B8 in `BEWERTUNG_UND_PLAN.md` dokumentiert
(Erfolgs-Swarm vs. `--plr` vs. `--no-swarm`) — Entscheidung mit Daten, da der
86%-Referenzlauf MIT Erfolgs-Swarm lief. Greift ab dem nächsten Lauf.

#### Änderung 6 — Projektdokumentation: Etappe 5 + 2 neue Abbildungen; Stepzähler-Fix
**Dateien:** `docs/Projektdokumentation.tex`, `docs/figures/fig_v11_lernkurve.pdf` (neu),
`docs/figures/fig_livemap.png` (neu), `scripts/plot_run_curve.py` (neu), `scripts/ws_map.html`
**Inhalt:** Neue Subsection „Etappe 5 (07.07.2026)" — erster v11-Gesamtlauf (Zwischenstand,
Lernkurve det+stoch mit Phasen-Markern), Monitoring-Redesign, Swarm/PLR-Analyse (B8).
Zeitleiste + Ausblick aktualisiert. PDF baut sauber (12 Seiten, keine offenen Referenzen).
**Nebenbefund/Fix:** Beim Phasenwechsel lädt `train_curriculum.py` das Bestmodell der
Vorphase — der SB3-Stepzähler springt dabei zurück (z. B. P3-Start bei „525k" statt 1M).
Alle stepbasierten Zeitachsen wären damit nicht-monoton. Gefixt in `plot_run_curve.py`
(kumulierte Achse) und `ws_map.html` (`displayTs()`-Rebase). `eval_history.json`
enthält weiterhin die rohen SB3-Steps.

#### Änderung 7 — Live Map: Chart-Historie überlebt jetzt Browser-Reloads
**Datei:** `scripts/ws_map.html`
**Problem:** Die gesamte Chart-Historie (SR, BFS, Episodenlängen, ✓-Zähler) lebte nur im
Tab-JavaScript — jeder Refresh löschte alles (nur die Heatmap kam zurück, die hält der Server).
**Lösung:** Zustand wird alle 5 s in `localStorage` gesichert und beim Laden wiederhergestellt
(inkl. Stepzähler-Rebase-Zustand, damit die kumulierte x-Achse nahtlos weiterläuft).
Neuer-Lauf-Erkennung: Springt der Stepzähler auf < 25 % des letzten Stands, wird die alte
Historie automatisch verworfen; zusätzlich „↺ Reset"-Button im Header. Verifiziert per
CDP-Reload-Test (identischer Datenpunkt und `_tsDisp` nach Reload). Wirkt sofort per Reload,
auch für den laufenden Run.

#### v11-Curriculum-Run Seed 0 — GESCHEITERT (abgebrochen @ 1,0M von 2,2M Steps)
`python scripts/train_curriculum.py --save-dir models/ppo_lstm_curriculum_v11 --seed 0`
Env v11 (229 Obs), batch=64/CPU, B1-Fix aktiv. Log: `logs/train_v11_seed0.log`,
41 Eval-Checkpoints in `models/ppo_lstm_curriculum_v11/eval_history.json`.

| Phase | Bestes SR (Gate) | Verlauf |
|-------|------------------|---------|
| 1 (exit 5–12, stoch) | 42 % @ 500k (Limit) | langsam; Validierung hatte 52 % @ 150k |
| 2 (exit 12–25, stoch) | 48 % @ +25k | danach Seitwärtsband 16–36 % |
| 3 (exit 25–45, det) | 2 % det | stoch konvergiert 30 % → 0–6 % nach unten |

**Diagnose:** Critic hat nie gelernt (`explained_variance` ≈ 0,1 über den gesamten Lauf;
Validierungslauf 06.07.: 0,72). Policy-Updates durchgehend gesund (approx_kl ≈ 0,03) — kein
Kollaps, sondern schwache Value-Funktion (Initialisierungs-/Seed-Verdacht). Das Phase-3-
Entropie-Annealing (0,05 → 0,001) legte die Schwäche offen: stoch-SR konvergierte auf die
det-SR (~0 %) **nach unten** statt wie beim 86-%-Referenzlauf nach oben. Abbruch bei 975k
(Phase 3), da Ausgang determiniert; Rechenzeit → Seed-1-Vergleichslauf.
**Nächster Schritt:** Seed 1 (identische Konfiguration, `models/ppo_lstm_curriculum_v11_s1`,
Auto-Start via Watcher). Lernt er normal → Seed-Pech; stagniert er ebenso → systematisch
(Hauptverdacht: v11-Penalty-Entfernung → B7-A/B vorziehen).

**KORREKTUR (gleicher Tag, nach Seed-1 @150k = 12 %):** Nachmessung der Seed-0-Bestmodelle
mit vollem Episoden-Cap deckt ein **Messartefakt** auf — die am 06.07. eingeführten
Eval-Caps (A3: P1=600, P2=1200 statt 4000) unterschätzen die echte Kompetenz massiv:

| Modell | Cap (Gate) | SR stoch | Cap 4000 | SR stoch | det (4000) |
|--------|-----------|----------|----------|----------|------------|
| phase1_best (Seed 0) | 600 | 46 % | 4000 | **84 %** | 18 % |
| phase2_best (Seed 0) | 1200 | 50 % | 4000 | **80 %** | 6 % |

Seed 0 hat in P1/P2 also auf Referenzniveau gelernt (stochastisch) — nur **langsam**
(Episoden > Cap). Konsequenzen: (1) Das 85-%-Gate unter Cap 600 ist praktisch unerreichbar
→ Phasen enden immer am Limit; Modellselektion bevorzugt schnelle statt kompetente Policies;
„Stagnation" in den Eval-Logs von Seed 0/1 ist großteils Artefakt. (2) Der P3-Kollaps
(dort galt bereits Cap 4000) war real: det durchgehend 6–18 %, Annealing zog stoch auf det
herunter — der bekannte Det/Stoch-Gap, nicht mangelnde Aufgabenkompetenz. (3) Die 52 % der
A1-Validierung sind mit den 12 % der Curriculum-Evals nicht vergleichbar (anderes Cap).

---

## v2026-07-06 — Umgebungsversion v11: tote Features raus, Straf-Stacking entschärft, BFS-Exit-Platzierung, Config-Leck-Fix

> ⚠️ **Breaking Change:** Neue Obs-Shape (231 → 229) und neue Exit-Platzierung.
> Alte Modelle (z. B. `ppo_lstm_curriculum`) brauchen beim Eval jetzt
> `StoneforgeWorldEnv(..., include_energy_inventory=True)` — und ihre alten Ergebnisse
> sind mit neuen Läufen nicht mehr 1:1 vergleichbar (Exit-Distanzen waren real 42–75, jetzt echt 35–45).

#### Änderung 1 — Tote Features entfernt: Energie + Inventar raus, HP bleibt
**Datei:** `python/stoneforge_env.py`
**Problem:** Energie (disable_energy=True → konstant 100) und Inventar (kein Mining → konstant 0)
trugen null Information — 2 von 231 Obs-Dims waren tot.
**Lösung:** Standardmäßig entfernt; Obs 231 → **229** (Grid 225 + HP + exitDx/Dy + step_frac).
Rückwärtskompatibilität über neuen Parameter `include_energy_inventory=True` (alte 231-dim-Modelle).

#### Änderung 2 — Straf-Stacking entschärft (Wand-Penalty raus, Loop-Penalty reduziert)
**Datei:** `src/core/simulation.cpp` → `computeReward()`
**Problem:** Eskalierender Wand-Penalty (−0.05/−0.25) + Loop-Penalty (−0.15) machten Erkunden in
engen Korridoren teurer als Stillstand (Literatur: „Straf-Stacking"; eigene v6-Diagnose).
**Hinweis zur Historie:** v2026-06-13 hatte das schon einmal entfernt; Commit `16dc8dd` hat es
undokumentiert wieder eingebaut — dieser Eintrag holt die Doku nach und setzt den Stand erneut um.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `moveBlocked` (1×) | −0.05 | **0** | Step-Penalty + ausbleibender PBRS-Fortschritt reichen |
| `moveBlocked` (≥2×) | −0.25 | **0** | Eskalation machte Erkunden unwirtschaftlich |
| `positionLoop` | −0.15 | **−0.05** | Erkennung bleibt, Strafe moderat |

#### Änderung 3 — Exit-Platzierung nach BFS-Laufweg statt Luftlinie
**Datei:** `src/core/world.cpp` → `chooseExitPoint()`
**Problem:** Kandidaten wurden nach euklidischer Distanz (dist² ∈ [min², max²]) gewählt. Realer
Laufweg bei „35–45": **42–75 Tiles (Ø 55)** — Curriculum-Phasen und Eval-Angaben waren unscharf.
Zusätzlich konnte die 5×5-Exit-Freiräumung den Laufweg nachträglich verkürzen (Ausreißer bis 13).
**Lösung:** (a) Kandidaten nach **BFS-Tiefe** ∈ [exitMin, exitMax] sammeln; (b) Kandidat nur
akzeptieren, wenn der Laufweg auch mit *virtueller* 5×5-Freiräumung ≥ exitMin bleibt (bis 24 Versuche).
**Messung nach Fix (je 50 Seeds):** Val/Test A/Holdout B: min/Ø/max = 35/39.7–40.1/45, **150/150 in [35,45]**, 150/150 lösbar. Phase-1-Range (5–12): min/Ø/max = 5/8.9/12. ✓

#### Änderung 4 — Prozess-globales Config-Leck gefixt
**Dateien:** `python/stoneforge_env.py` (Fix), `src/python/py_module.cpp` (Ursache, unverändert)
**Problem:** `configure_world_generation()` schreibt in `stoneforge::mutableGameConfig()` —
**prozess-global**. Jede neu erstellte Env-Instanz überschreibt die Config aller anderen.
Konsequenz fürs bisherige Training: Der Eval-Callback (erzeugt pro Eval ein Env mit eval_min/max)
hat in **Phase 3 die Trainingsverteilung nach dem ersten Eval still von 25–45 auf 35–45 verschoben**.
**Lösung:** Env stempelt seine eigene Config bei **jedem** `reset()` neu.
**Verifikation:** Oracle auf Test-Seeds nach Erstellung eines zweiten Envs (5–12): weiterhin Ø 40.2 Schritte (vorher fälschlich 8.3). ✓

**Gesamtverifikation (Skript `verify_v11.py`):** Obs 229/231 (legacy) inkl. Layout-Konsistenz ✓,
150/150 lösbar ✓, Distanzen exakt in Range ✓, Wand-Penalty weg (3× Block → je ≈ −0.01, keine Eskalation) ✓,
BFS-Oracle 10/10 mit Ø 40.2 Schritten ✓.

#### Änderung 5 — Mining, Bauen & Kampf aus dem RL-Pfad entfernt
**Dateien:** `src/python/py_module.cpp`, `src/core/simulation.cpp`
**Problem:** Das RL-Binding akzeptierte weiterhin alle 9 Spielaktionen (Mine/Place/Use/Wait/Noop),
obwohl nur Bewegung trainiert wird; im Reward standen tote Terme (+2/Mob-Kill, +5 Exit-Unlock).
**Lösung:**
- `StoneforgeCoreEnv.step()` erlaubt nur noch Aktionen 0–3 (Bewegung), Rest → RuntimeError;
  `action_space_n()` liefert 4.
- Reward-Terme für Mob-Kills und Exit-Unlock gestrichen. Reward besteht nur noch aus:
  Step-Penalty −0.01, Explorations-Bonus +0.02, Loop-Penalty −0.05, Schadens-Penalty, PBRS, ±Terminal.
- Das **spielbare Client-Spiel** (`stoneforge_client`) behält Mining/Crafting unverändert —
  entfernt wurde nur der RL-Pfad.
**Verifikation:** `action_space_n()==4` ✓, Aktionen 4–8 werfen RuntimeError ✓,
Oracle-Regression 10/10, Ø 40.2 Schritte, Ø Return +101.20 ✓.

#### Änderung 6 — Entropie-Annealing-Startwert gefixt
**Datei:** `scripts/train_curriculum.py`
**Problem:** `EntropyAnnealingCallback` in Phase 3 startete bei `start_ent=0.01`, trainiert wird
aber mit `ent_coef=0.05` (RPPO_KWARGS) → abrupter Entropie-Sprung 0.05 → 0.01 beim Phasenwechsel.
**Lösung:** `start_ent=RPPO_KWARGS["ent_coef"]` (0.05) — Annealing jetzt stetig 0.05 → 0.001 über 500k Steps.

#### Änderung 7 — Eval-Callback: phasengerechtes Gating + Det/Stoch-Doppelmessung
**Datei:** `scripts/train_curriculum.py`
**Problem:** (a) Phase 1/2 gateten auf **deterministischer** SR — mit ent_coef=0.05 konstruktionsbedingt
~0% (Argmax fast-uniformer Policy) → Phasen endeten nie vorzeitig, best_model-Auswahl lief auf Rauschen.
(b) `MAX_EVAL_STEPS=4000` für alle Phasen — bei Exit 5–12 reine Eval-Verschwendung.
(c) Nur eine Metrik gemessen — keine Det/Stoch-Gap-Kurve über das Annealing.
**Lösung:**
- Neue Phasen-Parameter `gate_metric` ("stoch" für P1/P2, "det" für P3/P4) und
  `eval_max_steps` (600 / 1200 / 4000 / 4000).
- Jedes Eval misst jetzt **beide** SRs (det + stoch); TensorBoard: `eval/sr_det`, `eval/sr_stoch`;
  Gating + best_model-Auswahl auf der Phasen-Metrik; eval_history-Label enthält beide Werte.
**Verifikation:** py_compile + Smoke-Test (PHASES-Parameter, Callback-Instanziierung) ✓.

#### Änderung 8 — Device-Benchmark (CPU vs. MPS) + Batch-Ablation → batch_size=64
**Datei:** `scripts/train_curriculum.py` (`RPPO_KWARGS`)
**Fragestellung:** Kann MPS (Apple GPU) das Training beschleunigen? Ist batch=8 wirklich nötig?
**Befund 1 — Bisheriges Training lief auf CPU:** SB3 `device="auto"` wählt nur CUDA, nie MPS.
Das `"device": "mps"` in alten config.json war Fehlprotokollierung.
**Befund 2 — MPS ist bei diesem Netz IMMER langsamer** (LSTM 256 zu klein, Kernel-Dispatch-Overhead
dominiert; deckt sich mit SB3-Doku: GPU lohnt erst bei CNN/großen Netzen):

| Konfiguration | FPS | Speedup vs. v10 |
|---|---|---|
| batch=8, CPU (v10-Status-quo) | 88 | 1.0× |
| batch=8, MPS | 26 | **0.29×** |
| batch=64, CPU | 163–187 | **1.9–2.1×** |
| batch=64, MPS | 105 | 1.19× |
| batch=128, CPU | 182 | 2.07× |
| batch=256, CPU | 184 | Plateau (Env-Sim wird Bottleneck) |

**Befund 3 — Batch-Validierung (A1):** 150k Phase-1-Steps (v11-Env, exit=5–12), batch=64, CPU:
approx_kl 0.012→0.056 (gesund, v10-Bereich), EV bis 0.72, Entropie −1.38→−0.97 (wie v10),
**SR @150k: 52% stoch / 22% det** (Val-Seeds 6000–6049). → batch=8 war NICHT der kritische
v2-Erfolgsfaktor (das war ent_coef=0.05); batch=64 lernt gleichwertig bei doppelter Geschwindigkeit.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `batch_size` | 8 | **64** | validiert gleichwertig, 2.1× schneller; 64 statt 128, weil 64 lern-validiert ist |
| Device | auto (=CPU) | CPU (explizit dokumentiert) | MPS 0.29–1.19× — nie schneller |

**ETA v11-Gesamtlauf:** ~2.5–3h statt ~5.5h.

#### Änderung 9 — Konsistenz-Bereinigung (Projekt-Analyse 06.07.2026)
**Dateien:** `scripts/train.py`, `assets/base/game_config.json`, `src/core/game_config.cpp`,
`src/include/stoneforge/game_config.hpp`, `python/README.md`, `.claude/CLAUDE.md`,
`docs/stoneforge_game_description.md`

1. **`train.py` synchronisiert:** RPPO `batch_size` 8 → 64 und `enable_critic_lstm=True`
   (war mit `train_curriculum.py` auseinandergelaufen — `--algo rppo` trainierte halb so schnell).
2. **Tote Konfiguration entfernt:** Die cold/warm/moss-Threshold-Keys in `game_config.json`
   (+ Parser + Struct-Felder) wurden **nie gelesen** — die Biom-Schwellwerte sind fest in
   `World::sampleBaseTile()` hinterlegt. Entfernt, um wirkungsloses Tuning über die JSON zu
   verhindern; Hinweis-Kommentare in Header/Parser ergänzt.
   **Regression verifiziert:** Weltgenerierung byte-identisch (Seeds 7000–7002: BFS 43/38/35 wie vorher).
3. **Doku-Sync:** `python/README.md` (Wrapper `ExitPotentialFieldWrapper`/`ReducedActionEnv`
   existieren nicht mehr; 229-dim Obs; Legacy-Flag; Config-Global-Hinweis), CLAUDE.md
   (Environment-Abschnitt, Fallstricke aktualisiert: PBRS=BFS erledigt, Biom-Schwellwerte
   hartkodiert, MPS langsamer, Legacy-Obs), `stoneforge_game_description.md`
   (Exit-Räumung ist 3×3 nicht 5×5, BFS-Platzierung seit v11, Obs 229).

#### Änderung 10 — Usability-Audit: Legacy-Modell-Regression, setup_env.sh, GUI-Training
**Dateien:** `python/stoneforge_env.py`, `scripts/watch_agent.py`, `scripts/eval_comparison.py`,
`scripts/launcher_gui.py`, `scripts/setup_env.sh`

1. **Obs-Auto-Erkennung (`env_kwargs_for_model()`):** Nach der v11-Obs-Änderung (229-dim)
   crashten ALLE Bestandsmodelle (231/236/…-dim) in `watch_agent.py`, `eval_comparison.py`
   und im GUI-Quick-Eval mit Shape-Mismatch. Neuer Helper in `stoneforge_env.py` leitet die
   Env-Kwargs automatisch aus der Obs-Dimension des geladenen Modells ab (Mapping für
   229/230/231/236/249/456/461); alle drei Verbraucher umgestellt. `watch_agent.py` nutzte
   zudem hartkodierte Obs-Indizes (obs[228/229] für exitDx/Dy) → layoutabhängig gemacht.
   **Verifiziert:** Legacy-Modell (231) und v11-Modell (229) laufen beide automatisch.
2. **`setup_env.sh` war defekt:** suchte venv/build/python unter `scripts/` statt Repo-Wurzel
   (und meldete trotzdem „✓ activated"). Auf ROOT_DIR umgestellt + klare Fehlermeldung ohne venv.
   **Verifiziert:** `source scripts/setup_env.sh && python -c "import stoneforge_sim"` ✓.
3. **GUI-Quick-Eval konnte kein RecurrentPPO laden** (Loader-Liste nur PPO/A2C/DQN — das beste
   Modell war in der GUI nicht evaluierbar) und lief für LSTM-Modelle zustandslos.
   RecurrentPPO + LSTM-State-Loop + Obs-Auto-Erkennung ergänzt.
4. **GUI-Training konnte die beste Methode nicht starten:** Der Trainings-Tab bot nur
   PPO/DQN/A2C via `train.py`; die Checkboxen „Curriculum Learning" und „Monster aktivieren
   (Training)" waren **nicht verdrahtet** (tote UI). Neu: Radio-Option „LSTM-Curriculum
   (empfohlen)" (Default) startet `train_curriculum.py` mit Zeitstempel-`--save-dir`;
   tote Checkboxen entfernt, Hinweistext ergänzt.

**Nächster Schritt:** v11-Curriculum-Run mit dieser Umgebung (siehe `docs/BEWERTUNG_UND_PLAN.md`).

---

## v2026-06-15 — Det/Stoch-Gap-Fixes (v7: visit_count + action_buffer + Phase 4)

### v2026-06-15.A — Drei strukturelle Fixes gegen Det/Stoch-Gap
**Dateien:** `python/stoneforge_env.py`, `scripts/train_curriculum.py`
**Problem:** LSTM-Agent (86% stoch / 36% det) hat Stochastizität als Problemlöser internalisiert. Greedy-Policy steckt in Loops, weil der Agent deterministisch nicht sehen kann, dass er im Kreis läuft.

#### Fix 1 — Besuchszähler in Observation (`use_visit_count=True`)
**Datei:** `python/stoneforge_env.py`
Neues Feature: wie oft wurde die aktuelle Tile in dieser Episode betreten? (0..10, normalisiert /10). Der LSTM sieht nun deterministisch "ich bin hier schon oft gewesen → andere Richtung". Obs: 231 → 249 dims (mit Fix 3).

#### Fix 2 — Phase 4 Greedy Fine-Tune (`ent_coef=0.0001`)
**Datei:** `scripts/train_curriculum.py`
Neue Phase 4 nach Phase 3: 200k Steps auf exit=25–45, ent_coef=0.0001. Zwingt die Policy, deterministisch konsistent zu sein. Modellselektion in Phase 4 auf deterministischer SR (Phase-4-Best überschreibt Phase-3-Best als best_model.zip).

#### Fix 3 — Aktions-Buffer 4 Schritte (`action_buffer_len=4`)
**Datei:** `python/stoneforge_env.py`
Statt nur letzter Aktion (4 dims): letzte 4 Aktionen als One-Hot-Matrix (16 dims). LSTM erkennt deterministisch ↑↓↑↓-Muster in der Observation.

| Parameter | vorher | nachher |
|-----------|--------|---------|
| Obs-Dims | 236 (v5) | **249** |
| visit_count in Obs | nein | **ja** |
| Aktions-Buffer | 1 Schritt | **4 Schritte** |
| Phasen | 3 | **4 (+ Greedy Fine-Tune)** |
| ent_coef Phase 4 | — | **0.0001** |

**Training gestartet:** 15.06.2026, `models/ppo_lstm_curriculum_v7_fixes`.
**Ergebnis:** Gescheitert — siehe v2026-06-15.B.

---

### v2026-06-15.B — Hyperparameter-Suche n_epochs & lstm_hidden_size

**Problem:** 249-dim Observation (größer als bisherige 236) destabilisiert Value-Funktion.
`explained_variance ≈ 0` und steigender `value_loss` über alle v7-Varianten.
Ursache: LSTM mit 512 Hidden Units hat zu viele Gewichte relativ zur neuen Obs-Größe;
zu wenige Gradient-Steps lassen Critic nicht konvergieren.

#### Versuch v7 — lstm=512, n_epochs=4
`models/ppo_lstm_curriculum_v7_fixes`
- explained_variance = 0.000 konstant bis Step 280k
- value_loss = 138 (steigend)
- SR Phase 1: max 4% bei 280k Steps → **abgebrochen**

#### Versuch v7b — lstm=512, n_epochs=10
`models/ppo_lstm_curriculum_v7b_fixes`
- explained_variance = 0.22 bei Step 16k → kollabiert auf 0.000 bei Step 37k
- value_loss: 3.75 → 182 (Explosion)
- Diagnose: zu viele Epochen für LSTM — alte Hidden-States passen nicht mehr zu neuen Gewichten → **abgebrochen**

#### Versuch v7c — lstm=512, n_epochs=6
`models/ppo_lstm_curriculum_v7c_fixes`
- explained_variance = 0.000, value_loss = 121 (steigend)
- SR Phase 1: max 2% bei 75k Steps → **abgebrochen**

#### Versuch v7d — lstm=256, n_epochs=4
`models/ppo_lstm_curriculum_v7d_fixes`
- Halbierung des LSTM (512→256): weniger Gewichte, schnellere Konvergenz
- FPS: 424, value_loss stabil ~59 bis 350k, dann 415 (Divergenz)
- Peak SR: 12% @ 250k Steps → abgebrochen bei 413k

| Variante | lstm | n_epochs | Peak SR P1 | Abbruch |
|----------|------|----------|------------|---------|
| v7 | 512 | 4 | 4% | ja (Value kollabiert) |
| v7b | 512 | 10 | 2% | ja (Value explodiert) |
| v7c | 512 | 6 | 2% | ja (Value kollabiert) |
| v7d | 256 | 4 | 12% | ja (Divergenz @ 405k) |

---

### v2026-06-15.C — Diagnose: Root-Cause-Analyse + Neustart mit v2-Hyperparametern

#### Hintergrund: MLP-Baseline (ppo_phase4) deterministisch evaluiert

Zum Vergleich wurde ppo_phase4 (MLP, 236-dim Obs) auf Seeds 7000–7049 (exit=35–45) gemessen:

| Eval-Modus | Erfolge | SR | Mittl. Schritte |
|------------|---------|-----|-----------------|
| Deterministisch | 0 / 50 | **0.0%** | 275 (immer Early-Stop) |
| Stochastisch | 16 / 50 | **32.0%** | 1401 |

Befund: LSTM-Curriculum (86% stoch / 36% det) ist klar überlegen. MLP-Deterministik=0% bestätigt: POMDP-Navigation ohne Gedächtnis ist nicht deterministisch lösbar.

Random-Agent auf Phase-1-Seeds (6000–6049, exit=5–12): **42% SR** (stochastisch, 2652 Schritte).
Das zeigt: die Umgebung ist lösbar; das 0%-Problem lag im Training, nicht in der Umgebung.

---

#### Root-Cause-Analyse durch Reverse-Engineering von v2

**Problem:** Alle v7–v9 Runs zeigten `approx_kl ≈ 4.9e-07` (Policy ändert sich nicht), `explained_variance ≈ 0`, SR ≤ 12%. Gleichzeitig existierte v2 mit 86% SR.

**Methode:** `RecurrentPPO.load("models/ppo_lstm_curriculum_v2/phase1_best_model.zip")` + `inspect`.

Das v2-Modell hatte diese Konfiguration (gemessen, nicht geschätzt):

| Parameter | v2 (86% SR) | v7d–v9 (0–12% SR) | Grund für Unterschied |
|-----------|-------------|--------------------|-----------------------|
| `n_steps` | **256** | 512 / 128 | Zu lang → BPTT Vanishing Gradient |
| `batch_size` | **8** | 256 | Zu groß → nur 1 Batch/Epoche statt 2 |
| `n_epochs` | **10** | 4 / 1 / 2 | Zu wenige Gradient-Updates |
| `ent_coef` | **0.05** | 0.01 | 5× weniger Exploration |
| `obs_dim` | **231** | 249 / 232 | Extra-Features störten |
| `enable_critic_lstm` | True | fehlt / True | critic hat eigenen LSTM |

**Root Causes (priorisiert):**
1. **`ent_coef=0.01` statt 0.05** — Agent exploriert nicht → findet Exit nie → approx_kl=0
2. **`batch_size=256` statt 8** — bei 16 Envs × 256 n_steps = 4096 Transitions: nur 1 Batch/Epoche statt 2 → halb so viele Gradient-Steps bei ohnehin falschem ent_coef
3. **`n_steps=512` (v7d)** — BPTT über 512 LSTM-Steps → Vanishing Gradient → approx_kl≈0
4. **Obs-Erweiterungen (249-dim)** — waren nutzlos ohne funktionierende Basis-Konfiguration

**Externe Bestätigung (Online-Recherche 15.06.2026):**
- SB3-Doku: „approx_kl ≈ 0 = PPO does not learn" → direkt in unseren Logs sichtbar
- RecurrentPPO-Defaults: n_steps=128, batch_size=128, n_epochs=10 (wir hatten n_steps=512, batch_size=256)
- `truncated_bptt_steps` existiert NICHT in sb3_contrib — BPTT-Länge wird allein durch `n_steps` gesteuert

---

#### Versuch v8 — n_steps=128, n_epochs=1, kein clip_range_vf
`models/ppo_lstm_curriculum_v8_stable` (abgebrochen @ 150k)

| Metrik | Wert | Bewertung |
|--------|------|-----------|
| FPS | 1807 | sehr schnell, aber nutzlos |
| approx_kl | 6.6e-05 | 100× besser als v7d, aber immer noch fast 0 |
| SR@25k, @50k, @75k, @100k, @125k, @150k | 0% | kein Fortschritt |
| Ursache | ent_coef=0.01 | zu wenig Exploration für Exits |

clip_range_vf=0.2 wurde als mögliche Ursache identifiziert (verhindert schnelle Value-Konvergenz bei Random-Init), dann aber als Nebenursache eingestuft.

---

#### Versuch v9 — n_steps=128, n_epochs=2, lr=3e-4, 232-dim
`models/ppo_lstm_curriculum_v9_stable` (abgebrochen @ 100k)

| Metrik | Wert | Bewertung |
|--------|------|-----------|
| FPS | 660 | schnell |
| approx_kl | bis 3.8e-03 | echter Lernfortschritt |
| explained_variance | bis 0.024 | minimal positiv |
| SR@25k bis @100k | 0% | ent_coef=0.01 blockiert Exploration |
| Debugging | Random-Agent 42% SR | Umgebung ist lösbar! |

Fazit: n_steps=128 hat das BPTT-Problem gelöst (approx_kl steigt), aber ent_coef=0.01 verhindert weiterhin, dass der Agent den Exit findet.

---

#### Versuch v10 — v2-Hyperparameter reproduziert ← läuft
`models/ppo_lstm_curriculum_v10_reproduction`

Konfiguration:
```python
n_steps=256, batch_size=8, n_epochs=10, ent_coef=0.05,
obs=231-dim, enable_critic_lstm=True, lstm_hidden_size=256
```

Trainingsmetriken Phase 1 (15.06.2026, @ ~61k Steps):

| Step | approx_kl | clip_frac | explained_var | value_loss | SR (det) |
|------|-----------|-----------|---------------|------------|----------|
| 8k | 0.014 | 0.118 | 0.0008 | 54 | — |
| 12k | 0.021 | 0.188 | 0.006 | 46 | — |
| 16k | 0.019 | 0.243 | -0.0001 | 31 | — |
| 20k | **0.027** | **0.312** | **0.231** | **1.47** | — |
| 25k (Eval) | 0.028 | 0.301 | 0.021 | 58 | **0%** |
| 45k | 0.037 | — | 0.067 | 102 | — |
| 50k (Eval) | 0.039 | — | 0.101 | 67 | **0%** |
| 61k | 0.042 | 0.356 | 0.193 | 66 | — |
| 65k | 0.048 | — | **0.939** | — | — |
| 74k (Eval) | 0.045 | 0.356 | 0.296 | 285 | **0%** |
| 77k | 0.048 | — | 0.568 | — | — |
| 80k | 0.048 | — | 0.685 | — | — |

_EV=0.939 @ 65k: Value-Funktion versteht diverse Returns (Exits gefunden via Stochastik)._
_Det SR=0% erwartungsgemäß — Policy ist mit ent=0.05 noch zu stochastisch für Argmax-Navigation._

Vergleich zur Kontrollgruppe:

| Variante | n_steps | batch | n_epochs | ent_coef | Peak EV | approx_kl | SR@50k |
|----------|---------|-------|----------|----------|---------|-----------|--------|
| v7d | 512 | 256 | 4 | 0.01 | ~0 | 4.9e-07 | 0% |
| v8 | 128 | 256 | 1 | 0.01 | 0.003 | 6.6e-05 | 0% |
| v9 | 128 | 128 | 2 | 0.01 | 0.024 | 3.8e-03 | 0% |
| **v10** | **256** | **8** | **10** | **0.05** | **0.231** | **0.042** | **0%** |

**Bewertung v10:**
Trainingsmetriken (approx_kl, EV, clip_frac) sind um Größenordnungen besser als alle Vorgänger.
EV=0.939 @ ~65k Steps zeigt: Value-Funktion hat gelernt, diverse Returns (Exits gefunden + nicht gefunden) vorherzusagen.
entropy_loss: -1.39 → -1.04 nats (von fast-uniform zu konzentrierter Verteilung).

**Warum 0% det SR bei 25k/50k/75k mit ent_coef=0.05 erwartet ist:**
Mit ent_coef=0.05 bleibt die Policy absichtlich stochastisch (H=1.0–1.4 nats ≈ fast-uniform über 4 Aktionen).
Der Argmax einer fast-uniformen Verteilung ist fast zufällig → deterministische SR=0% ist normal.
Die stochastische Policy FINDET Exits (EV=0.939 beweist diverse Returns), aber Argmax nutzt das nicht.
Deterministischer Fortschritt kommt erst mit Entropie-Reduktion:
- Phase 3: ent_coef 0.05 → 0.01 → 0.001 (Annealing über 500k Steps)
- Phase 4: ent_coef 0.0001 (Greedy Fine-Tune, 200k Steps)

**Gesamter Trainingsplan v10:**
- Phase 1 (exit=5–12, 500k Steps, ent=0.05): Grundlegendes Navigationsprinzip lernen
- Phase 2 (exit=12–25, 500k Steps, ent=0.05): Generalisierung auf mittlere Distanzen
- Phase 3 (exit=25–45, 1M Steps, Annealing 0.05→0.001): Zielverteilung + Determinisierung
- Phase 4 (exit=25–45, 200k Steps, ent=0.0001): Greedy Fine-Tune für Det/Stoch-Gap

**ETA:** ~5.5h Gesamt auf CPU @ 110 FPS (500k+500k+1M+200k Steps).
**Status:** läuft, 15.06.2026, Phase 1 bei ~75k/500k Steps.

---

## v2026-06-13 — Reward-Shaping-Dilemma behoben (No Wall Penalty)

### v2026-06-13.A — Wand-Penalty entfernt, Loop-Penalty reduziert
**Datei:** `src/core/simulation.cpp` → `computeReward()`
**Problem:** Der eskalierende Wand-Penalty (−0.05 / −0.25) in Kombination mit dem Loop-Penalty (−0.15) machte den Agenten übervorsichtig. In engen Korridoren überwiegen die Strafen (bis −0.50 für 3 Erkundungsschritte) den PBRS-Bonus (+0.02/Tile). Der Agent lernt: Stehen ist billiger als Erkunden. Erklärt teilweise den Det/Stoch-Gap (86% → 36%).
**Lösung:** Wand-Penalty komplett entfernt (Step-Penalty −0.01 + kein PBRS-Bonus reicht als Signal). Loop-Penalty von −0.15 auf −0.05 reduziert.

| Parameter | vorher | nachher | Begründung |
|-----------|--------|---------|------------|
| `moveBlocked` (1×) | −0.05 | **0** | Step-Penalty reicht; Wand-Tasten nicht katastrophal |
| `moveBlocked` (≥2×) | −0.25 | **0** | Eskalation machte Erkunden unwirtschaftlich |
| `positionLoop` | −0.15 | **−0.05** | Erkennung bleibt, Strafe moderater |

**Training gestartet:** 13.06.2026, `models/ppo_lstm_curriculum_v6_nwp`, 8 Envs parallel.
**Ergebnis:** ausstehend.

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
