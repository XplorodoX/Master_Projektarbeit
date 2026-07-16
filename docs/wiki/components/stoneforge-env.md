---
id: stoneforge-env
title: StoneforgeWorldEnv — das Gym-Environment
type: component
tags: [env, code, gym]
path: python/stoneforge_env.py
related: [observation-space, cpp-core, config-prozessglobal, swarm-seed-pool, obs-shape-legacy]
updated: 2026-07-17
---

# StoneforgeWorldEnv

`python/stoneforge_env.py` — das Gym-Environment. Wird **direkt** verwendet, kein Wrapper-Stack.
Die früheren `ExitPotentialFieldWrapper` / `ReducedActionEnv` liegen archiviert in `OLD/`.

## Schnittstelle

- **Aktionsraum:** `Discrete(4)` — 0 = hoch, 1 = runter, 2 = links, 3 = rechts.
- **Observation:** 229 Features, siehe [[observation-space]].
- **Episodenlimit:** `maxSteps = 4000`.

Mining, Bauen und Kampf sind seit [[v11-env-bruch]] **im C++-Binding entfernt** (Aktionen 4–8
werfen `RuntimeError`). Sie existieren nur noch im spielbaren Client. Das ist kein Rückbau,
sondern Aufräumen: Für die Exit-Suche waren sie tote Freiheitsgrade.

## Wichtige Konstruktor-Argumente

```python
env = StoneforgeWorldEnv(exit_min=35, exit_max=45)          # Standard-Eval
env = StoneforgeWorldEnv(..., include_energy_inventory=True)  # Legacy-Modelle (231-dim)
```

`env_kwargs_for_model(model)` leitet die passenden kwargs aus der Obs-Shape des Modells ab —
der bequeme Weg um [[obs-shape-legacy]] herum.

## Was das Env zusätzlich zum C++-Core macht

- **Visit-Count-Penalty:** `reward -= 0.03 * min(visit_count / 25.0, 2.0)` — dämpft Im-Kreis-Laufen.
- **Stagnations-Abbruch:** 256 Schritte ohne positiven Reward → Episode endet.
- **Config-Stempel bei jedem `reset()`** — kritisch, siehe [[config-prozessglobal]].
- **Normalisierung** der rohen C++-Tile-Liste.

## Der Config-Stempel ist kein Detail

Die WorldGen-Config lebt **prozessglobal im C++-Core**. Das Env stempelt sie bei jedem `reset()`
neu, weil sonst ein anderer Konsument (z. B. der Eval-Callback) die Trainingsverteilung still
verschiebt. Genau dieser Bug war lange im Projekt aktiv → [[config-prozessglobal]].
