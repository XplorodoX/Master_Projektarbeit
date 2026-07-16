---
id: pleines-2022
title: Pleines et al. (2022) — Generalization, Mayhems and Limits in Recurrent PPO
type: reference
tags: [literatur, recurrent-ppo, grenzen]
bibkey: pleines2022recurrent
url: https://arxiv.org/abs/2205.11104
venue: "arXiv 2205.11104, eingereicht 23.05.2022"
authors: "Marco Pleines, Matthias Pallasch, Frank Zimmer, Mike Preuss"
verified: 2026-07-17
related: [recurrent-ppo, e3-lstm512, det-stoch-gap, literatur-lstm-groesse, popgym-2023, batch-size-8]
updated: 2026-07-17
---

# Pleines et al. (2022) — Grenzen von Recurrent PPO

**Vollständig:** Marco Pleines, Matthias Pallasch, Frank Zimmer, Mike Preuss: *Generalization,
Mayhems and Limits in Recurrent Proximal Policy Optimization.* arXiv:2205.11104, eingereicht
23.05.2022. (Verifiziert 17.07.2026.)

## Was das Paper belegt (Abstract, wörtlich)

> "we highlight vital details that one must get right when adding recurrence to achieve a correct
> and efficient implementation, namely: properly shaping the neural net's forward pass, arranging
> the training data, correspondingly selecting hidden states for sequence beginnings and masking
> paddings for loss computation. We further explore the limitations of recurrent PPO by benchmarking
> the contributed novel environments **Mortar Mayhem** and **Searing Spotlights** that challenge the
> agent's memory **beyond solely capacity and distraction tasks**. Remarkably, we can demonstrate a
> **transition to strong generalization in Mortar Mayhem when scaling the number of training seeds**,
> while the agent **does not succeed on Searing Spotlights**, which seems to be a tough challenge for
> memory-based agents."

## ⚠️ Korrektur vom 17.07.2026

Die frühere Fassung listete **drei** Kernaussagen, darunter *"Trainings-Instabilität ist ein
bekanntes, dokumentiertes Phänomen"* — und nutzte das, um die eigene Instabilität
([[batch-size-8]], Critic-Kollaps) als literaturgedeckt darzustellen.

**Das steht so nicht im Abstract.** Das Paper handelt von Implementierungsdetails der Rekurrenz und
von zwei neuen Benchmark-Umgebungen. Es behandelt Instabilität nicht als seinen dokumentierten
Kernbefund. Die Zuordnung war eine Überdehnung.

**Konsequenz:** Der Critic-Kollaps bei `batch_size=64` ist durch die **eigene Forensik** belegt
([[batch-size-forensik]]: 4 Läufe + A/B, EV ≈ 0,1 gegen 0,939) — und das ist ein solider Beleg. Er
braucht diese Fremdreferenz nicht und darf sie nicht bekommen.

## Was das Paper tatsächlich für die Arbeit trägt

| Befund im Projekt | Deckung bei Pleines | Belastbar? |
|---|---|---|
| Generalisierung braucht Seed-Vielfalt | "transition to strong generalization … when scaling the number of training seeds" | ✓ direkt |
| Recurrent PPO stößt an Grenzen, die nicht nur Kapazität sind | Umgebungen "beyond solely capacity"; Scheitern auf Searing Spotlights | ✓ sinngemäß |
| Trainings-Instabilität | — | ✗ **nicht** hierüber belegen |

Die mittlere Zeile ist die wertvolle: Sie stützt [[e3-lstm512]]. Wenn schon die Autoren von
Recurrent PPO Umgebungen bauen, an denen ihre Agenten **jenseits reiner Kapazität** scheitern, ist
"mehr LSTM-Kapazität hat nicht geholfen" ein erwartbares, kein peinliches Ergebnis.

Die Implementierungsdetails (Hidden-State-Auswahl an Sequenzgrenzen, Padding-Masking) sind
nebenbei ein Beleg dafür, dass Recurrent PPO **schwer richtig zu bekommen** ist — passend zur
Projektgeschichte ([[v7-v9-rootcause]]), aber als Kontext, nicht als Entschuldigung.

## Weiterführend (Ausblick — mit Vorsicht)

- [[popgym-2023]] — größter Vergleich über RL-Gedächtnismodelle.
- [[memory-rewriting-2026]] — findet klassische rekurrente Modelle **überlegen** gegenüber
  strukturierten Speichern. **Widerspricht** einem naiven "Transformer statt LSTM"-Ausblick.
- **Benchmarking Partial Observability** (Tao, Guo, Allen, Konidaris; arXiv 2508.00046, RLC/RLJ 2025) —
  Best-Practice-Leitlinien für POMDP-Benchmarking, Bibliothek POBAX.
- **AMAGO** (Grigsby, Fan, Zhu; arXiv 2310.09971, ICLR 2024, `grigsby2024amago`) — In-Context-RL über
  lange Sequenzen, explizit auch für **prozedural generierte** Umgebungen. Von den
  Ausblick-Referenzen die passendste.
