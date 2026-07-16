---
id: memory-rewriting-2026
title: Shchendrigin et al. (2026) — Memory Retention Is Not Enough (KORRIGIERT)
type: reference
tags: [literatur, memory, korrektur, warnung]
bibkey: shchendrigin2026memory
url: https://arxiv.org/abs/2601.15086
venue: "arXiv 2601.15086, eingereicht 21.01.2026"
authors: "Oleg Shchendrigin, Egor Cherepanov, Alexey K. Kovalev, Aleksandr I. Panov"
verified: 2026-07-17
related: [e3-lstm512, det-stoch-gap, pleines-2022, recurrent-ppo]
status: korrigiert
updated: 2026-07-17
---

# Shchendrigin et al. (2026) — Memory Retention Is Not Enough

> ⚠️ **Dieser Eintrag korrigiert eine Fehldarstellung.** Bis zum 17.07.2026 wurde dieses Paper im
> Projekt (Memory-Notizen, Wiki) so zitiert, als belege es zwei Dinge, die es **nicht** belegt.
> Wer die alte Formulierung noch irgendwo findet: sie ist falsch.

## Was das Paper wirklich sagt

**Titel:** *Memory Retention Is Not Enough to Master Memory Tasks in Reinforcement Learning.*
Oleg Shchendrigin, Egor Cherepanov, Alexey K. Kovalev, Aleksandr I. Panov. arXiv:2601.15086,
eingereicht 21.01.2026.

Aus dem Abstract (verifiziert 17.07.2026):

> "Our experiments reveal that classic recurrent models, despite their simplicity, demonstrate
> **greater flexibility and robustness in memory rewriting tasks than modern structured memories**,
> which succeed only under narrow conditions, and transformer-based agents, which often fail beyond
> trivial retention cases."

Die Lücke, die das Paper adressiert, ist **Memory Rewriting** — das Überschreiben veralteter
Information, wenn sich die Umgebung ändert. Nicht "Abrufen über lange Horizonte".

## Die zwei Fehler, die hier gemacht wurden

| Behauptet (falsch) | Tatsächlich |
|---|---|
| "Flaschenhals ist das *Abrufen/Nutzen* über lange Horizonte" | Flaschenhals ist das *Überschreiben/Vergessen* bei Umgebungsänderung |
| Stützt den Ausblick "strukturierter Speicher statt LSTM" | **Widerspricht ihm** — klassische rekurrente Modelle schlagen strukturierte Speicher und Transformer |

Der erste Fehler ist eine Titel-Fehllesung: "Retention is not enough" wurde als "Behalten reicht
nicht, es fehlt das Abrufen" gelesen. Gemeint ist "Behalten reicht nicht, es fehlt das Vergessen".

Der zweite ist der gravierendere: Das Paper wurde als **Stütze** für eine Aussage zitiert, die es
**angreift**.

## Was das für die Arbeit bedeutet

Es ist trotzdem zitierfähig — nur mit anderer Rolle. Es stützt **nicht** den Ausblick, sondern
relativiert ihn. Zusammen mit [[popgym-2023]] ergibt sich ein ehrlicheres Bild:

> Strukturierter Speicher ist die **meistgenannte** Richtung für Langhorizont-Belief-Tracking, aber
> die Evidenz, dass er LSTMs tatsächlich schlägt, ist **gemischt**. Zwei Benchmark-Arbeiten finden
> klassische rekurrente Modelle konkurrenzfähig bis überlegen.

Das ist keine Schwächung der Arbeit — im Gegenteil. Es macht [[e3-lstm512]] (größeres LSTM hilft
nicht) interessanter: Wenn weder mehr LSTM-Kapazität **noch** die fancy Alternativen zuverlässig
helfen, ist das ein Hinweis darauf, dass das Problem tiefer liegt als in der Speicherarchitektur —
und genau das ist die POMDP-These ([[pomdp-charakter]]).

**Formulierungsvorschlag fürs Fazit:** Strukturierten Speicher als *offene, nicht als sichere*
Richtung ausweisen — mit Verweis darauf, dass aktuelle Benchmarks rekurrente Modelle nicht
abgeschrieben haben.
