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

**Mining** — verifiziert gegen `object.cpp` (17.07.2026), Schritte = Härte / Speed:

| Tile | Härte | ohne Werkzeug | Werkzeug Lv1 | Lv2 |
|------|:---:|---|---|---|
| `Resource` (Spitzhacke) | 6,0 | 0,08 → **75 Schritte** | 0,22 → **~27** | 0,45 → **~13** |
| `Tree` (Axt) | 2,2 | 0,22 → **10 Schritte** | 0,45 → **~5** | 0,78 → **~3** |
| `Workbench` (Axt) | 3,6 | 0,16 → **~23 Schritte** | 0,38 → **~9** | 0,65 → **~6** |

**Abbaubar sind nur diese drei Typen.** Wände sind es **nicht** — auch nicht langsam, auch nicht mit
Werkzeug. Details und der Nachweis: [[tile-typen]].

> ⚠️ Die erste Fassung dieses Eintrags schrieb "ohne Werkzeug 0,1/Schritt (eine Wand braucht ~10
> Schritte), Pickaxe Lv1 0,22". Das war aus `stoneforge_game_description.md` übernommen und gleich
> dreifach falsch: Wände sind gar nicht abbaubar; 0,1 ist ein toter Default; und 0,22/0,45 sind
> `Resource`-Werte (ohne Spitzhacke 0,08, nicht 0,1). Siehe
> [[faktencheck-spiel-2026-07-17]].

**Mobs sind aus.** `mobSpawn.count = 0`. Die Verhaltensprofile (`entityBehaviorProfiles`:
zombie-Controller, `detectRange` 8, `loseRange` 12, `wanderChance` 0,55) existieren noch in der
Konfiguration, laufen aber ins Leere — die Welt ist derzeit menschenleer.

## Crafting — die Werkzeugkette (fehlte bisher im Wiki)

Sieben Rezepte, **hartkodiert im `RecipeCatalog`-Konstruktor** (`recipe.cpp` ~Z. 200 ff.,
verifiziert 17.07.2026). Sie erklären die Tool-Level der Mining-Tabelle:

| Rezept | Eingabe | Ausgabe | Werkbank nötig |
|--------|---------|---------|:---:|
| Planks ×4 | 1 Wood | 4 Planks | – |
| Sticks ×4 | 2 Planks | 4 Sticks | – |
| Workbench Kit | 10 Planks | 1 Kit (platzierbar) | – |
| Axe Lv1 / Pickaxe Lv1 | 3 Planks + 2 Sticks | Tool-Upgrade | **✓** |
| Axe Lv2 / Pickaxe Lv2 | 3 Ore + 2 Sticks | Tool-Upgrade | **✓** |

Die Progression ist eine Schleife: Baum fällen → Holz → Planks → Sticks + Werkbank → Lv1-Tools →
schneller Erz abbauen → Lv2-Tools. Erz (Härte 6,0, ohne Spitzhacke 0,08/Schritt = 75 Schritte) ist
praktisch erst mit Lv1-Spitzhacke wirtschaftlich — das ist der Gating-Mechanismus des Spiels.

> ⚠️ **`assets/base/recipes.json` ist eine tote Datei.** Sie spiegelt die hartkodierten Rezepte,
> aber `RecipeCatalog::loadJsonFile()` wird **nirgends aufgerufen** — kein `grep`-Treffer in `src/`.
> Gleiches gilt für `biomes.json` (enthält noch das alte cold/warm-System!), `entities.json` und
> `sprites.json`: **keine dieser Dateien wird vom Code geladen.** Vermutlich Überbleibsel eines
> geplanten Mod-Systems (`mod.json`: `"scripts": []`). Wer Rezepte ändern will, muss `recipe.cpp`
> anfassen und neu bauen ([[rebuild-pflicht]]) — die JSON zu editieren bewirkt nichts.

## Survival-Mechanik (Zahlen aus dem Code)

Start: **HP 10, Energie 100** (`simulation.cpp:204–205`). Jede Aktion kostet 1 Energie
(Mining/Platzieren zusätzlich); Ruhe regeneriert +1 je `idleEnergyRegenInterval` (8). Bei
Energie 0 zählt ein Hunger-Zähler: alle `starvationTicksToDamage` (18) Ticks −1 HP
(`simulation.cpp:353–356`).

Im RL-Env ist das alles **abgeschaltet** (`disable_energy=True`, `stoneforge_env.py:155`) — deshalb
waren Energie/Inventar tote Features und flogen mit Env v11 aus der Observation
([[observation-space]]).

## Kuriosum: die Exit-Freischaltung ist toter Code

Die Simulation kennt einen Mechanismus, den Exit erst nach N Mob-Kills freizuschalten
(`mobsKilledUnlocksExit_`, `killsRequired_`, `simulation.cpp:328–330`). Aber:
`mobsKilledUnlocksExit_ = false` ist der Member-Default (`simulation.hpp:208`) und wird **nirgends
im Code auf `true` gesetzt** — es gibt auch keinen Config-Key dafür. Der Exit ist daher immer
offen (`exitUnlocked_ = !mobsKilledUnlocksExit_` → immer `true`). Passt ins Bild: Mobs sind aus
(`mobSpawn.count = 0`), Kampf ist aus dem RL-Binding entfernt.

## Zwei Binaries

| Binary | Quellen | Zweck |
|--------|---------|-------|
| `stoneforge_client` | `src/client/` — `raylib_main.cpp`, `sdl_main.cpp`, `render_engine.cpp`, `render_ui.cpp`, `render_fx.cpp`, `command_registry.cpp` | grafisch spielbar |
| Headless-Runner | `src/apps/headless_main.cpp` | ohne Fenster, schnell |

Beide sitzen auf demselben `stoneforge_core`. Die Trennung ist sauber und hat einen praktischen
Nutzen, siehe [[cpp-core]]: Rendering-Änderungen können die Simulation nicht berühren.
