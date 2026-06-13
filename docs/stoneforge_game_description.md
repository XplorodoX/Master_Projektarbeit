# Stoneforge — Spielbeschreibung

Stand: 13.06.2026

---

## Was ist Stoneforge?

Stoneforge ist ein **2D-Sandbox-Spiel** mit prozedural generierten Welten,
geschrieben in C++ mit einem Python-Binding (pybind11) für RL-Experimente.
Die Welten werden bei jedem Start aus einem zufälligen Seed neu generiert —
keine zwei Karten sind identisch.

Das Spiel läuft als reine Simulation (kein Grafik-Modus nötig) und kann
über das Python-Binding als Gym-Environment angesteuert werden.

---

## Spielwelt

Die Welt ist ein **2D-Gitter** (theoretisch unbegrenzt, prozedural generiert).
Jede Zelle ist genau ein Tile-Typ:

| Tile | Bedeutung |
|------|-----------|
| `Empty` | Begehbarer Boden / Luft |
| `Wall` | Wand / Fels — blockiert Bewegung |
| `Tree` | Baum — abbaubar, gibt Holz |
| `Ore` | Erz — abbaubar, gibt Erz |
| `Workbench` | Werkbank — für Crafting |
| `Exit` | Ausgang — Ziel des Agenten |
| `Water` | Wasser — passierbar, visuell |

Die Weltgenerierung nutzt **Perlin-Noise** (mehrere Schichten: Biom, Dichte,
Erz, Bäume) mit festen Salt-Werten. Ein Seed erzeugt deterministisch immer
dieselbe Welt.

### Biome

Drei Biom-Zonen (kalt, warm, Moos) entstehen aus dem Noise-Wert:

| Biom | Noise-Bereich | Charakter |
|------|--------------|-----------|
| Kalt | 0.0 – 0.33 | weniger Wände, wenig Bäume |
| Warm | 0.33 – 0.66 | mittlere Dichte |
| Moos | 0.66 – 1.0 | dicht, viele Bäume |

---

## Spielfigur (Agent / Spieler)

Der Spieler startet immer bei Koordinate **(0, 0)** (Spawnpunkt).
Der **Exit** wird zufällig in 35–45 Tiles Wegdistanz (BFS, nicht Manhattan)
um den Spawn platziert.

### Attribute

| Attribut | Startwert | Effekt |
|----------|-----------|--------|
| HP | 10 | 0 → Episode beendet |
| Energy | 100 | Sinkt beim Laufen, regeneriert bei Idle |
| Inventory | 24 Slots | Items aufnehmen (Holz, Erz, …) |

---

## Aktionen

Der Spieler (bzw. Agent) kann pro Schritt **eine** Aktion ausführen:

| Aktion | Effekt |
|--------|--------|
| `MoveUp` | Bewegt 1 Tile nach oben (y−1) |
| `MoveDown` | Bewegt 1 Tile nach unten (y+1) |
| `MoveLeft` | Bewegt 1 Tile nach links (x−1) |
| `MoveRight` | Bewegt 1 Tile nach rechts (x+1) |
| `Mine` | Baut Tile vor dem Spieler ab (benötigt mehrere Schritte) |
| `Place` | Platziert Item aus Hotbar |
| `Use` | Schlägt Mobs in 3×3-Umgebung |
| `Wait / Noop` | Nichts tun, Energy regeneriert |

Für RL-Training: Mining, Place, Use und Wait sind **deaktiviert** — der Agent
hat nur die 4 Bewegungsaktionen (`Discrete(4)`).

---

## Spielziel

> **Den Exit finden.**

Der Exit-Tile ist irgendwo in der Welt, 35–45 BFS-Tiles vom Spawn entfernt.
Der Agent sieht nur ein **lokales 15×15-Grid** um sich herum
(Sichtradius = 7 Tiles) — er sieht den Exit also nicht direkt,
solange er nicht nahe genug heran kommt.

Als Hilfe bekommt er den **Kompass** (exitDx, exitDy) — die Richtung zum Exit
in Welt-Koordinaten. Das ist ein Luftlinien-Vektor, kein Pfad.

---

## Was macht das Spiel für RL schwierig?

### 1. Partielle Beobachtbarkeit (POMDP)
Der Agent sieht nur 15×15 Tiles. Wände, Sackgassen und der Pfad dahinter
sind unsichtbar. Das macht Stoneforge zu einem **Partially Observable Markov
Decision Process** — der Agent muss sich merken, was er schon erkundet hat.

### 2. Maze-Navigation ohne Karte
Der kürzeste Pfad führt oft durch Korridore, die vom Kompass weg zeigen.
Eine reine „geh Richtung Exit"-Strategie scheitert an Wänden.

### 3. Prozedurale Generierung
Jede Episode ist eine neue, unbekannte Welt. Der Agent kann sich keine
Karte memorieren — er muss **generell** navigieren lernen.

### 4. Langer Horizont
Bis zu 4.000 Schritte pro Episode. Der relevante Reward (+100) kommt erst
am Ende — extrem sparse ohne Reward-Shaping.

---

## Technische Eckdaten

| Parameter | Wert |
|-----------|------|
| Sichtradius | 7 Tiles → 15×15 Grid |
| Observation-Größe | 231 Features |
| Max. Schritte/Episode | 4.000 |
| Exit-Distanz (Eval) | 35–45 Tiles (BFS) |
| Aktionsraum | Discrete(4) |
| Weltgenerierung | Perlin-Noise + Seed |
| Simulation | C++ (pybind11 → Python) |
| FPS (Training) | ~330 Steps/s (8 Envs parallel) |
