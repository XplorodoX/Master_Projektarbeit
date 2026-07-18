---
id: literatur-lstm-groesse
title: LSTM-Größe — was die Literatur trägt und was nicht (KORRIGIERT)
type: reference
tags: [literatur, architektur, lstm, korrektur, warnung]
verified: 2026-07-17
related: [e3-lstm512, recurrent-ppo, popgym-2023, memory-rewriting-2026, pleines-2022]
status: korrigiert
updated: 2026-07-17
---

# LSTM-Größe: was belegt ist — und was nicht

> ⚠️ **Korrigiert am 17.07.2026.** Die frühere Fassung stützte sich auf eine Quelle, die für diese
> Arbeit **nicht zitierfähig** ist. Details unten — **nicht** in die Arbeit übernehmen, ohne das
> gelesen zu haben.

## Die Quelle, die hier rausgeflogen ist

Behauptet wurde: *"Konkrete PPO-LSTM-Studie: 512 optimal, 1024 schlechter."*

Nachrecherche (17.07.2026): Diese Zahlen stammen aus **arXiv 2212.02721 — "A Novel Deep
Reinforcement Learning Based Automated Stock Trading System Using Cascaded LSTM Networks"**. Ein
**Aktienhandels-Paper**. Die Metriken hinter den Zahlen sind CR (Cumulative Return), MER und SR
(**Sharpe Ratio**) — Finanzkennzahlen, keine Success Rate.

**Warum das nicht geht:** Eine Hidden-Size-Ablation auf Börsenzeitreihen sagt nichts über
Gedächtnisbedarf in prozeduraler POMDP-Navigation. Die Domänen teilen die Buchstaben "LSTM" und
sonst nichts. Ein Prüfer, der der Fußnote nachgeht, findet ein Trading-Paper in einer
RL-Navigations-Arbeit — das kostet mehr Glaubwürdigkeit, als die Zahl je einbringen könnte.

## Was stattdessen trägt

**1. Die eigene Messung.** [[e3-lstm512]] ist der Beleg für 256 > 512 *in dieser Umgebung*:
A 44 % gegen 73 %, phasenweise durchgängig unter der Baseline, ein voller Lauf über 22 h 54 min.
Das ist eine kontrollierte Einzelmessung in der Zieldomäne und schlägt jede fachfremde Analogie.
Ehrlich dazusagen: **n=1**.

**2. Domänenrichtige Literatur** für die *allgemeine* Aussage, dass mehr/aufwändigerer Speicher
nicht automatisch besser ist:
- [[popgym-2023]] — größter Vergleich über RL-Gedächtnismodelle (13 Baselines, 15 POMDPs).
- [[memory-rewriting-2026]] — klassische rekurrente Modelle schlagen strukturierte Speicher und
  Transformer beim Memory Rewriting.
- [[pleines-2022]] — Grenzen von Recurrent PPO in prozeduraler Navigation.

## Die Aussage, die man damit machen darf

> **Mehr Gedächtnis-Kapazität ≠ besserer Gedächtnis-Nutzen.**

Belegt durch die eigene Messung, plausibilisiert durch die Benchmark-Literatur. Was man **nicht**
sagen darf: dass es eine allgemeine, quantitativ übertragbare Schwelle gäbe ("512 ist zu viel").
Die Schwelle ist task-spezifisch — das ist gerade der Punkt. Bei uns liegt sie bei 256.

## Lehre

Die alte Fassung entstand, weil eine Web-Recherche eine passend klingende Zahl fand und die
Domäne nicht mitgeprüft wurde. **Bei jeder übernommenen Zahl gehört die Domäne der Quelle mit in
die Notiz** — sonst wandert sie unbemerkt in die Arbeit.

## Derselbe Fehler ein zweites Mal: die „25–53 %"-Spanne (17.07.2026)

Die Doku behauptete: *"Publizierte Zero-Shot-Ergebnisse auf prozeduralen Labyrinthen liegen
typischerweise bei 25–53 % Success Rate"* — mit Verweis auf MiniGrid, Crafter und Procgen. Die
Zahl hatte **keine Provenienz** (kein Eintrag im CHANGELOG, keine konkrete Fundstelle) und fiel
beim Faktencheck durch:

| Zitierter Benchmark | Was er wirklich berichtet |
|---|---|
| **Procgen** | **normalisierten Return** $(R-R_{min})/(R_{max}-R_{min})$ in [0,1] — **keine Success Rate**. 0,25–0,53 normalisierter Return ≠ 25–53 % Erfolgsquote. |
| **MiniGrid** | aufgabenspezifische Success Rate — PPO-LSTM erreicht auf KeyCorridorS3R3 **100 %**. Widerspricht der Spanne direkt. |
| **Crafter** | Achievements/Reward, keine Labyrinth-Erfolgsquote. |

Die Spanne mischte also **drei Benchmarks mit drei verschiedenen Metriken** zu einer Zahl, die in
keiner der drei Quellen steht.

**Besonders gefährlich war, dass sie fast tragend wurde:** Sie sollte als Hauptargument dienen
("unser Wert liegt über dem publizierten Spektrum"), nachdem [[v12-final]] das Zielkriterium
verfehlte. Ein Prüfer, der nachfragt, hätte die Verteidigung an ihrer wichtigsten Stelle
zerlegt.

**Ersetzt durch** einen Absatz, der die Nicht-Vergleichbarkeit **explizit macht** (§ Einordnung in
verwandte Arbeiten): Unterschiedliche Metriken + unterschiedliche Sichtradien/Episodenlängen →
eine Aussage "über/unter dem Stand der Technik" wäre Scheinpräzision. Belastbar ist nur der
Vergleich **innerhalb** der Arbeit (Ablation, Gap-Experimente, eigene Kriterien) — gleicher
Generator, gleiche Metrik, gleiches Protokoll.

**Die verschärfte Regel:** Eine Zahl, die eine Verteidigung tragen soll, braucht eine
Primärquelle mit Seitenzahl — nicht drei Benchmark-Zitate in der Nähe.
