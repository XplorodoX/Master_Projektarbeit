---
id: stoneforge-spiel
title: Stoneforge — das Spiel
type: concept
tags: [spiel, grundlagen, gameplay]
path: src/client/, src/apps/headless_main.cpp, assets/base/game_config.json
verified: 2026-07-17
related: [weltgenerierung, biome, cpp-core, tile-typen, doku-worldgen-veraltet]
updated: 2026-07-17
---

# Stoneforge — das Spiel

Ein **rundenbasiertes 2D-Survival-Erkundungsspiel** in prozedural generierten Welten. Der Spieler
startet an (0, 0) in einer unbekannten Welt mit genau einem Ziel: **den Exit finden und erreichen**.

## Warum das Design so gewählt ist

Die Welt ist ein reines **Tile-Grid** — keine Grafik-Engine im Kern, kein Echtzeit-Loop. **Jede
Spieleraktion ist exakt ein Simulationsschritt.** Das ist der entscheidende Designzug: Er macht das
Spiel deterministisch, schnell und beliebig parallelisierbar. Rundenbasiert heißt hier nicht
"altmodisch", sondern "exakt reproduzierbar" — dieselbe Aktionsfolge auf demselben Seed erzeugt
dieselbe Welt und denselben Verlauf.

## Die Welt

Theoretisch **unendlich groß**, unterteilt in **Chunks von 8 × 8 Tiles** (`kChunkSize = 8` in
`src/include/stoneforge/world.hpp:15`). Chunks werden **lazy** generiert — ein Chunk existiert erst,
wenn ihn jemand betritt. Nicht besuchte Bereiche kosten keinen Speicher.

> ⚠️ `docs/stoneforge_game_description.md` behauptet an drei Stellen **16 × 16**. Der Code sagt
> **8 × 8**. Siehe [[doku-worldgen-veraltet]].

| | |
|---|---|
| Weltkoordinaten | (−∞, +∞) × (−∞, +∞) |
| Chunk-Größe | **8 × 8** Tiles |
| Spawn | (0, 0), Freiradius 2 |
| Exit | zufällig, **35–45 BFS-Laufweg** vom Spawn, Freiradius 1 |

Wie die Welt entsteht: [[weltgenerierung]]. Was drin ist: [[tile-typen]], [[biome]].

## Spielmechaniken

Aus `assets/base/game_config.json`, `gameplay`-Block (verifiziert 17.07.2026):

| Parameter | Wert |
|-----------|------|
| `observationRadius` | 7 (→ 15 × 15 Sichtfenster) |
| `maxSteps` | 4 000 |
| `inventorySlots` / `inventoryStackLimit` / `hotbarSlots` | 24 / 64 / 9 |
| `miningRangeBaseTiles` | 4,5 (+ 0,75 je Tool-Level) |
| `idleEnergyRegenInterval` | 8 |
| `activeEnergyDrainInterval` | 18 |
| `starvationTicksToDamage` | 18 |
| `render.stepIntervalSeconds` | 0,12 |

**Mining** ist tool-abhängig: ohne Werkzeug 0,1/Schritt (eine Wand braucht ~10 Schritte), Pickaxe
Lv1 0,22/Schritt, Lv2 0,45/Schritt; Äxte fällen Holz schneller.

**Mobs sind aus.** `mobSpawn.count = 0`. Die Verhaltensprofile (`entityBehaviorProfiles`:
zombie-Controller, `detectRange` 8, `loseRange` 12, `wanderChance` 0,55) existieren noch in der
Konfiguration, laufen aber ins Leere — die Welt ist derzeit menschenleer.

## Zwei Binaries

| Binary | Quellen | Zweck |
|--------|---------|-------|
| `stoneforge_client` | `src/client/` — `raylib_main.cpp`, `sdl_main.cpp`, `render_engine.cpp`, `render_ui.cpp`, `render_fx.cpp`, `command_registry.cpp` | grafisch spielbar |
| Headless-Runner | `src/apps/headless_main.cpp` | ohne Fenster, schnell |

Beide sitzen auf demselben `stoneforge_core`. Die Trennung ist sauber und hat einen praktischen
Nutzen, siehe [[cpp-core]]: Rendering-Änderungen können die Simulation nicht berühren.
