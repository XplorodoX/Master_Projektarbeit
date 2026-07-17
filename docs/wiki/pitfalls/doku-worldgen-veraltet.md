---
id: doku-worldgen-veraltet
title: ⚠️ Doku-Abweichungen im Weltgenerierungs-Kapitel (Prüfung 17.07.2026)
type: pitfall
tags: [doku, korrektur, worldgen, spiel, kritisch]
verified: 2026-07-17
related: [weltgenerierung, biome, tile-typen, stoneforge-spiel, exit-platzierung, faktencheck-2026-07-17]
updated: 2026-07-17
---

# Doku-Abweichungen: Weltgenerierung

Prüfung des Aalen-Dokuments (`hs_aalen_projektdoku`, Kap. 4 "Konzept Prozedurale
Weltengenerierung", Stand 16.07.2026) gegen den Code am 17.07.2026.

**Das Kapitel ist überwiegend korrekt.** Aber es enthält **eine Aussage, die das Gegenteil des
Codes behauptet** — und die trägt eine ganze Schlussfolgerung. Zusätzlich weichen
`docs/stoneforge_game_description.md` an zwei Stellen ab.

> Hinweis: Das Aalen-Dokument liegt **nicht im Repo** (`find` am 17.07. ohne Treffer) und ist
> nicht identisch mit `docs/Projektdokumentation.tex` — Letzteres hat **kein**
> Weltgenerierungs-Kapitel. Es sind zwei getrennte Dokumente; wer sie pflegt, muss beide anfassen.

## 🔴 Der schwere Fehler: "Garantierter Pfad AKTIVIERT"

**Tabelle 4.1** behauptet:

| Feature | Status laut Doku | **Tatsächlich** |
|---|---|---|
| Garantierter Pfad (Manhattan) | **AKTIVIERT** (`game_config.json`, Z. 7) | **`forceGuaranteedPath: false`** (Z. **8**) |

Und § 4.3 zieht daraus den Schluss:

> "Durch diese Konfiguration arbeitet das System in der Produktion primär über den schnellen
> Manhattan-Fallback, während die rechenintensiveren graphenbasierten und zellulären
> Validierungsschritte für iterative Testphasen abgeschaltet sind."

**Das ist genau verkehrt herum.** Der Manhattan-Carve läuft **nie**:

```cpp
// world.cpp Z. 33–34
if(cfg.forceGuaranteedPath) { carveGuaranteedPath(); }     // false → nie
// world.cpp Z. 44–45
if(!reachable && cfg.guaranteedPathFallback) { carveGuaranteedPath(); }  // auch false → nie
```

`forceGuaranteedPath: false` (Z. 8) **und** `guaranteedPathFallback: false` (Z. 9). Die Funktion
`World::carveGuaranteedPath()` (Z. 699) ist toter Code im Produktionspfad.

**Warum das zählt:** Die Aussage dreht die Kernbotschaft des Kapitels um. Tatsächlich arbeitet das
System **nicht** über einen Fallback-Korridor, sondern über die **BFS-Exit-Platzierung**, die
Lösbarkeit per Konstruktion garantiert ([[exit-platzierung]]). Das ist die *bessere* Geschichte —
sauberer Entwurf statt Krücke. Die Doku verkauft die Arbeit hier unter Wert **und** ist falsch.

Ein Prüfer, der `game_config.json` öffnet, sieht `false` in Zeile 8.

## 🟡 Zeilenverweise in Tabelle 4.1

| Feature | Doku sagt | Tatsächlich |
|---|---|---|
| Cellular Smoothing | Z. 25 | **Z. 19** (`enableCellularSmoothing: false`) |
| FloodFill-Validation | Z. 25 | **Z. 23** (`enableFloodFillValidation: false`) |
| — | — | Z. 25 ist `enableMacroGraphPrecheck: false` (in der Doku nicht erwähnt) |

Der **Status** (DEAKTIVIERT) stimmt bei beiden ✓ — nur die Zeilen nicht.

## 🟡 § 4.3: Legacy-Parameter sind entfernt, nicht nur obsolet

Die Doku führt `coldBiomeMax` / `warmBiomeMax` als "technologische Schuld": *"werden zwar vom
Konfigurations-Parser eingelesen, sind jedoch … funktional obsolet"*.

**Verifiziert:** `grep` über `src/` und `assets/` findet sie nur noch in
`assets/base/game_config.json.bak`. Sie sind **entfernt**, der Parser liest sie nicht mehr. Die
Schuld ist beglichen → [[biome]].

## 🟡 `docs/stoneforge_game_description.md` — vier Fehler

Diese Datei ist die **unzuverlässigste Quelle im Repo**. Nicht ungeprüft übernehmen (genau das ist
beim ersten Anlegen der Wiki-Spieleinträge passiert → [[faktencheck-spiel-2026-07-17]]).

| Behauptung | Tatsächlich |
|---|---|
| Chunks sind **16 × 16** Tiles (3 Stellen) | **`kChunkSize = 8`** → 8 × 8 (`world.hpp:15`) |
| `Water` ist ein Tile-Typ | **Kein `Water` im `TileType`-Enum** — Wasser ist eine Maske (`lakeMaskAt`) → [[tile-typen]] |
| `Wall` ist "✅ abbaubar (langsam)", "Kein Tool: 0.1/Schritt (Wand braucht ~10 Schritte)" | **Wand ist NICHT abbaubar.** `WallObject` überschreibt `isMineable()` nicht → Default `false`. Der Fortschritt läuft voll und wird verworfen → [[tile-typen]] |
| Biome entstehen durch **"Perlin-Noise"** | **Value Noise** — vier skalare Eckwerte, zwei Lerps, keine Gradienten → [[weltgenerierung]] |

Was in dieser Datei **stimmt** (geprüft): die Spitzhacken-Werte 0,22 / 0,45 und die daraus
abgeleiteten Resource-Schrittzahlen (~28 / ~14 bei Härte 6,0) ✓, 7 Biome ✓, Spawn (0,0) ✓,
Exit 35–45 BFS ✓.

## 🔴 Nachtrag (3. Prüfrunde): Kapitel 0.1/0.2 — Formeln teils veraltet/falsch

Auch der Formelteil des Aalen-Dokuments wurde gegen den Code geprüft (17.07.2026):

| Formel | Doku sagt | Code sagt |
|---|---|---|
| Straf-Terme (0.7) | −0.05·Wand − 0.25·Wand≥2× − **0.15**·Positionsloop | **Wand-Penalties existieren nicht mehr** (`(void)moveBlocked` — v11 hat sie entfernt); Loop ist **−0.05**, nicht −0.15. Das ist der **vor-v11-Reward**. |
| Gesamt-Reward (0.1) | Step-Penalty + sparse + PBRS + Strafen | Es fehlt der **Explorations-Bonus +0.02** für neu betretene Zellen (`computeReward`, Z. 1467–1469) |
| Entropie-Annealing (0.12) | $c_e(k) = 0.01 - \frac{0.009}{\Delta_3}\min(k,\Delta_3)$ → Start **0.01** | Start ist **0.05** (= `ent_coef` aus Phase 1/2), Ende 0.001. Der Kommentar im Code erklärt ausdrücklich, warum 0.05 sein muss. Endwert der Doku-Formel (0.001) ✓ |

Korrekt sind: PBRS-Block (0.2)–(0.5) mit β=2.5, γ=0.999, Φ=−BFS/128, +0.02 netto pro Tile ✓ ·
Sparse Rewards +100/−10 (0.6) ✓ · −0.5·ΔHP und −0.04·Idle in (0.7) ✓ · PPO-Formeln (0.8)–(0.11)
mit ε=0.2, γ=0.999, λ=0.95, c_v=0.5 ✓.

**To-do für die Doku:** Straf-Term-Formel auf den v11-Stand bringen (nur noch Loop −0.05, ΔHP,
Idle + Explorations-Bonus), Annealing-Start auf 0.05 korrigieren. Und im Code den veralteten
Docstring von `EntropyAnnealingCallback` (Z. 240) gleich mit.

## ✅ Was geprüft und korrekt ist

Damit klar ist, dass das Kapitel im Kern solide ist:

- Smoothstep $S(t) = 3t^2 - 2t^3$ ✓ (`t * t * (3.0 - 2.0 * t)`)
- Seed + Salt + Mix-/Hash-Funktion, Value Noise, bilineare Interpolation ✓
- Domain Warping gegen Raster-Artefakte ✓
- Multi-Frequenz-Kombination ✓ (konkret `0.62 / 0.28 / 0.10`)
- **Sieben** Biome, striktes **1/7**-Binning ✓
- Wasser: `Score = 0.75·a + 0.25·b`, harter Cutoff **0.86** ✓ (Z. 314/317)
- Strukturen: **P = 0.10** pro Chunk ✓ (Z. 220), handgemachte **5 × 5**-ASCII-Matrizen ✓
- Exit-Kandidaten per **BFS-Traversierung im Distanzring** ✓
- Zellulärer Automat: Moore-Nachbarschaft, Birth/Survival-Regeln konfigurierbar ✓ (nur: aus)
- Freiradien um Spawn (2) und Exit (1) ✓

## To-do

1. **Tabelle 4.1 korrigieren** — "Garantierter Pfad: DEAKTIVIERT", Zeilen 8/19/23 richtigstellen.
2. **§ 4.3 neu schließen** — nicht "Manhattan-Fallback in Produktion", sondern: Lösbarkeit folgt aus
   der BFS-Exit-Platzierung, deshalb sind Carve und Validierung **bewusst** abgeschaltet.
3. Legacy-Parameter-Absatz streichen oder auf "entfernt" umschreiben.
4. `stoneforge_game_description.md`: Chunk-Größe und Water-Tile korrigieren.
