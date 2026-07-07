# Stoneforge — Spielbeschreibung

*Stand: 25.06.2026*

---

## Was ist Stoneforge?

Stoneforge ist ein **rundenbasiertes 2D-Survival-Erkundungsspiel** in prozedural generierten Welten.
Der Spieler startet in einer unbekannten Welt und hat genau ein Ziel: den **Exit finden und erreichen**.

Die Welt existiert als reines Tile-Grid — kein Grafik-Engine-Overhead, kein Echtzeit-Loop.
Jede Spieleraktion entspricht exakt einem Simulationsschritt. Das macht das Spiel ideal
als RL-Umgebung: vollständig deterministisch, schnell, und beliebig parallelisierbar.

**Kernprinzip:** Erkunde, navigiere, überlebe — finde den Ausgang.

---

## Die Spielwelt

### Chunks und unendliche Welt

Die Welt ist theoretisch **unendlich** groß. Sie wird in **Chunks** von je 16×16 Tiles unterteilt,
die lazy generiert werden — ein Chunk existiert erst, wenn der Spieler (oder die BFS-Suche) ihn
zum ersten Mal betritt. Nicht besuchte Bereiche kosten keinen Speicher.

```
Weltkoordinaten:   (-∞, +∞) × (-∞, +∞)
Chunk-Größe:       16 × 16 Tiles
Spawn:             (0, 0)
Exit:              zufällig, 35–45 BFS-Tiles vom Spawn
```

### Tile-Typen

Jede Zelle der Welt ist genau ein Tile-Typ:

| Tile | Passierbar | Abbaubar | Beschreibung |
|------|-----------|----------|--------------|
| `Empty` | ✅ | — | Boden / Luft — freier Weg |
| `Wall` | ❌ | ✅ (langsam) | Fels / Wand — blockiert Bewegung |
| `Resource` | ❌ | ✅ | Erz im Fels — gibt Rohstoffe |
| `Tree` | ❌ | ✅ (schnell) | Baum — gibt Holz |
| `Exit` | ✅ | — | **Ziel** — Spielende bei Betreten |
| `Water` | ✅ | — | Wasser — passierbar, kein Effekt |
| `Workbench` | ❌ | — | Werkbank — Crafting-Station |
| `StructureGrassland` … `StructureHelle` | ❌ | — | Biom-Strukturen (dekorativ + blockierend) |

Mining-Geschwindigkeit hängt vom Tool-Level ab:
- Kein Tool: 0.1/Schritt (Wand braucht ~10 Schritte)
- Pickaxe Lv1: 0.22/Schritt (Resource-Stein: ~28 Schritte)
- Pickaxe Lv2: 0.45/Schritt (Resource-Stein: ~14 Schritte)
- Axt Lv1+: schnelleres Holzfällen

---

## Biome

Die Welt ist in **7 Biome** unterteilt, die durch Perlin-Noise (mit Domain-Warping) entstehen.
Das Biom bestimmt: Wand-Dichte, Ressourcen-Häufigkeit, Baumbestand und Strukturtyp.

```
Biom-Wert (0.0 → 1.0, aus Noise):

  0.000 ──── 0.143  →  Grasland    (offen, wenig Wände)
  0.143 ──── 0.286  →  Wald        (viele Bäume, mitteldicht)
  0.286 ──── 0.429  →  Wüste       (wenig Vegetation, viel Stein)
  0.429 ──── 0.571  →  Bergland    (dichte Wände, viel Erz)
  0.571 ──── 0.714  →  Steppe      (flach, weitläufig)
  0.714 ──── 0.857  →  Tundra      (kalt, spärlich)
  0.857 ──── 1.000  →  Hölle       (extrem dicht, gefährlich)
```

Jedes Biom hat eigene **Strukturen** — 5×5-Tile-Muster die mit 10% Wahrscheinlichkeit
pro Chunk platziert werden (nie über Spawn oder Exit):

```
Grasland:  #...#    Wald:   ..#..    Wüste:  ..#..
           .#.#.           .###.            .###.
           #####           ##.##            #####
           .###.           #...#            .###.
           #...#           #####            ..#..
```

---

## Der Spieler

### Attribute

| Attribut | Startwert | Verlust | Effekt bei 0 |
|----------|-----------|---------|--------------|
| **HP** | 10 | Mob-Kontakt (−1/s), Verhungern | Episode endet (Tod) |
| **Energie** | 100 | Jede aktive Aktion (−1/18 Schritte) | Verhungern: −1 HP/18 Ticks |
| **Inventory** | 24 Slots, je 64× | — | Items sammeln, Crafting |

**Energie-Regeneration:** Bei `Wait`-Aktion +1 Energie alle 8 Schritte.
**Werkzeug-Level:** Axt-Level und Pickaxe-Level erhöhen Mining-Geschwindigkeit.

### Spawn-Bereich

Um (0,0) wird beim Start ein 5×5-Tile-Bereich freigeräumt (`spawnClearRadius=2`),
sodass der Spieler nie direkt in einer Wand spawnt.

---

## Aktionen

Pro Schritt führt der Spieler **genau eine Aktion** aus:

| Aktion | Code | Effekt |
|--------|------|--------|
| `MoveUp` | 0 | Bewegt den Spieler um (0, −1) wenn Tile passierbar |
| `MoveDown` | 1 | Bewegt den Spieler um (0, +1) wenn Tile passierbar |
| `MoveLeft` | 2 | Bewegt den Spieler um (−1, 0) wenn Tile passierbar |
| `MoveRight` | 3 | Bewegt den Spieler um (+1, 0) wenn Tile passierbar |
| `Mine` | 4 | Baut Tile in Blickrichtung ab (mehrere Schritte nötig) |
| `Place` | 5 | Platziert Item aus aktiver Hotbar-Slot |
| `Use` | 6 | Angriff: trifft Mobs in 3×3-Umgebung |
| `Wait` | 7 | Nichts tun — Energie regeneriert |
| `Noop` | 8 | Identisch mit Wait |

**Für RL-Training:** Nur die 4 Bewegungsaktionen sind aktiv (`Discrete(4)`, seit Env v11
direkt im C++-Binding erzwungen — Aktionen 4–8 werfen einen Fehler).
Mining, Bauen und Angreifen existieren nur im spielbaren Client — der Agent muss rein navigieren.

---

## Mobs

Das Spiel hat 4 Mob-Typen mit unterschiedlichem KI-Verhalten.
**Für RL-Training sind Mobs vollständig deaktiviert** — sie existieren nur im vollen Spielmodus.

### Mob-Verhaltenssysteme

**Zombie** (Standard-Gegner):
```
Erkennt Spieler auf 10 Tiles  →  jagt direkt (stepToward)
Verliert Spieler auf 14 Tiles →  wandert zufällig (45% Chance/Schritt, 15% idle)
Schaden: 1 HP/Sekunde bei Kontakt
Immer aggressiv: defaultAggro=true
```

**Animal** (flüchtet):
```
Hält bevorzugt ≥4 Tiles Abstand zum Spieler
Wenn zu nah: flüchtet (stepAway)
Schaden: 1 HP/s (Verteidigung wenn eingeklemmt)
Wandert aktiv: 75% Chance/Schritt
```

**Boss** (Elite-Gegner):
```
Erkennt Spieler auf 16 Tiles — große Aggroreichweite
Verliert Spieler auf 20 Tiles
Immer aggressiv (kein idle, nur 25% Wandern)
Schaden: 2 HP/s — doppelt so stark wie Normal
```

**Default** (Fallback):
```
Wie Zombie, aber etwas passiver (55% Wandern, 25% idle)
Erkennt Spieler auf 8 Tiles
```

### Mob-Spawning

Mobs spawnen in der Nähe des Spielers (Offset 10 Tiles, Jitter ±8), passierbare Position wird
bis zu 20× neu gewürfelt. Spawn-Tabelle ist im aktuellen Training leer — keine Mobs.

---

## Weltgenerierung im Detail

### Wie ein Chunk entsteht (lazy, on-demand)

```
ensureChunk(cx, cy) wird aufgerufen
        ↓
Existiert Chunk? → sofort zurückgeben
        ↓ nein
biomeTagForChunk(cx, cy) berechnen (Perlin-Noise + Domain-Warping)
        ↓
Für jedes der 16×16 Tiles:
  wallThreshold aus Biom auslesen
  noise01(x, y, densitySalt) > threshold?  → Wall
  noise01(x, y, oreSalt) > oreThreshold?   → Resource (Erz)
  noise01(x, y, treeSalt) > treeThreshold? → Tree (Baum)
  sonst                                    → Empty
        ↓
placeBiomeStructure? (10% Wahrscheinlichkeit, 5×5-Pattern)
        ↓
Chunk fertig, gecacht in HashMap
```

### Noise-Funktion

Kein klassisches Simplex/Perlin — stattdessen eine **hash-basierte Punkt-Noise**:
```
noise01(x, y, salt):
  value = x * 0x9e3779b185ebca87
  value ^= y * 0xc2b2ae3d27d4eb4f
  value ^= seed * 0x165667b19e3779f9
  value ^= salt
  value = mix(value)   ← bijektive Bit-Mixing-Funktion
  return (value & 0xFFFFFFFFFFFF) / 0xFFFFFFFFFFFF
```

Das ergibt einen deterministischen float in [0,1] für jede (x,y,seed,salt)-Kombination.
Für Biome werden 3 Noise-Schichten mit Domain-Warping kombiniert (verhindert quadratische Biom-Grenzen).

### Exit-Platzierung

```
1. Flood-Fill vom Spawn über begehbare Tiles; Kandidaten mit
   BFS-Tiefe in [exitMinDistance, exitMaxDistance]  (seit v11 echter Laufweg,
   vorher Luftlinie — realer Weg streute damals auf 42–75 bei "35–45")
2. Kandidaten-Check mit virtueller Freiräumung: Räumung darf den Laufweg
   nicht unter exitMinDistance verkürzen (bis 24 Versuche)
3. 3×3-Bereich um Exit wird freigeräumt (exitClearRadius=1), Exit-Tile gesetzt
4. BFS vom Exit über die gesamte Welt berechnet (einmal pro reset())
   → O(1)-Distanz-Lookup während der Episode
```

---

## BFS-Distanzfeld

**Das wichtigste technische Feature für RL:**

Bei jedem `reset()` wird vom Exit-Tile aus eine **Breitensuche** über die gesamte erreichbare
Welt durchgeführt. Das Ergebnis ist ein Dictionary `{(x,y) → BFS-Distanz zum Exit}`.

```
BFS-Box: Bounding-Box(Spawn, Exit) + Buffer von 80 Tiles
  → deckt >95% aller möglichen Spieler-Positionen ab (sqrt(4000 maxSteps) ≈ 63 Tiles)
```

**Nutzen:**
- Reward-Shaping: Agent wird für jeden BFS-Schritt näher zum Exit belohnt
- `stepsWithoutProgress`: wie lange der Agent kein neues BFS-Minimum erreicht hat
- `bfsDistanceAtOffset(dx, dy)`: zeigt dem Agenten welche Richtung wirklich hilft

---

## Crafting-System

Das Spiel hat ein vollständiges Crafting-System (für den menschlichen Spieler, im RL deaktiviert):

- Items werden gesammelt durch Mining (Holz, Erz, Stein)
- Werkbank (`Workbench`-Tile) ermöglicht Rezepte
- Werkzeuge (Axt, Pickaxe) erhöhen Mining-Geschwindigkeit
- Rezepte sind in JSON-Dateien definiert und werden per `nlohmann/json` geladen

---

## Spielziel & Episode-Ende

Eine Episode endet wenn eine der folgenden Bedingungen eintritt:

| Bedingung | Ergebnis | Reward |
|-----------|----------|--------|
| Spieler betritt Exit-Tile | **Sieg** | +100 |
| HP ≤ 0 (Tod durch Mob/Hunger) | Niederlage | −10 |
| Schritte ≥ 4.000 (Timeout) | Niederlage | −10 |
| 256 Schritte ohne positiven Reward (RL-Only) | Truncation | (kein Extra-Penalty) |

---

## Das Spiel aus Sicht des RL-Agenten

So sieht die Welt aus Agenten-Perspektive aus (schematisch):

```
Weltausschnitt (viel größer als der Agent sieht):

  ██████████████████████████████████
  ██    ██      ██   ██    ██      ██
  ██    ██  ██  ██   ██    ██  ██  ██
  ██        ██       ██        ██  ██
  ██████████████  ████████████████  ██
  ██       ██  ██    ██              ██
  ██   @   ██        ██   (Exit?)    ██   ← Spieler bei @
  ██        ██  ████ ██              ██      sieht nur 15×15
  ██    ██████  ██   ██████████████████
  ██    ██      ██   ██   E            ██  ← Exit irgendwo hier
  ████████████████████████████████████

Was der Agent wirklich sieht (15×15 um @):

  ░░░░░░░░░░░░░░░
  ░░░░░░░░░░░░░░░
  ░░░░░░░░░░░░░░░
  ░░░░░░░░░░░░░░░
  ░░██░░░░░░██░░░
  ░░██░░░░░░░░░░░
  ░░░░░░@░░░░██░░   @ = Spieler (Mitte)
  ░░░░░░░░██░░░░░   ██ = Wand
  ░░░░░░░░██░░░░░   ░ = Boden/Luft
  ░░░░░░░░░░░░░░░
  ░░████░░░░░░░░░
  ░░░░░░░░░░░░░░░
  ░░░░░░░░░░░░░░░
  ░░░░░░░░░░░░░░░
  ░░░░░░░░░░░░░░░

  + exitDx = +28  (Exit liegt 28 Tiles rechts)
  + exitDy = −12  (Exit liegt 12 Tiles über dem Spieler)
```

**Das Problem:** exitDx/exitDy zeigt die Luftlinie — nicht den Pfad durch Wände.
Der Agent muss lernen, Wänden auszuweichen ohne die globale Karte zu kennen.

---

## Technische Eckdaten

| Parameter | Wert |
|-----------|------|
| Weltstruktur | Chunks à 16×16 Tiles, lazy generiert |
| Biome | 7 (Grasland, Wald, Wüste, Bergland, Steppe, Tundra, Hölle) |
| Noise-Typ | Hash-basierte Punktnoise + Domain-Warping |
| Sichtradius (RL) | 7 Tiles → 15×15 Gitter (225 Tiles) |
| Observation-Größe | 229 Features (float32) — seit Env v11; Legacy-Modelle: 231 |
| Aktionsraum (RL) | Discrete(4) — nur Bewegung |
| Max. Schritte/Episode | 4.000 |
| Exit-Distanz (Eval) | 35–45 Tiles (BFS) |
| BFS-Buffer | 80 Tiles um Spawn/Exit-Box |
| Mob-Typen | 4 (Zombie, Animal, Boss, Default) |
| Simulation | C++ mit pybind11 → Python |
| Steps/Sekunde (Training) | ~330 Steps/s bei 8 parallelen Envs |
| Determinismus | Vollständig: seed → identische Welt |

---

## Relevante Dateien

| Datei | Inhalt |
|-------|--------|
| [src/core/simulation.cpp](../src/core/simulation.cpp) | Hauptschleife, Reward, BFS, Observation |
| [src/core/world.cpp](../src/core/world.cpp) | Weltgenerierung, Chunks, Noise, Biome |
| [src/core/object.cpp](../src/core/object.cpp) | Tile-Objekte, Mining-Eigenschaften |
| [src/core/recipe.cpp](../src/core/recipe.cpp) | Crafting-Rezepte |
| [assets/base/game_config.json](../assets/base/game_config.json) | Alle Spielparameter |
| [python/stoneforge_env.py](../python/stoneforge_env.py) | Gym-Wrapper, Observation, Curriculum |
