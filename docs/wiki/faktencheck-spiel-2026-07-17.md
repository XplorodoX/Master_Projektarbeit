---
id: faktencheck-spiel-2026-07-17
title: Faktencheck Spiel & Weltgenerierung — Prüfprotokoll (17.07.2026)
type: project
tags: [qualitaet, protokoll, korrekturen, spiel, worldgen]
verified: 2026-07-17
related: [stoneforge-spiel, tile-typen, weltgenerierung, biome, exit-platzierung, doku-worldgen-veraltet, faktencheck-2026-07-17]
updated: 2026-07-17
---

# Faktencheck: Spiel & Weltgenerierung

Gegenprüfung der Spiel-Einträge — Zeile für Zeile gegen `world.cpp`, `object.cpp`, `object.hpp`,
`types.hpp`, `simulation.cpp`, `item.cpp`, `game_config.json`, plus Web-Abgleich der
Fachbegriffe.

**5 Fehler in den eigenen Einträgen gefunden**, einer davon inhaltlich schwer. Alle korrigiert.

## 🔴 Der schwere Fehler: „Wände sind abbaubar"

[[tile-typen]] führte `Wall` als "abbaubar (langsam)", [[stoneforge-spiel]] schrieb "ohne Werkzeug
0,1/Schritt (eine Wand braucht ~10 Schritte)".

**Wände sind nicht abbaubar.** `isMineable()` ist im Basis-`WorldObject` auf `false` vorbelegt
(`object.hpp:32–34`); `WallObject` (`object.cpp:41–54`) überschreibt es **nicht** — nur
`tileType()`, `id()`, `blocksLineOfSight()`.

Nur **drei** Typen setzen `isMineable() → true`: `Resource`, `Tree`, `Workbench`. `WoodWall`,
`WoodLog` und alle sieben `Structure*` ebenfalls **nicht**.

Der Mining-Pfad (`simulation.cpp:1040–1059`) zeigt, warum die "10 Schritte" trotzdem plausibel
klangen: Der Fortschritt **läuft tatsächlich voll** (Default-Speed 0,1, Default-Härte 1,0 → 10
Schritte) — und wird dann von `if(!object.isMineable()) { clearMiningProgress(); return; }`
verworfen. Ein Laufband. Die Werte sind für `Wall` toter Code.

**Woher der Fehler kam:** aus `docs/stoneforge_game_description.md` — einer Datei, die ich im selben
Durchgang bereits bei Chunk-Größe und Water-Tile als falsch überführt hatte, und dann bei den
Mining-Werten trotzdem ungeprüft übernahm. Der Fehler ist nicht, dass die Quelle schlecht war —
er ist, dass ich sie **nach** dem Beweis ihrer Unzuverlässigkeit weiterbenutzt habe.

## 🟡 Vier weitere Korrekturen

| # | Behauptung | Tatsächlich |
|---|---|---|
| 2 | "Pickaxe Lv1 0,22, Lv2 0,45" als **allgemeine** Mining-Werte | Das sind **`Resource`**-Werte (Härte 6,0). Ohne Spitzhacke **0,08**, nicht 0,1. `Tree`: 0,22/0,45/0,78 bei Härte 2,2. `Workbench`: 0,16/0,38/0,65 bei Härte 3,6. |
| 3 | "Äxte fällen Holz schneller" (vage) | Jetzt mit Zahlen + Schrittzahlen (Härte / Speed) belegt |
| 4 | "ein Chunk ist **exakt 64 Byte**" | Das `tiles`-Array ist 64 B; der `Chunk` trägt zusätzlich `bool generated` → **65 B** |
| 5 | `mix()` sei "splitmix64-**artig**" | Es ist der **exakte** splitmix64-Finalizer (beide Konstanten, alle drei Shifts) — untertrieben statt falsch. Präzisiert. |

## ✅ Web-verifizierte Fachbegriffe

| Begriff | Ergebnis |
|---|---|
| **splitmix64** | Referenz: `z = (z^(z>>30))*0xbf58476d1ce4e5b9; z = (z^(z>>27))*0x94d049bb133111eb; return z^(z>>31)` — **identisch** mit `World::mix()`. Ursprung: MurmurHash3-64-Bit-Finalizer. Verwendet wird nur der Finalizer als Koordinaten-Hash, **nicht** der PRNG-Strom (kein State-Increment `0x9e3779b97f4a7c15`). |
| **Value Noise vs. Perlin** | Value Noise = skalare Zufallswerte an den Gitterecken, interpoliert (Hermite/Smoothstep 3t²−2t³). Perlin = **Gradientenvektoren** + Skalarprodukte + quintische Interpolation. Der Code sampelt vier **Werte** und lerpt → **eindeutig Value Noise**. |
| **Domain Warping** | Vorverzerrung der Eingangskoordinaten durch vorgeschaltetes Rauschen. Code: `warpX`/`warpY` → `x += (warpX - 0.5) * 2.8`. Der Kommentar bestätigt den Zweck: *"warped coordinates remove square-looking biome regions"*. ✓ |

## ✅ Gegen den Code bestätigt (unverändert korrekt)

- `kChunkSize = 8` (`world.hpp:15`) ✓ · Spawn (0,0), `spawnClearRadius` 2, `exitClearRadius` 1 ✓
- 15 Tile-Typen, `std::uint8_t` ✓ · passierbar nur `Empty` + `Exit` ✓
- Drops: `Resource → Ore`, `Tree → Wood`, `Workbench → WorkbenchKit` ✓
- `WoodWall`/`WoodLog` **platzierbar** (`item.cpp:144–145`, `PlaceableItem`) ✓ — aber nicht abbaubar
- 7 Biome, striktes **1/7**-Binning, pro **Chunk** bestimmt ✓
- Multi-Frequenz `0.62·n1 + 0.28·n2 + 0.10·n3`, Frequenzen 1 / 2.1 / 4.2 ✓ (fraktalähnlich)
- Wasser `0.75·a + 0.25·b > 0.86`, nie auf Spawn/Exit ✓ · Strukturen `P = 0.10` ✓
- **ASCII-Matrizen 5 × 5** ✓ (`std::array<std::string_view, 5>` mit 5-Zeichen-Zeilen, Rautenmuster)
- Exit per BFS-Distanzring ✓ · `forceGuaranteedPath` + `guaranteedPathFallback` **beide false** ✓
- `mobSpawn.count = 0` ✓ · gameplay-Parameter vollständig ✓
- Zwei getrennte Binaries ✓

## Nachtrag: 3. Prüfrunde (gleicher Tag)

Sweep über die restlichen, bis dahin ungeprüften Behauptungen + Web-Abgleich der Konstanten:

**Bestätigt ✓:** Obs-Layout 229 (Kommentarblock `stoneforge_env.py:68–72` = Wiki) · Demo-`REPRO_RUNS`
exakt (7025/52, 7037/55, 7000/4, 7028/60; 144–280 Schritte) + `exit_min/max=35/45` fest verdrahtet ·
Swarm `maxlen=500`, `swarm_prob=0.3` · **xxHash64-Primes** in `noise01` (PRIME64_1/2/3, Web-Quelle:
xxHash-Spec) — der Generator kombiniert xxHash-Primes mit dem splitmix64-Finalizer.

**Neu gefunden und ergänzt:**

| Fund | Wo dokumentiert |
|---|---|
| **Crafting fehlte komplett im Wiki** — 7 hartkodierte Rezepte, Werkzeugkette als Gating-Mechanismus | [[stoneforge-spiel]] |
| **4 tote Asset-Dateien**: `recipes.json`, `biomes.json` (altes cold/warm-System!), `entities.json`, `sprites.json` werden **nirgends geladen** (`loadJsonFile` ohne Aufrufer) | [[stoneforge-spiel]] |
| **Exit-Freischaltung ist toter Code** — `mobsKilledUnlocksExit_` default `false`, nie gesetzt, kein Config-Key | [[stoneforge-spiel]] |
| Survival-Zahlen (HP 10, Energie 100, Regen 8, Starvation 18) | [[stoneforge-spiel]] |
| Curriculum-Gates präzisiert: **Gate-Metrik wechselt stoch → det ab Phase 3**; Annealing ist **0.05 → 0.001**, nicht 0.01 (Docstring UND Aalen-Formel 0.12 falsch) | [[curriculum-learning]] |
| Aalen-PDF Kap. 0.1: Straf-Term-Formel ist der **vor-v11-Stand** (Wand-Penalties existieren nicht mehr, Loop −0.05 statt −0.15, Explorations-Bonus +0.02 fehlt) | [[doku-worldgen-veraltet]] |

## Die Lehre — dieselbe wie beim ersten Faktencheck

Der [[faktencheck-2026-07-17]] endete mit: *"Notizen sind Hypothesen, nicht Fakten. Reihenfolge:
Code → CHANGELOG → Doku → Primärquelle."*

Beim Anlegen der Spiel-Einträge habe ich genau dagegen verstoßen — und zwar **wissentlich**: Die
Chunk-Größe und das Water-Tile hatte ich bereits als Fehler derselben Datei nachgewiesen, die
Mining-Tabelle daraus dann trotzdem übernommen, weil sie plausibel aussah und Zahlen enthielt.

**Verschärfte Regel:** Eine Quelle, die bei einer Prüfung durchfällt, ist **für den ganzen
Durchgang verbrannt** — nicht nur an der Stelle, wo der Fehler auffiel. `game_description.md` gilt
ab jetzt als unzuverlässig ([[doku-worldgen-veraltet]]).
