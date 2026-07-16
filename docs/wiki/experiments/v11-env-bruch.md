---
id: v11-env-bruch
title: Env v11 — der Env-Bruch (Breaking Change)
type: experiment
tags: [env, breaking-change, v11, vergleichbarkeit]
related: [observation-space, pbrs-reward-shaping, config-prozessglobal, ablation-abc, v12-final]
status: bestätigt
updated: 2026-07-17
---

# Env v11 (06.07.2026) — Breaking Change

Der wichtigste Schnitt im Projekt. **Alle Zahlen davor und danach sind unvergleichbar.**
CHANGELOG `v2026-07-06`, Commit `9a6b95d`.

## Was sich geändert hat

1. **Exit-Platzierung nach BFS-Laufweg** statt Luftlinie. Vorher hieß "Exit-Distanz 35–45" real
   ein Laufweg von **42–75** Feldern — die Aufgabe war unbemerkt schwerer als deklariert. Jetzt
   heißt 35–45 auch 35–45.
2. **Wand-Penalty entfernt**, Loop-Penalty −0.15 → **−0.05** (Straf-Stacking entschärft,
   siehe [[pbrs-reward-shaping]]).
3. **Tote Features raus:** Energie + Inventar aus der Observation (231 → **229**), nachdem
   Mining/Kampf aus dem Binding entfernt wurden. HP bleibt.
4. **Config-Leck gefixt** → [[config-prozessglobal]]. Der Eval-Callback verschob still die
   Phase-3-Trainingsverteilung auf 35–45. Das lief lange unbemerkt mit.
5. `forceGuaranteedPath` ist **redundant** geworden: Die BFS-Exit-Platzierung garantiert
   Lösbarkeit von selbst.

## Die Konsequenz für die Arbeit

**Derselbe Seed erzeugt vor und nach `9a6b95d` eine andere Welt.** Bestätigt am 15.07.2026.

Daraus folgt eine harte Regel: Die historischen 86 % ([[ablation-abc]]) und die v12-Zahlen
([[v12-final]]) dürfen **nie in dieselbe Tabelle**. Das ist kein Formalismus — es wäre schlicht
ein Vergleich zweier verschiedener Aufgaben.

Praktisch belegt: Das alte 86-%-Modell liefert gegen Env v11 nur 68 % stoch / 18 % det. Das ist
**kein Regressionsschaden**, sondern ein anderes Testset.

## Umgang in der Doku

Die Ablationstabelle behält die historischen Zahlen **mit Klarstellungssatz in der Caption**
(Referenzlauf vor v11 + Verweis auf die v12-Tabelle). Die 11 weiteren Vorkommen der 86 % im
Etappen-Narrativ, in der Zeitleiste und in der Root-Cause-Tabelle bleiben bewusst stehen — dort
sind sie korrekt **historisch** gemeint. Die Headline-Zahlen dagegen sind durchgängig v12.
