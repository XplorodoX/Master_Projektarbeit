---
id: e3-lstm512
title: E3 — größeres LSTM (256 → 512)
type: experiment
tags: [negativergebnis, gap-experiment, e3, architektur]
path: models/exp_E3_lstm512
related: [det-stoch-gap, recurrent-ppo, v12-final, literatur-lstm-groesse, e1-critic]
status: widerlegt
updated: 2026-07-17
---

# E3 — LSTM 512

**Hypothese:** Der Belief verschwimmt auf langen Wegen ([[det-stoch-gap]] skaliert mit der
Weglänge), also fehlt **Gedächtnis-Kapazität**. Ein doppelt so großes LSTM sollte helfen.

**Eingriff:** `lstm_hidden_size` 256 → **512**. Sonst v12-Konfiguration.
Vollständig durchgelaufen: 22 h 54 min, alle 4 Phasen.

## Ergebnis

| Testset | stoch | det |
|---------|-------|-----|
| A | 44 % | **14 %** |
| B | 58 % | 20 % |

Gegen v12 (A 73,3 / 32 · B 80 / 42): **klar schlechter, auf det am schwächsten.**

Und zwar nicht erst am Ende — phasenweise durchgängig unter der 256er-Baseline:
Phase 2 stoch-Peak 66 % gegen 80–88 %; Phase-3-det-Peak 30 % und **fallend**; die letzten 250k
Steps kollabieren.

## Bewertung — der wichtigste Negativbefund

Das ist die stärkste Zeile im Diskussionsteil, weil sie die naheliegendste Hypothese widerlegt:
**Mehr Gedächtnis-Kapazität ≠ besserer Gedächtnis-Nutzen.**

Der Flaschenhals ist nicht, wie viel das LSTM *behalten* kann, sondern wie gut es das Behaltene
über lange Horizonte *abrufen und nutzen* kann. Die Literatur deckt das:
[[literatur-lstm-groesse]] (diminishing returns, 512 optimal / 1024 schlechter in einer PPO-LSTM-
Studie) und "Memory Retention Is Not Enough" (arXiv 2601.15086).

**Folge für den Ausblick — vorsichtiger, als hier lange stand:** Naheliegend wäre "nicht größer,
sondern **strukturierter** Speicher" (GTrXL, Neural Map, FFM, In-Context-RL wie AMAGO). Aber die
Literatur trägt das nur schwach: [[memory-rewriting-2026]] findet klassische rekurrente Modelle
**überlegen** gegenüber strukturierten Speichern und Transformern, [[popgym-2023]] sieht RNNs
konkurrenzfähig. Als **offene Richtung** ausweisen, nicht als sichere Lösung. Siehe
[[literatur-lstm-groesse]] — die früher hier zitierte "512-optimal"-Studie war ein
Aktienhandels-Paper und ist raus.

**Belegkraft:** n=1. Ein voller Lauf, kontrolliert gegen die v12-Baseline, phasenweise durchgängig
schlechter — das ist ein solides Indiz, aber keine Seed-Statistik. So auch berichten.

Nebenbefunde: ~3–4× langsamer als die 256er-Baseline. Der Lauf litt unter Sleep-Stalls; `caffeinate -w`
hat sie behoben (danach ~48 fps). Bei Langläufen also **mit** `-w` starten — siehe
[[nohup-training]].

CHANGELOG `v2026-07-08.3`.
