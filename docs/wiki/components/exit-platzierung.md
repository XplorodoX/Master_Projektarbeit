---
id: exit-platzierung
title: Exit-Platzierung — die Lösbarkeitsgarantie
type: component
tags: [spiel, worldgen, exit, bfs]
path: src/core/world.cpp (~Z. 328–410)
verified: 2026-07-17
related: [weltgenerierung, stoneforge-spiel, v11-env-bruch, doku-worldgen-veraltet]
updated: 2026-07-17
---

# Exit-Platzierung

Der Schritt, an dem die Spielbarkeit hängt. `world.cpp`, ~Z. 328–410.

## Das Verfahren

Eine **BFS-Traversierung vom Spawn aus** sammelt alle Zellen, deren **Pfadlänge** in
`[exitMinDistance, exitMaxDistance]` = **[35, 45]** liegt. Aus diesen Kandidaten wird der Exit
gewählt (Freiradius 1).

Die Suche bricht ab, sobald der Ring überschritten ist — der Code vermerkt:
*"tiefer expandieren lohnt nicht — BFS-Tiefe wächst monoton"* (Z. 373).

## Warum BFS statt Luftlinie — der teuerste Bugfix des Projekts

Der Kommentar im Code (Z. 348) erklärt es selbst:

> "Kandidaten nach BFS-Pfadlänge statt Luftlinie wählen: exitMin/MaxDistance bedeutet damit echten
> Laufweg vom Spawn. Vorher (dist² in [min², max²]) streute der reale Pfad bei »35–45« auf
> **42–75 Tiles**."

Die alte Version maß die **Luftlinie**. Um Wände herum wird der echte Weg aber länger — teils fast
doppelt so lang. Die Aufgabe war also unbemerkt **erheblich schwerer als deklariert**: "Exit-Distanz
35–45" hieß in Wahrheit "Laufweg 42–75".

Dieser Fix ist der Kern des Umgebungsbruchs v11 — mit der Folge, dass **derselbe Seed vorher und
nachher eine andere Welt erzeugt**. Details und die Konsequenzen: [[v11-env-bruch]].

## Das ist zugleich die Lösbarkeitsgarantie

Der entscheidende Nebeneffekt: **Wer den Exit per BFS vom Spawn aus findet, hat bewiesen, dass ein
Weg existiert.** Die Lösbarkeit fällt aus der Konstruktion, statt nachträglich geprüft oder
erzwungen zu werden.

Deshalb sind alle Krücken abgeschaltet ([[weltgenerierung]]): der Manhattan-Carve
(`forceGuaranteedPath = false`), sein Fallback (`guaranteedPathFallback = false`) und die
FloodFill-Validierung. Sie lösen ein Problem, das nicht mehr auftreten kann.

Das ist der elegantere Entwurf: Statt eine womöglich unspielbare Welt zu erzeugen und sie dann zu
reparieren, wird nur eine Welt erzeugt, die per Konstruktion spielbar ist.

## Ein subtiles Detail: die virtuelle BFS

Ab Z. 403 steht ein Hinweis auf eine **BFS mit virtueller** Behandlung — der Grund: Nachträgliche
Eingriffe (etwa Freiradien um Spawn und Exit) können den Weg **nachträglich unter `exitMinDistance`
verkürzen**. Wer den Ring vor dem Freischneiden misst, misst die falsche Welt.

Ein gutes Beispiel dafür, dass "erst generieren, dann aufräumen" die Invarianten brechen kann, die
man vorher etabliert hat.
