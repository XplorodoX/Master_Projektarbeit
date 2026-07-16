---
id: ablation-abc
title: Ablation A→B→C — die Narrativlinie der Arbeit
type: experiment
tags: [ablation, kernbeitrag, narrativ]
related: [observation-space, recurrent-ppo, pbrs-reward-shaping, det-stoch-gap, v11-env-bruch]
status: bestätigt
updated: 2026-07-17
---

# Ablation A → B → C

Die Dramaturgie der Arbeit in drei Zeilen. Sie beantwortet die Frage, die ein Prüfer zuerst
stellt: *Löst hier das RL das Problem oder ein Algorithmus, den ihr reingereicht habt?*

| Bedingung | Architektur | BFS in Obs | Curriculum | SR stoch | SR det |
|-----------|-------------|------------|------------|----------|--------|
| **A** `ppo_phase4` | MLP | ✓ (6 Features) | ✓ exit 5→45 | 32 % | **0 %** |
| **B** `ppo_no_bfs` | MLP | ✗ | ✗ (nur exit 5–12) | — | **0 %** (kollabiert) |
| **C** `ppo_lstm_curriculum` | LSTM | ✗ | ✓ exit 5→45 | **86 %** | 36 % |

## Die Aussage

- **A:** Mit BFS-Distanz in der Observation ist die Aufgabe lösbar — aber das ist kein echtes RL,
  sondern ein Agent, der einem Orakel folgt. Bezeichnend: deterministisch **0 %**; er hat nie
  gelernt zu navigieren, nur zu folgen.
- **B:** Nimmt man das BFS weg und lässt das MLP allein, **kollabiert** es. Das BFS war also nicht
  Beiwerk, sondern Voraussetzung.
- **C:** Ein LSTM ohne BFS-Orakel löst die Aufgabe zu 86 %. **Gedächtnis ersetzt das Orakel.**

Der Kernsatz der Arbeit steht in Zeile C: *Gedächtnis schlägt Orakel.* Ein Agent mit Belief State
löst ein POMDP besser als einer, dem man die Antwort hinlegt — weil Letzterer nie gelernt hat, die
Frage zu stellen.

Wichtig: Das BFS ist nicht weg, es steckt im **Reward** ([[pbrs-reward-shaping]]) statt in der
Observation. Der Agent bekommt im Training Feedback, zur Testzeit kein Orakel.

## ⚠️ Die 86 % sind historisch

Sie stammen aus einem Einzellauf **vor** [[v11-env-bruch]], gemessen gegen das alte Env
(Luftlinien-Exit-Platzierung). Gegen Env v11 nachgemessen liefert dasselbe Modell 68 % stoch / 18 % det
— **kein Regressionsschaden**, sondern ein anderes Testset: Derselbe Seed erzeugt vor/nach `9a6b95d`
eine andere Welt.

**Für die Arbeit:** Die Ablationstabelle behält die 86 % als historischen Referenzlauf, **mit
Klarstellungssatz in der Caption** und Verweis auf die v12-Tabelle. Historische Zahlen und
v12-Zahlen dürfen **nie in dieselbe Tabelle**. Aktuelle Zahlen: [[v12-final]].
