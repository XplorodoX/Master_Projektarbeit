---
id: pleines-2022
title: Pleines et al. (2022) — Generalization, Mayhems and Limits in Recurrent PPO
type: reference
tags: [literatur, recurrent-ppo, grenzen]
bibkey: pleines2022recurrent
url: https://arxiv.org/abs/2205.11104
venue: arXiv 2205.11104
related: [recurrent-ppo, e3-lstm512, det-stoch-gap, literatur-lstm-groesse]
updated: 2026-07-17
---

# Pleines et al. (2022) — Grenzen von Recurrent PPO

**Kernaussagen:**
- Recurrent PPO generalisiert erst mit **vielen Trainings-Seeds**.
- Es **scheitert jenseits der Gedächtnis-Kapazität**.
- **Trainings-Instabilität** ist ein bekanntes, dokumentiertes Phänomen.

## Warum das die Arbeit entlastet

Das Projekt reproduziert alle drei Punkte — und zwar unfreiwillig, aber lehrreich:

| Befund im Projekt | Deckung bei Pleines |
|---|---|
| [[det-stoch-gap]] wächst mit der Weglänge | Scheitern jenseits der Gedächtnis-Kapazität |
| Instabilität, Critic-Kollaps ([[batch-size-8]]) | dokumentierte Trainings-Instabilität |
| Generalisierung braucht Seed-Vielfalt | dito |

Das ist der Unterschied zwischen "unser Setup ist kaputt" und "wir stoßen an bekannte Grenzen der
Methode". Die Instabilität, die uns [[v7-v9-rootcause]] und die [[batch-size-forensik]] gekostet
hat, ist kein Einzelfall dieses Repos — sie ist eine Eigenschaft von Recurrent PPO in
prozeduraler Navigation.

## Weiterführend (Ausblick-Belege)

- **"Memory Retention Is Not Enough…"** (arXiv 2601.15086) + Memory-Maze-Benchmarks: Gedächtnis
  *Behalten* reicht nicht — der Flaschenhals ist *Abrufen/Nutzen* über lange Horizonte. Genau die
  Formulierung "der Belief verschwimmt auf langen Wegen".
- **Benchmarking Partial Observability** (arXiv 2508.00046).
- **AMAGO** (arXiv 2310.09971, `grigsby2024amago`), GTrXL, FFM, Neural Map — Stand der Technik für
  strukturierten Speicher statt größerem LSTM. Das ist der Ausblick nach [[e3-lstm512]].

BibTeX-Key `pleines2022recurrent`, in `docs/references.bib`.
