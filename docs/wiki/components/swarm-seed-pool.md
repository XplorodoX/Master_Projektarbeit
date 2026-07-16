---
id: swarm-seed-pool
title: SwarmSeedPool — Erfolgs-Replay von Seeds
type: component
tags: [training, code, offene-frage]
path: python/stoneforge_env.py (class SwarmSeedPool)
related: [stoneforge-env, train-curriculum, batch-size-8]
status: offen
updated: 2026-07-17
---

# SwarmSeedPool

Von PokéRL inspiriert: Seeds, auf denen der Agent Erfolg hatte, kommen in einen Pool und werden
mit `swarm_prob = 0.3` erneut gespielt (`maxlen = 500`).

## Die offene Frage

Die PLR-Literatur (Prioritized Level Replay) spricht eigentlich **gegen** Erfolgs-Replay — sie
priorisiert Level mit hohem Lernpotential, also eher die **schwierigen**, nicht die bereits
gelösten. Trotzdem lief der historische 86-%-Lauf mit genau diesem Mechanismus.

Der Pool hat deshalb einen `plr_mode`-Schalter. Der geplante A/B-Test (B8 in
`docs/BEWERTUNG_UND_PLAN.md`): Erfolgs-Swarm (Status quo) vs. `--plr` vs. `--no-swarm`.

**Status: nicht abschließend beantwortet.** Die Forensik vom 07.07.2026 hat den Swarm als Ursache
der v11-Instabilität **entlastet** (der Schuldige war [[batch-size-8]]), aber ob er netto *hilft*,
ist damit nicht gezeigt — nur, dass er nicht schadet. Ehrliche Formulierung für die Arbeit: als
übernommene Heuristik mit offener Wirksamkeit ausweisen, nicht als Beitrag verkaufen.

## Gefixter Bug

Der Pool wurde beim Phasenwechsel nicht geleert → Seeds aus Phase 1 (Exit-Distanz 5–12) wurden in
Phase 3 (25–45) weitergespielt und haben die Trainingsverteilung verwässert. Seit 07.07.2026 wird
der Pool bei Phasenstart geleert.
