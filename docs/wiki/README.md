# Stoneforge RL — LLM-Wiki (OKF-Bundle)

Kompiliertes Projektwissen im Open-Knowledge-Format-Muster: ein Verzeichnis atomarer
Markdown-Einträge mit YAML-Frontmatter, querverlinkt über `[[entry-id]]`.

**Zweck:** Ein Agent (oder ein Mensch) soll nach dem Lesen dieses Bundles den Projektstand
kennen, ohne 1.900 Zeilen CHANGELOG durchzuarbeiten — inklusive der Sackgassen, denn die
Negativergebnisse sind hier ein Kernbeitrag, kein Abfall.

## Einstieg in 5 Minuten

1. [Problemstellung & Zielkriterium](concepts/zielkriterium.md) — was gilt als Erfolg
2. [Stoneforge als POMDP](concepts/pomdp-charakter.md) — die zentrale These der Arbeit
3. [Det/Stoch-Gap](concepts/det-stoch-gap.md) — der Diskussionskern
4. [Ablation A→B→C](experiments/ablation-abc.md) — die Narrativlinie
5. [v12: Endergebnis](experiments/v12-final.md) — die Zahlen, die in die Arbeit gehen
6. [Aktueller Stand](projekt-status.md) — was gerade läuft, was offen ist

## Karte

| Sammlung | Was drin steht |
|----------|----------------|
| [concepts/](concepts/) | POMDP, Det/Stoch-Gap, PBRS, Curriculum, Observation Space, Recurrent PPO, Zielkriterium |
| [components/](components/) | Gym-Env, C++-Core, Trainingsskript, Eval-Protokoll, Swarm-Pool, Live-Map, Demo |
| [experiments/](experiments/) | Ablation, v2, v7–v9-Rootcause, v11-Env-Bruch, v12, E1/E2/E3, Temperatur-Sweep |
| [decisions/](decisions/) | batch_size=8, stochastische Eval, Eval-Cap 4000, kein größeres LSTM |
| [pitfalls/](pitfalls/) | Obs-Shape-Legacy, prozessglobale Config, Rebuild-Pflicht, nohup, Testset-Leakage |
| [references/](references/) | Singh 1994, Ghosh 2021, Pleines 2022, Henderson 2018, Pineau 2021 |

## Konventionen

- **Ein Eintrag = eine Sache.** Wer zwei Themen erklären will, macht zwei Dateien und verlinkt.
- **Zahlen immer mit Env-Version.** Vor/nach [v11](experiments/v11-env-bruch.md) sind Ergebnisse
  nicht vergleichbar — derselbe Seed erzeugt eine andere Welt. Nie in dieselbe Tabelle.
- **Frontmatter-Feld `status`** bei Experimenten: `bestätigt` · `widerlegt` · `läuft` · `historisch`.
- **Der CHANGELOG gewinnt.** Dieses Wiki ist die kompilierte Sicht, nicht die Quelle. Bei
  Widerspruch: `CHANGELOG.md` prüfen und den Eintrag hier korrigieren.

## Pflege

Nach jedem Experiment: CHANGELOG schreiben (Pflicht laut CLAUDE.md), **dann** den betroffenen
Wiki-Eintrag nachziehen und `updated:` setzen. Neue Erkenntnis, die keinen bestehenden Eintrag
hat → neue Datei, von mindestens einem bestehenden Eintrag aus verlinken. Verwaiste Einträge
sind tot.
