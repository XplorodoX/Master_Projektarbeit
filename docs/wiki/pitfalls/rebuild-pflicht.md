---
id: rebuild-pflicht
title: Fallstrick — Rebuild nach C++- oder Config-Änderung
type: pitfall
tags: [fallstrick, build, betrieb]
related: [cpp-core, config-prozessglobal, reproduzierbarkeit]
updated: 2026-07-17
---

# Rebuild-Pflicht

**Nach jeder Änderung an C++-Code oder an `assets/base/game_config.json` ist ein Rebuild nötig.**

```bash
cmake --build build -j
```

**Symptom bei Vergessen:** Crash — oder, schlimmer, **falsche Ergebnisse ohne Fehlermeldung**. Die
`game_config.json` wird zur Buildzeit eingebunden; das Python-Env liest fröhlich die alte
kompilierte Version weiter, während die JSON-Datei etwas anderes behauptet.

Besonders relevant bei `observationRadius` — eine Änderung dort ändert die Obs-Shape. Ohne Rebuild
passen Modell und Env scheinbar zusammen, meinen aber verschiedene Welten.

## Warum das für die Arbeit zählt

Der C++-Build ist **Teil der Umgebung**, nicht nur Werkzeug. Ein Experiment ist damit nur
reproduzierbar, wenn man **Git-Commit-Hash + `game_config.json`-Stand** kennt. Deshalb stempelt
`doc_logger.save_run_config` seit 09.07.2026 den Short-Hash (mit `-dirty`-Marker) in jede
`config.json` — siehe [[reproduzierbarkeit]].

Der `-dirty`-Marker ist wichtiger, als er aussieht: Er verrät genau den Fall "Code geändert, nicht
committet, Lauf gestartet" — also den Lauf, den später niemand mehr rekonstruieren kann.
