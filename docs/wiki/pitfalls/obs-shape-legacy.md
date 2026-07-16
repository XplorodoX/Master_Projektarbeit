---
id: obs-shape-legacy
title: Fallstrick — Obs-Shape-Mismatch bei alten Modellen
type: pitfall
tags: [fallstrick, kompatibilitaet, evaluation]
related: [observation-space, stoneforge-env, v11-env-bruch, eval-protokoll]
updated: 2026-07-17
---

# Obs-Shape-Mismatch

**Symptom:** `RecurrentPPO.load(...)` oder der erste `env.step()` wirft einen Shape-Fehler — das
Modell erwartet z. B. 231 Features, das Env liefert 229.

**Ursache:** Der Observation Space wurde im Projektverlauf mehrfach geändert
([[observation-space]]). Jedes Modell ist an die Shape gebunden, mit der es trainiert wurde.

## Lösung je Modellgeneration

Die maßgebliche Quelle ist `_OBS_DIM_KWARGS` in `python/stoneforge_env.py` (verifiziert 17.07.2026):

| Modell | Shape | kwargs |
|--------|-------|--------|
| aktuell (ab Env v11) | 229 | — (Default) |
| v2–v10 (vor 06.07.2026) | 231 | `include_energy_inventory=True` |
| `ppo_no_bfs` | 230 | `include_energy_inventory=True, use_step_frac=False` |
| `ppo_phase4` | 236 | `include_energy_inventory=True, use_last_action_reward=True` |
| v7–v9 | 249 | gescheiterte Läufe, keine Rekonstruktion nötig |

Der bequeme Weg: `env_kwargs_for_model(model)` leitet die kwargs aus der Shape des geladenen Modells
ab. **Alle fünf Layouts sind darüber ladbar** — "inkompatibel" ist keines.

> ⚠️ **Korrigiert 17.07.2026.** Die frühere Fassung hatte das `skip`-Flag **vertauscht** und
> `ppo_no_bfs` für inkompatibel erklärt. Tatsächlich steht in `scripts/eval_comparison.py`:
> `A_ppo_phase4 → skip: True` (Zeile 43), `B_ppo_no_bfs → skip: False` (Zeile 50). Also genau
> andersherum. Der Kommentar am `skip`-Flag ist zudem selbst veraltet ("inkompatibel mit aktueller
> 230-Feature-Env" — die Env hat 229).

## Die Falle hinter der Falle

Ein Modell **lädt** und **läuft** zu bringen heißt noch nicht, dass die Zahl vergleichbar ist. Das
alte 86-%-Modell lässt sich mit `include_energy_inventory=True` sauber gegen Env v11 evaluieren —
und liefert dann 68 %. Das ist kein Regressionsschaden, sondern ein **anderes Testset**
([[v11-env-bruch]]): Derselbe Seed erzeugt eine andere Welt.

Wer den Shape-Fehler behebt und die Zahl dann in eine Tabelle mit v12 schreibt, hat den
eigentlichen Fehler erst begangen.
