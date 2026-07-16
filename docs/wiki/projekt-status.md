---
id: projekt-status
title: Aktueller Projektstand — was läuft, was offen ist
type: project
tags: [status, aufgaben]
related: [v12-final, zielkriterium, testset-leakage, nohup-training, reproduzierbarkeit]
status: läuft
updated: 2026-07-17
---

# Projektstand (17.07.2026)

> Der einzige Eintrag mit Verfallsdatum. Nach jeder Statusänderung hier nachziehen — oder ihn
> löschen, wenn die Arbeit abgegeben ist.

## Läuft gerade: Aufstockung auf n=7

Vier zusätzliche Seeds: `models/ppo_lstm_curriculum_v12_s{4,5,6,7}`, Logs
`logs/train_v12_extra/s{4,5,6,7}.log`. Gestartet **15.07. ~21:14** mit `nohup` +
`--no-live-map --no-heatmap`, `OMP_NUM_THREADS=2`.

**Grund:** Bei n=3 ist der Abstand zum Ziel A (2,7 pp) kleiner als die Streuung (9,5 pp) →
"robust über 70 %" war nicht belegbar ([[v12-final]]). n=7 verkleinert den Standardfehler.

**Stand 17.07. 00:45** — alle vier Prozesse leben, alle in **Phase 3**:

| Lauf | Steps | letzte Eval (det / stoch) |
|------|-------|---------------------------|
| s4 | 1.42 M | 8 % / 28 % |
| s5 | 1.12 M | 28 % / 60 % |
| s6 | 1.35 M | 28 % / 54 % |
| s7 | 1.40 M | 10 % / 20 % |

⚠️ **Zwei Beobachtungen, die Aufmerksamkeit brauchen:**

1. **Deutlich langsamer als erwartet.** Angesetzt waren 8–12 h, inzwischen sind es ~27 h Wall-Clock
   bei nur ~11 h CPU-Zeit je Prozess (≈40 % Auslastung). Verdacht: Der Mac war zwischendurch im
   Ruhezustand — `caffeinate` hat sich schon bei [[e3-lstm512]] selbst terminiert. Prüfen.
2. **s4 und s7 sehen schwach aus** (stoch 20–28 % mitten in Phase 3). Das sind Momentaufnahmen, keine
   Kurven — laut [[batch-size-8]]-Lehre also **nicht** vorschnell bewerten. Aber falls sie so
   enden, sinkt der n=7-Schnitt gegenüber n=3 spürbar.

**Nächster Schritt nach Abschluss:** Standard-Eval A+B (Cap 4000, det+stoch) auf alle 7 Seeds,
Mittel ± Std mit `ddof=1`, dann CHANGELOG + Doku aktualisieren.

## ⚠️ Folge für die Doku, wenn n=7 fertig ist

`docs/Projektdokumentation.tex` ist **bereits mit n=3 ausformuliert** (A 73,3 / B 80,0 + CI) —
in Zusammenfassung, Fazit, Gap-These und Related Work. Diese Headline-Zahlen müssen auf n=7 neu
gerechnet und **überall** nachgezogen werden. Die Stellen sind im CHANGELOG-Eintrag vom 09.07.
gelistet.

Und: Die historische 86 %-Angabe in der Ablationstabelle **bleibt bewusst stehen** (mit
Klarstellung in der Caption) — siehe [[v11-env-bruch]]. Nicht "aufräumen".

**Wenn n=7 das Ziel A verfehlt:** Das ist kein Beinbruch, sondern ein Ergebnis. Dann führt man B
(Holdout, Ziel ≥60 %) als Hauptzahl — was ohnehin die sauberere Wahl ist ([[testset-leakage]]) —
und berichtet A ehrlich mit Streuung. Vorher überlegen, nicht hinterher.

## Offen

- **[[testset-leakage]] (P0.1)** — bekannt, nicht behoben. Im Methodikteil offen benennen.
- **Doku-Feinschliff** — inhaltlich rund (25 S., kompiliert exit 0), fehlt: eigene Durchsicht.
- **Optional:** Bootstrap-95%-CIs statt ±Std ([[reproduzierbarkeit]]).

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
