---
id: observation-space
title: Observation Space — 229 Features und ihre Geschichte
type: concept
tags: [env, observation, kompatibilitaet]
related: [stoneforge-env, obs-shape-legacy, v11-env-bruch, pomdp-charakter]
updated: 2026-07-17
---

# Observation Space

Aktuell (Env v11, seit 06.07.2026): **229 Features**. Gebaut wird der Vektor in
[[stoneforge-env]].

| Block | Größe | Inhalt |
|-------|-------|--------|
| Grid | 225 | 15×15 Tiles um den Agenten (`observationRadius=7`) |
| HP | 1 | Lebenspunkte |
| exitDx / exitDy | 2 | Richtungsvektor zum Exit (**nicht** die Position) |
| step_frac | 1 | Fortschritt in der Episode (Schritt / maxSteps) |

Kein BFS-Feature. Das ist Absicht und der eigentliche Beitrag — siehe [[ablation-abc]]: Ein Agent
mit BFS-Distanz in der Observation löst die Aufgabe, aber ohne echtes RL. Das Weglassen macht das
Problem erst interessant und erzwingt [[recurrent-ppo]].

## Warum exitDx/exitDy erlaubt bleibt

Es ist ein **Richtungs-**, kein Wegfeature. Es sagt "der Exit liegt grob dort drüben", nicht "geh
links um die Wand". Die Navigation um Hindernisse — der schwierige Teil — bleibt Aufgabe des
Agenten. Das ist der Grund, warum die Umgebung trotz dieses Features ein [[pomdp-charakter]] ist.

## Shape-Historie (kritisch beim Laden alter Modelle)

| Shape | Wer | Anmerkung |
|-------|-----|-----------|
| 236 | `ppo_phase4` | BFS-Features, veraltet; braucht `use_last_action_reward=True, action_buffer_len=1` |
| 230 | `ppo_no_bfs` | kein BFS, kein Step-Counter |
| 231 | v2–v10 | kein BFS + step_frac; Laden braucht `include_energy_inventory=True` |
| 249 | v7–v9 | gescheitert: extra visit_count + action_buffer_len=4 + last_reward |
| **229** | **ab Env v11, aktuell** | Energie + Inventar entfernt (tote Features), HP bleibt |

Der Sprung 231 → 229 kam mit [[v11-env-bruch]]: Energie und Inventar waren tote Features, seit
Mining/Kampf aus dem Binding entfernt sind. Fallstrick beim Laden alter Modelle:
[[obs-shape-legacy]].

Die 249er-Runde ist lehrreich: Alle "Fixes" (visit_count, action_buffer) haben die Observation
**vergrößert**, ohne das eigentliche Problem zu treffen — das lag bei den Hyperparametern
([[v7-v9-rootcause]]). Mehr Features waren die falsche Antwort auf eine falsch gestellte Frage.
