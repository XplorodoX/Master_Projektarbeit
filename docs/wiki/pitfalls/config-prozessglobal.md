---
id: config-prozessglobal
title: Fallstrick — die WorldGen-Config ist prozessglobal
type: pitfall
tags: [fallstrick, cpp, env, still-verfaelschend]
related: [cpp-core, stoneforge-env, v11-env-bruch, demo-und-visualisierung, testset-leakage]
updated: 2026-07-17
---

# WorldGen-Config ist prozessglobal (C++)

Der heimtückischste Fallstrick im Projekt, weil er **nichts kaputt macht** — er verschiebt still
die Ergebnisse.

**Fakt:** Die WorldGen-Config lebt als **prozessglobaler Zustand im C++-Core**. Wer sie setzt,
setzt sie für **alle** Konsumenten im selben Prozess: Trainings-Env, Eval-Callback, Client.

## Der Schaden, den das angerichtet hat (bis v11)

Der Eval-Callback setzte die Exit-Distanz auf 35–45 (seinen Eval-Bereich) — und verschob damit
**still die Phase-3-Trainingsverteilung** mit. Das Training lief plötzlich auf einer anderen
Verteilung als deklariert. Kein Fehler, kein Crash, nur falsche Zahlen. Gefixt mit
[[v11-env-bruch]].

**Die Lösung:** `StoneforgeWorldEnv` stempelt die Config bei **jedem `reset()`** neu. Damit gewinnt
immer der, der zuletzt resettet — und das ist definitionsgemäß der, der gerade läuft.

## Wann es dich trifft

1. **C++-Binding direkt ohne Env-Wrapper benutzen** → Config selbst stempeln, sonst erbst du, was
   der Vorgänger hinterlassen hat.
2. **Demo im echten Client** → Der Client nutzt die globale Config mit Exit-Distanz **35–45**. Das
   Env **muss** `exit_min/max=35/45` setzen, sonst laufen Client-Welt und Agent-Welt auseinander
   und der Agent irrt sinnlos durchs Fenster. Genau das ist bei der Demo-Suche bei 12–22 passiert
   ([[demo-und-visualisierung]]).
3. **Mehrere Envs/Evals im selben Prozess** → immer prüfen, wer zuletzt gestempelt hat.

## Merkregel

> Wenn Welt und Erwartung auseinanderlaufen, ohne dass etwas abstürzt: **zuerst hier nachsehen.**
