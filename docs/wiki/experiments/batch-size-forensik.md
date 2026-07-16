---
id: batch-size-forensik
title: Die Forensik vom 07.07. — wie batch_size=64 überführt wurde
type: experiment
tags: [forensik, methodik, lehre, negativergebnis]
related: [batch-size-8, swarm-seed-pool, eval-cap-4000, recurrent-ppo]
status: bestätigt
updated: 2026-07-17
---

# Forensik 07.07.2026

Die v11-Läufe waren instabil: SR oszillierte zwischen 10 % und 74 % ohne Konvergenz,
`explained_variance ≈ 0.1` — der Critic lernte nie. Die Frage war: **Wer ist schuld?**

CHANGELOG `v2026-07-07.4`.

## Systematisch entlastet

Der Reihe nach wurden alle Verdächtigen ausgeschlossen:

- **Swarm** ([[swarm-seed-pool]]) — Diagnose-Arm `..._v11_s1_noswarm`: entlastet.
- **StreamWrapper** — Arm `..._v11_s1_nostream`: entlastet.
- **Penalties** — entlastet.
- **Curriculum-Stack** — sogar **bit-identisch** zum nackten Lauf verifiziert.

Übrig blieb `batch_size=64` → [[batch-size-8]].

## Die eigentliche methodische Lehre

Die frühere Freigabe von `batch_size=64` beruhte auf **einem einzelnen Eval-Snapshot**
(52 % SR, `explained_variance` 0.72). Der sah gut aus. Er war aber nur ein zufällig günstiger Punkt
eines **chaotisch schwankenden Prozesses** — kein Zustand, sondern ein Moment.

> **Einzelne Eval-Snapshots sind keine Validierung.** Die Lerndynamik kann chaotisch sein;
> transiente Hochphasen sehen aus wie Erfolg. Hyperparameter-Entscheidungen nur auf Basis ganzer
> Eval-**Kurven** treffen.

Kontrast: Die v10-Reproduktion mit `batch_size=8` hatte `explained_variance = 0.939` — durchgehend,
nicht punktuell.

Diese Lehre ist teuer erkauft (mehrere Läufe) und deshalb heute im `CLAUDE.md` als Fallstrick
verankert. Sie ist auch der Grund, warum `eval_history.json` je Lauf geschrieben wird: Kurven statt
Punkte.

## Zeitgleich aufgedeckt

Das [[eval-cap-4000]]-Artefakt. Ein Teil der beobachteten "Stagnation" war gar nicht real, sondern
Messfehler durch zu kleine Eval-Caps. Der Phase-3-Kollaps dagegen war real.

Zwei überlagerte Fehler — einer im System, einer im Messgerät. Das ist der Grund, warum die
Diagnose so lange gedauert hat.
