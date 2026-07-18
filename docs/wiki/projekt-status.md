---
id: projekt-status
title: Aktueller Projektstand — was läuft, was offen ist
type: project
tags: [status, aufgaben]
related: [v12-final, zielkriterium, testset-leakage, nohup-training, reproduzierbarkeit]
status: läuft
updated: 2026-07-18
---

# Projektstand (18.07.2026)

> Der einzige Eintrag mit Verfallsdatum. Nach jeder Statusänderung hier nachziehen — oder ihn
> löschen, wenn die Arbeit abgegeben ist.

## Aufstockung auf n=7 — ABGESCHLOSSEN (17.07., 09:32)

Vier zusätzliche Seeds: `models/ppo_lstm_curriculum_v12_s{4,5,6,7}`, Logs
`logs/train_v12_extra/s{4,5,6,7}.log`. Gestartet **15.07. ~21:14**, alle vier **komplett
durchgelaufen** (4 Phasen, kein Absturz), `best_model.zip` je Lauf vorhanden.

**Grund:** Bei n=3 ist der Abstand zum Ziel A (2,7 pp) kleiner als die Streuung (9,5 pp) →
"robust über 70 %" war nicht belegbar ([[v12-final]]). n=7 verkleinert den Standardfehler.

**Trainings-Evals (Val-Seeds 6000–6049, NICHT Testset A/B):**

| Lauf | Dauer | P1 | P2 | P3 (det) | P4 (det) | git |
|------|-------|----|----|----------|----------|-----|
| s4 | 36h 12m | 0,94 | 0,84 | 0,40 | **0,42** | `3c8fa26-dirty` |
| s5 | 36h 18m | 0,72 | 0,70 | 0,32 | 0,28 | `3c8fa26-dirty` |
| s6 | 29h 32m | 0,82 | 0,88 | 0,38 | 0,36 | `3c8fa26-dirty` |
| s7 | 36h 13m | 0,90 | 0,76 | 0,32 | **0,26** | `3c8fa26-dirty` |

Die det-Werte liegen im selben Band wie s1–s3 (0,26–0,38) → **der Det/Stoch-Gap bleibt auch bei
n=7** ([[det-stoch-gap]]). Nur s4 und s6 erreichten das P1-Gate nicht bzw. knapp — ohne Folge für
den Endstand.

⚠️ **Alle vier auf `3c8fa26-dirty`, s1–s3 auf ~`97ab30d`** → das n=7-Mittel mischt zwei
Code-Stände. Vor der Aggregation verifizieren (die offenen Änderungen vom 15.07. waren
`launcher_gui.py` + `render_engine.cpp` = Client-Dateien, die laut Build-Graph nicht in
`stoneforge_sim.so` landen, [[cpp-core]]) und im CHANGELOG festhalten — nicht annehmen.

**Laufzeit: 29,5–36,3 h statt der erwarteten 8–12 h.** Ursache geklärt (Nutzerangabe 17.07.):
Der Laptop war zugeklappt, die Läufe liefen deshalb gedrosselt. **Kein `caffeinate`-Problem**
(das war eine falsche Vermutung in der ersten Fassung dieses Eintrags) und **kein
Qualitätsmangel**: Training ist step-basiert und das Curriculum leistungsbasiert
([[curriculum-learning]]) — die Drosselung kostet Wall-Clock, nicht Lernschritte. Die Läufe sind
mit s1–s3 vergleichbar.

⚠️ **Dritter Punkt, gefunden beim Faktencheck 17.07.:** `v12_s4/config.json` trägt
`_git_commit = 3c8fa26-dirty`, die Läufe s1–s3 entstanden auf ~`97ab30d`. **Ein n=7-Mittel mischt
damit zwei Code-Stände**, einen davon mit uncommitteten Änderungen. Entwarnung ist wahrscheinlich,
aber sie muss *ausgesprochen* werden: Die beiden offenen Änderungen vom 15.07. betreffen
`launcher_gui.py` und `render_engine.cpp` — beides **Client-Dateien**, die laut Build-Graph nicht in
`stoneforge_sim.so` landen ([[cpp-core]]). Vor der n=7-Aggregation kurz verifizieren und im
CHANGELOG festhalten, sonst ist es eine unbelegte Annahme.

**Erledigt seit 17./18.07.:** Standard-Eval A+B auf allen 7 Seeds gelaufen (Ergebnis: Ziel A
verfehlt, [[v12-final]]), Doku auf n=7 umgeschrieben (CHANGELOG `v2026-07-17`), `ddof=1`
vereinheitlicht und n=3-Reste beseitigt (CHANGELOG `v2026-07-18`). Die historische 86 %-Angabe in
der Ablationstabelle **bleibt bewusst stehen** (mit Klarstellung in der Caption) — siehe
[[v11-env-bruch]]. Nicht "aufräumen".

## Offen

- **Doku-Feinschliff** — inhaltlich rund (27 S., kompiliert exit 0), fehlt: eigene Durchsicht.
- **Betreuer-Mail versenden** (`docs/betreuer_mail_entwurf.md`) — Kriterienrevision + verfehltes
  Ziel A vor Abgabe besprechen.
- **Formalia der Hochschule prüfen:** Deckblatt, eidesstattliche Erklärung, Verzeichnisse; Autor
  „Laurin" hat noch keinen Nachnamen auf der Titelseite.
- Bootstrap-CI für die n=7-Tabelle (aktuell t-basiert; nice-to-have).

## Erledigt (18.07., 4. Runde) — Struktur-Straffung, CHANGELOG `v2026-07-18.4`

- **Kriterienrevision** aus §1.1 in die Methodik verschoben (§1 behält kompakte Offenlegung +
  Vorwärtsverweis — die HARKing-Absicherung bleibt vorn). **Neuer Abschnitt „Iterative
  Modellverbesserung"** in der Methodik (drei Schlüsselbefunde: ent_coef, batch 8, Eval-Cap).
  Vom vorgeschlagenen Voll-Umbau bewusst **nicht** übernommen: Kapitel-4/6-Verschmelzung,
  DQN-Streichung. Doku: 30 S., exit 0, Referenzen sauber.

## Erledigt (18.07., 3. Runde) — redaktionelle Politur, CHANGELOG `v2026-07-18.3`

- Zusammenfassung mit Relevanz-Aufhänger (verfehltes Ziel A bleibt prominent — „Erfolgs-Framing"
  bewusst nicht übernommen), Singh-1994 in die Grundlagen vorgezogen (Gap = geprüfte Hypothese),
  Fazit mit `\paragraph`-Struktur inkl. **Technische Leistung** und **Einschränkungen (Threats to
  Validity)**, Etappe 3 mit Symptom/Root-Cause/Ergebnis-Lead-ins.

## Erledigt (18.07., 2. Runde) — Gutachter-Punkte, CHANGELOG `v2026-07-18.2`

- **Baseline-Versprechen aufgelöst:** DQN/A2C-Ankündigung aus den Grundlagen raus, Absatz
  „Algorithmenwahl" in der Methodik rein (DQN-Anlauf + Wechselgrund), fehlende DQN/A2C-Baseline
  auf v11 als **Grenze (5)** im Fazit ausgewiesen.
- **Related Work ausgebaut** (Gedächtnis-Methoden: DRQN/R2D2/POPGym/Pleines; Curriculum &
  Exploration: Bengio/PLR/ICM/RND). Neue Bib-Einträge `kapturowski2019r2d2`, `morad2023popgym`.
- **Neue Abbildung `fig:v12kurven`** (2×2 je Phase, 7 Läufe + Ensemble-Mittel det/stoch) via
  `scripts/plot_v12_learning_curves.py`; alte Lernkurve als historischer Referenzlauf
  gekennzeichnet. ⚠️ Fallstricke beim Plotten (sr-Feld = Gating-Metrik; Schrittzähler) im
  CHANGELOG dokumentiert.

## Erledigt (18.07.) — Doku-Konsistenzpass, CHANGELOG `v2026-07-18`

- **`ddof` vereinheitlicht** auf `ddof=1`: n=3-Zwischenauswertung im Text jetzt A 73,3 ± 8,3 /
  B 80,0 ± 8,0 → [[eval-protokoll]].
- **n=3-Reste in der Doku beseitigt:** Evaluationsprotokoll („drei Läufe (Seeds 1–3)" → sieben),
  Reproduzierbarkeits-Checkliste (Anzahl Läufe, Mittelung), Rechenaufwand (8 h + Hinweis auf die
  gedrosselten 29–36-h-Läufe), Titeldatum 06.07. → 18.07.
- **Pleines-Korrektur war bereits drin:** Die Doku enthält den Formulierungsvorschlag aus
  [[doku-check-2026-07-17]] wortgleich (jetzt Z. ~861). Kein Handlungsbedarf mehr.
- **Code-Stand-Mix bei n=7:** war bereits am 17.07. verifiziert (Diff über den Simulationspfad =
  nur `doc_logger.py`; CHANGELOG `v2026-07-17`, Alternativerklärung 2) — stand hier fälschlich
  noch als offen.

Nicht mehr offen: Das **Testset-Leakage ist seit 11.06. behoben** ([[testset-leakage]]) und die
**Bootstrap-CIs sind längst in der Doku** — beides stand hier fälschlich als offen.

## Erledigt (15.07.)

- Branch `vps/lstm-curriculum-v2` via PR #8 nach `main` gemergt (`3c8fa26`).
- **Spieländerungen berühren das RL-Env nicht** — über den Build-Graph bewiesen ([[cpp-core]]).
- Zwei Bugs gefixt: `launcher_gui.py` IndentationError (Launcher startete seit 23.06. nicht) und
  `render_engine.cpp` raylib-6.0-API-Bruch. **Noch nicht committet.**
- v12-Eval unabhängig reproduziert: A det exakt identisch, stoch im Rauschen. Offen: B det je Seed
  1 Punkt niedriger als am 08.07., Ursache ungeklärt.

## Werkzeug-Hinweis

`gh` CLI ist installiert (2.96.0), aber **nicht eingeloggt** — `gh auth login` verlangt Scope
`read:org`, den das Keychain-Token nicht hat. Workaround: Token je Befehl als `GH_TOKEN`
durchreichen (`git credential fill` → `password=`); für `gh pr create` reicht `repo`.
