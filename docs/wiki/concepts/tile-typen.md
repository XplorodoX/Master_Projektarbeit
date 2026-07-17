---
id: tile-typen
title: Tile-Typen — Eigenschaften, Härte, Drops
type: concept
tags: [spiel, tiles, datenmodell]
path: src/include/stoneforge/types.hpp, src/core/object.cpp, src/include/stoneforge/object.hpp
verified: 2026-07-17
related: [stoneforge-spiel, weltgenerierung, biome, doku-worldgen-veraltet, faktencheck-spiel-2026-07-17]
updated: 2026-07-17
---

# Tile-Typen

Jede Weltzelle ist genau ein `TileType` (`types.hpp:16–32`). **15 Werte, 0–14.** Die Eigenschaften
stehen nicht am Tile, sondern an einem `WorldObject` pro Typ (`object.cpp`, Zugriff über
`objectForTile(tile)`).

Vollständig gegen den Code verifiziert (17.07.2026):

| # | Tile | passierbar | **abbaubar** | Härte | Speed (ohne / Lv1 / Lv2) | Drop |
|---|------|:---:|:---:|:---:|---|---|
| 0 | `Empty` | ✅ | ❌ | – | – | – |
| 1 | `Wall` | ❌ | **❌** | – | – | – |
| 2 | `Resource` | ❌ | ✅ | **6.0** | 0.08 / 0.22 / 0.45 (**Spitzhacke**) | `Ore` |
| 3 | `Exit` | ✅ | ❌ | – | – | – |
| 4 | `Tree` | ❌ | ✅ | **2.2** | 0.22 / 0.45 / 0.78 (**Axt**) | `Wood` |
| 5 | `Workbench` | ❌ | ✅ | **3.6** | 0.16 / 0.38 / 0.65 (**Axt**) | `WorkbenchKit` |
| 6 | `WoodWall` | ❌ | **❌** | – | – | – |
| 7 | `WoodLog` | ❌ | **❌** | – | – | – |
| 8–14 | `StructureGrassland` … `StructureHelle` | ❌ | **❌** | – | – | – |

Passierbar sind **nur `Empty` und `Exit`** — alles andere blockiert. `Wall`, `WoodWall`, `WoodLog`
und alle `Structure*` überschreiben zusätzlich `blocksLineOfSight() → true`.

Die sieben `Structure*`-Typen korrespondieren 1:1 mit den sieben [[biome]]n.

## ⚠️ Korrektur 17.07.2026: Wände sind NICHT abbaubar

Die erste Fassung dieses Eintrags schrieb "`Wall` — Fels, abbaubar (langsam)". **Falsch.**

`isMineable()` ist im Basis-`WorldObject` auf **`false`** vorbelegt (`object.hpp:32–34`), und
`WallObject` (`object.cpp:41–54`) überschreibt **nur** `tileType()`, `id()` und
`blocksLineOfSight()`. Es überschreibt `isMineable()` **nicht** → Wände sind nicht abbaubar.

Nur **drei** Typen setzen `isMineable() → true`: `Resource`, `Tree`, `Workbench`.

Der Abbau-Pfad in `simulation.cpp:1040–1059` macht die Folge sichtbar:

```cpp
miningProgress_ += miningSpeed(tile);           // füllt sich auch bei einer Wand
if(miningProgress_ < miningHardness(tile)) { return; }
const auto& object = objectForTile(tile);
if(!object.isMineable()) { clearMiningProgress(); return; }   // ← Wand endet hier
```

Der Fortschrittsbalken läuft bei einer Wand also voll (Default-Speed 0.1, Default-Härte 1.0 → nach
10 Schritten) — und dann passiert **nichts**: `clearMiningProgress()` setzt zurück, die Wand bleibt.
Ein Laufband. Die Werte 0.1 und 1.0 sind für `Wall` **toter Code**; sie werden berechnet und
verworfen.

Dasselbe gilt für `WoodWall`, `WoodLog` und alle `Structure*` — auch sie setzen nur
`blocksLineOfSight()`.

> Auch `docs/stoneforge_game_description.md` führt `Wall` als "✅ abbaubar (langsam)" und schreibt
> "Kein Tool: 0.1/Schritt (Wand braucht ~10 Schritte)". Beides falsch → [[doku-worldgen-veraltet]].

## Die kuriose Konsequenz: Holz ist eine Einbahnstraße

`WoodWall` und `WoodLog` sind **platzierbar** — `item.cpp:144–145` registriert sie als
`PlaceableItem` (`ItemId::Planks → WoodWall`, `ItemId::Wood → WoodLog`). Aber sie sind **nicht
abbaubar**. Wer eine Holzwand setzt, bekommt sie über den Mining-Pfad **nicht wieder weg**.

Ob das Absicht ist oder ein übersehener `isMineable()`-Override, sagt der Code nicht. Für die
RL-Seite ist es egal (Bauen ist im Binding entfernt), für den spielbaren Client ist es eine echte
Design-Eigenschaft.

## ⚠️ Wasser ist KEIN Tile-Typ

**`Water` existiert im Enum nicht.** Wasser ist eine **Maske**, zur Laufzeit aus dem Rauschen
berechnet — `World::lakeMaskAt(x, y)` / `World::isLakeAt(x, y)` (`world.cpp:308–320`, siehe
[[weltgenerierung]]). Sie liegt **über** dem Tile, ersetzt es nicht.

Der Unterschied ist nicht akademisch: Ein Tile-Typ läge im gespeicherten Chunk-Array, die Maske ist
eine reine Funktion der Koordinaten und kostet **keinen Speicher**. Deshalb kann sie auch um Spawn
und Exit ausgespart werden, ohne einen Chunk anzufassen.

> `docs/stoneforge_game_description.md` führt `Water` als eigenen Tile-Typ. Im Datenmodell falsch.

## Warum das Modell so klein ist

15 Typen, ein `std::uint8_t`, ein flaches Array pro Chunk:

```cpp
struct Chunk {
    std::array<TileType, kChunkSize * kChunkSize> tiles{};   // 8×8×1 B = 64 B
    bool generated = false;                                  // + 1 B
};
```

Kein Component-System, keine Vererbung pro Zelle — die Eigenschaften hängen am geteilten
`WorldObject`-Singleton, nicht an der Instanz. Das ist der Grund, warum die Simulation schnell genug
ist, um sie millionenfach zu durchlaufen.

> Korrektur: Die erste Fassung schrieb "ein Chunk ist **exakt 64 Byte**". Das gilt für das
> `tiles`-Array; der `Chunk` trägt zusätzlich das `generated`-Flag (→ 65 Byte, keine Padding-Lücke,
> da alle Member 1-Byte-aligned sind).
