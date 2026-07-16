---
id: nohup-training
title: Fallstrick — Trainingsläufe immer mit nohup starten
type: pitfall
tags: [fallstrick, betrieb, training]
related: [train-curriculum, projekt-status]
updated: 2026-07-17
---

# nohup nicht vergessen

**Symptom:** Der Lauf stirbt **still** nach ein paar Minuten. Kein Fehler, kein Traceback — das
Log endet einfach.

**Ursache:** Ohne `nohup` hängt der Prozess am Terminal. Terminal zu → Prozess weg.

Passiert am 07.07.2026: Die ersten v12-Läufe (gestartet ~18:20) starben ~18:25 und niemand merkte
es, bis anderthalb Stunden später jemand ins Log schaute. Neustart um ~19:50 mit `nohup`.

## Richtig

```bash
nohup .venv/bin/python scripts/train_curriculum.py \
  --save-dir models/... --seed 1 --n-envs 16 --no-live-map \
  > logs/train_v12_seed1.log 2>&1 &
```

## Weitere Betriebs-Fallen bei Langläufen

- **`--save-dir` immer setzen.** Der Default `models/ppo_lstm_curriculum` ist das historische
  Referenzmodell — ohne eigenes Verzeichnis überschreibt man es.
- **Bei parallelen Läufen `--no-live-map --no-heatmap`**, sonst kollidiert Port 8766.
- **`OMP_NUM_THREADS=2`** — vier Läufe brauchen sonst mehr Kerne, als da sind.
- **`caffeinate` prüfen.** Es hat sich bei [[e3-lstm512]] (22 h) selbst terminiert. Bei Läufen über
  Nacht kontrollieren, ob der Mac tatsächlich wach bleibt.
- **Nach ~20k Steps Rauchtest** (`approx_kl ≈ 0.029`, `clip_fraction ≈ 0.25`) statt acht Stunden
  blind zu warten.

Ein 8-Stunden-Lauf, der nach 5 Minuten stirbt, kostet nicht 5 Minuten — er kostet die Nacht.
