---
id: demo-und-visualisierung
title: Demo, Live-Map und Visualisierung
type: component
tags: [demo, praesentation, tooling]
path: scripts/demo_agent.py, scripts/live_map_server.py, scripts/launcher_gui.py
related: [config-prozessglobal, stoneforge-env, projekt-status]
updated: 2026-07-17
---

# Demo und Visualisierung

## `demo_agent.py` — die Vorführung

Spielt den Agenten im echten C++-Client vor. **Reine Policy, kein BFS-Fallback** — anders als
`watch_agent.py`, das bei Feststecken das Orakel erzwingt. Für eine ehrliche Vorführung ist genau
das der Unterschied: `watch_agent.py` zeigt, was das Spiel kann, `demo_agent.py` zeigt, was der
Agent kann.

Reproduzierbare Erfolgsrunden über feste torch/np-Seeds:

```python
REPRO_RUNS = [(7025, 52), (7037, 55), (7000, 4), (7028, 60)]  # (welt_seed, sampling_seed)
```

144–280 Schritte je Runde. `--free <seeds>` für echtes Zufalls-Sampling, `--speed`,
`--deterministic`.

**Warum Repro-Runden statt Live-Zufall:** Die stochastische Policy ist erfolgreich, aber
ineffizient — viel Zickzack, selbst kurze Distanzen brauchen 144+ Schritte, und die Länge streut
stark. Für eine Vorführung mit Publikum ist das zu unzuverlässig.

> ⚠️ **Kritischer Fallstrick (gelöst):** Der C++-Client nutzt die prozessglobale WorldGen-Config
> mit Exit-Distanz **35–45**. Das Env **muss** `exit_min/max=35/45` nutzen, sonst sind Client-Welt
> und Agent-Welt verschieden und der Agent läuft im Fenster sinnlos umher. In `demo_agent.py` fest
> verdrahtet. Hintergrund: [[config-prozessglobal]].

## `videos/stoneforge_demo.gif`

Backup-Video einer Erfolgsrunde (Seed 7037, 188 Schritte, 6,4 MB) mit Erfolgs-Banner. Aufgenommen
via `screencapture -R` Frame-für-Frame + PIL-GIF (kein ffmpeg vorhanden). Der Client-Reset bei
Zielkontakt ist sofort — der letzte saubere Frame zeigt den Agenten **am** Portal (Goal distance 2),
nicht darauf.

## Live-Map

`live_map_server.py` (WebSocket) + `ws_map.html`, uPlot, Viridis, det/stoch-SR im Verlauf.
Standard-Port 8766 — bei parallelen Läufen kollidiert er, deshalb dort `--no-live-map`.

## `launcher_gui.py`

Haupteinstiegspunkt: Training, Eval, Play, Build in einer GUI.

## Screenshots für Präsentationen

**Nur echte Screenshots verwenden.** Die "premium"-PNGs in `docs/figures/` sind KI-Mockups und
wurden verworfen. Echte Aufnahmen: `build/stoneforge_client --ai --window-pos/-size` per stdin mit
Modell steuern + macOS `screencapture -R`. Live-Map-Screenshot via headless Chromium + CDP — dabei
18 s **echte** Wartezeit auf die WebSocket-Daten einplanen, `virtual-time-budget` funktioniert
hier nicht.
