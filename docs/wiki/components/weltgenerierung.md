---
id: weltgenerierung
title: Prozedurale Weltgenerierung — der Algorithmus
type: component
tags: [spiel, worldgen, algorithmus, cpp]
path: src/core/world.cpp
verified: 2026-07-17
related: [stoneforge-spiel, biome, tile-typen, exit-platzierung, cpp-core, doku-worldgen-veraltet]
updated: 2026-07-17
---

# Prozedurale Weltgenerierung

`src/core/world.cpp` (733 Zeilen). Alle Angaben am 17.07.2026 gegen den Code verifiziert.

Die Architektur ist **zweigeteilt**: mathematisch fundierte, deterministische Rauscherzeugung für
die Basisgeometrie — plus pragmatische, heuristische Regeln für die Spielbarkeit. Diese Zweiteilung
ist der rote Faden des Kapitels.

## 1. Determinismus aus einem Seed

Alles hängt an einem initialen **Seed**. Damit eine unendliche Welt ohne Speicherung konsistent
bleibt, wird jede Zelle direkt aus ihren Koordinaten berechnet statt gespeichert.

**Hash-Funktion** `World::mix()` (Z. 113) — eine splitmix64-artige Bit-Mischung
(Konstanten `0xbf58476d1ce4e5b9`, `0x94d049bb133111eb`).

**`World::noise01(x, y, salt)`** (Z. 124) kombiniert Koordinaten, Welt-Seed und einen
**funktionsspezifischen Salt** zu einem Pseudozufallswert in [0, 1]. Die Salts trennen die
Domänen voneinander, damit Biom, Dichte, Erz und Bäume nicht korrelieren:

```json
"noiseSalts": { "biome": 2882395322, "density": 270544960,
                "ore": 2575857510, "tree": 1430532898 }
```

## 2. Value Noise mit Smoothstep

Für zusammenhängende Strukturen wird **Value Noise** auf Gitterzellen verwendet: Die vier Eckpunkte
einer Zelle werden gesampelt und bilinear interpoliert. Die Glättung nutzt **Smoothstep**:

$$S(t) = 3t^2 - 2t^3$$

Im Code als `t * t * (3.0 - 2.0 * t)` (Z. ~136). Smoothstep hat an beiden Enden Ableitung null —
deshalb sieht man die Zellgrenzen nicht.

## 3. Domain Warping und fraktale Kombination

**Domain Warping:** Die Eingangskoordinaten werden vor der Noise-Abfrage durch ein vorgeschaltetes
Rauschen verzerrt. Zweck: die sichtbaren **Raster-Artefakte** des quadratischen Gitters
verschwinden — ohne Warping sieht man dem Ergebnis das Gitter an.

**Multi-Frequenz-Kombination** (fraktalähnlich), Z. 169:

```cpp
const double combined = n1 * 0.62 + n2 * 0.28 + n3 * 0.10;
```

Drei Frequenzen mit fallender Gewichtung: grobe Struktur dominiert, feine Frequenzen fügen Details
hinzu.

## 4. Heuristik statt Mathematik: Wasser und Strukturen

Ab hier regiert das Spieldesign, nicht die Theorie.

**Seen** (`World::lakeMaskAt`, Z. 308–318) — gewichtete Linearkombination zweier Rauschfunktionen
mit hartem Cutoff:

```cpp
const double lakeScore = 0.75 * a + 0.25 * b;
return lakeScore > 0.86;        // "Very rare lakes"
```

Der Cutoff 0,86 ist reines Balancing — er macht Seen selten. Wasser wird **nie auf Spawn oder Exit**
gelegt (`isLakeAt`, Z. 320). Wichtig: Wasser ist eine **Maske**, kein Tile-Typ → [[tile-typen]].

**Strukturen / Landmarken** — feste Wahrscheinlichkeit **P = 0,10 pro Chunk**
(`kStructureChance`, Z. 220). Die Optik stammt aus handgemachten, biomspezifischen
**5 × 5-ASCII-Matrizen** (Z. ~239). Nicht generiert, sondern gezeichnet.

## 5. Exit-Platzierung — der wichtigste Schritt

Die Suche nach validen Exit-Kandidaten läuft über eine **BFS-Traversierung in einem Distanzring**
(Z. ~348). Der Kommentar im Code sagt, warum das so sein muss:

> "Kandidaten nach BFS-Pfadlänge statt Luftlinie wählen: exitMin/MaxDistance bedeutet damit echten
> Laufweg vom Spawn. Vorher (dist² in [min², max²]) streute der reale Pfad bei »35–45« auf 42–75 Tiles."

Das ist zugleich die **Lösbarkeitsgarantie**: Wer den Exit per BFS vom Spawn aus findet, hat bewiesen,
dass ein Weg existiert. Details: [[exit-platzierung]].

## 6. Optionale Validierung — komplett abgeschaltet

Drei Verfahren existieren, **keines läuft** (`game_config.json`, verifiziert 17.07.2026):

| Verfahren | Config-Schalter | Zeile | Status |
|-----------|-----------------|-------|--------|
| Cellular Smoothing (zellulärer Automat, Moore-Nachbarschaft, Birth 5 / Survival 4, 2 Iterationen) | `enableCellularSmoothing` | 19 | **false** |
| FloodFill-Validierung (fensterbasierte BFS-Konnektivität, Radius 8 Chunks) | `enableFloodFillValidation` | 23 | **false** |
| Macro-Graph-Precheck (Radius 16 Chunks) | `enableMacroGraphPrecheck` | 25 | **false** |
| **Garantierter Pfad (Manhattan-Carve)** | `forceGuaranteedPath` | **8** | **false** |
| Manhattan-Carve als Notfall-Fallback | `guaranteedPathFallback` | **9** | **false** |

> ⚠️ **Der Manhattan-Carve ist AUS.** `World::carveGuaranteedPath()` (Z. 699) wird nur aufgerufen,
> wenn `cfg.forceGuaranteedPath` (Z. 33–34) oder — bei nicht erreichbarem Ziel —
> `cfg.guaranteedPathFallback` (Z. 44–45) gesetzt ist. **Beide sind `false`.** Der Code läuft nie.
>
> Das Aalen-Dokument behauptet das **Gegenteil** ("Garantierter Pfad (Manhattan) AKTIVIERT" und
> "das System arbeitet in der Produktion primär über den schnellen Manhattan-Fallback") — siehe
> [[doku-worldgen-veraltet]].

**Warum das konsistent ist:** Der Manhattan-Carve wäre eine Krücke — er schneidet einen geraden
Korridor (erst X-, dann Y-Achse) in die Welt und garantiert damit Spielbarkeit, aber um den Preis
einer künstlichen Autobahn. Seit die Exit-Platzierung per BFS erfolgt, ist die Lösbarkeit **schon
durch die Konstruktion** garantiert. Die Krücke ist überflüssig geworden — nicht vergessen, sondern
**bewusst abgeschaltet** (`CLAUDE.md`: "redundant seit v11").

Die drei Validierungsverfahren sind aus Performancegründen aus: Sie sind rechenintensiv und
verhindern Probleme, die die BFS-Exit-Platzierung ohnehin ausschließt.
