---
id: biome
title: Biome — 7 Zonen per 1/7-Binning
type: concept
tags: [spiel, worldgen, biome, fallstrick]
path: src/core/world.cpp (biomeTagForChunk, sampleBaseTile)
verified: 2026-07-17
related: [weltgenerierung, tile-typen, stoneforge-spiel, doku-worldgen-veraltet]
updated: 2026-07-17
---

# Biome

Die Welt zerfällt in **sieben Biome**. Die Einteilung ist ein **striktes Binning** des
kontinuierlichen Biom-Rauschens in feste Intervalle von je **1/7** (`World::biomeTagForChunk`,
`world.cpp` Z. ~173, verifiziert 17.07.2026):

```cpp
int World::biomeTagForChunk(int cx, int cy) const {
    const double n = biomeFieldForChunk(cx, cy);
    // 0=grasland, 1=wald, 2=wueste, 3=bergland, 4=steppe, 5=tundra, 6=hoelle
    if(n < 1.0 / 7.0) { return 0; }
    if(n < 2.0 / 7.0) { return 1; }
    ...
```

| Tag | Biom |
|-----|------|
| 0 | Grasland |
| 1 | Wald |
| 2 | Wüste |
| 3 | Bergland |
| 4 | Steppe |
| 5 | Tundra |
| 6 | Hölle |

Das Biom wird **pro Chunk** bestimmt, nicht pro Tile — daher `biomeFieldForChunk(cx, cy)`. Innerhalb
eines Bioms steuern **handgetunte, biomspezifische Threshold-Profile** das Spawnen von Wänden, Erzen
und Vegetation (`sampleBaseTile`, Z. ~412 ff.).

## ⚠️ Fallstrick: die Schwellen sind hartkodiert

Die Biom-Grenzen sind **nicht über `game_config.json` tunebar**. Wer die Biom-Verteilung ändern
will, muss `world.cpp` anfassen und **neu bauen** ([[rebuild-pflicht]]).

Das ist der Fallstrick, den `CLAUDE.md` explizit nennt — und er hat eine Vorgeschichte: Früher gab
es Konfigurations-Keys `coldBiomeMax`, `warmBiomeMax` und `mossBiomeMax`. Die waren **tot**: Der
Parser las sie ein, aber die harte 1/7-Logik ignorierte sie. Wer daran drehte, sah keine Wirkung.

**Inzwischen sind sie entfernt** (verifiziert 17.07.2026): `grep` über `src/` und `assets/` findet
sie nur noch in `assets/base/game_config.json.bak` — einer Backup-Datei.

> Das Aalen-Dokument beschreibt sie noch als "werden zwar vom Konfigurations-Parser eingelesen,
> sind jedoch … funktional obsolet" (§ 4.3, als *technologische Schuld* geführt). Das war der
> Stand **vor** dem Aufräumen — die Schuld ist **beglichen**, die Keys sind weg. Siehe
> [[doku-worldgen-veraltet]].

## Warum striktes Binning und nicht weiche Übergänge

Sieben gleich breite Bins auf einem geglätteten Rauschfeld sind die simpelste denkbare Lösung — und
für die Aufgabe ausreichend: Die Biome sind **kosmetisch plus Hindernis-Dichte**, sie tragen keine
Spielmechanik. Weiche Übergänge (Blending) hätten Aufwand gekostet, ohne dass jemand etwas davon
hätte. Dass die Chunk-Granularität die Grenzen kantig macht, wird durch das Domain Warping der
darunterliegenden Rauschfunktion optisch abgemildert ([[weltgenerierung]]).
