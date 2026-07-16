---
id: faktencheck-2026-07-17
title: Faktencheck vom 17.07.2026 — Prüfprotokoll
type: project
tags: [qualitaet, protokoll, korrekturen]
related: [projekt-status, literatur-lstm-groesse, memory-rewriting-2026, testset-leakage, reproduzierbarkeit, eval-protokoll]
updated: 2026-07-17
---

# Faktencheck 17.07.2026

Vollständige Gegenprüfung des Wikis gegen (a) die Primärliteratur im Web und (b) CHANGELOG, Code
und `Projektdokumentation.tex`. **20 Fehler gefunden und korrigiert.** Dieses Protokoll dokumentiert
was geprüft wurde — auch das, was *nicht* abschließend geklärt werden konnte.

## Verifizierte Quellen (alle existieren, Angaben geprüft)

| Quelle | Status |
|---|---|
| [[singh-1994]] — Singh, Jaakkola, Jordan, ICML 1994, S. 284–292 | ✓ echt, Zitat trägt (mit Einschränkung, s. u.) |
| [[ghosh-2021]] — Ghosh, Rahme, Kumar, Zhang, Adams, Levine, NeurIPS 2021 | ✓ echt |
| [[pleines-2022]] — Pleines, Pallasch, Zimmer, Preuss, arXiv 2205.11104 | ✓ echt, eine Aussage war überdehnt |
| [[henderson-2018]] — Henderson, Islam, Bachman, Pineau, Precup, Meger, AAAI 2018 | ✓ echt |
| [[pineau-2021]] — Pineau et al., JMLR 22(164):1–20, 2021 | ✓ echt, Statistik war falsch |
| [[ng-1999]] — Ng, Harada, Russell, ICML 1999, S. 278–287 | ✓ echt — **fehlte komplett** |
| [[memory-rewriting-2026]] — Shchendrigin et al., arXiv 2601.15086 | ✓ echt, **war falsch dargestellt** |
| [[popgym-2023]] — Morad et al., ICLR 2023 | ✓ echt — neu ergänzt |
| [[stochastic-deterministic-minds-2025]] — IJACSA 16(10), 2025 | ✓ echt — **fehlte**, obwohl im CHANGELOG |
| AMAGO — Grigsby, Fan, Zhu, ICLR 2024, arXiv 2310.09971 | ✓ echt |
| Benchmarking Partial Observability — Tao, Guo, Allen, Konidaris, arXiv 2508.00046, RLC 2025 | ✓ echt |

**Keine erfundene Quelle gefunden.** Die verdächtigste (arXiv 2601.15086 mit einer 2026er-ID)
existiert — sie wurde nur falsch gelesen.

## Die vier gravierenden Fehler

1. **[[memory-rewriting-2026]] sagte das Gegenteil des Behaupteten.** Das Paper wurde als Stütze für
   den Ausblick "strukturierter Speicher statt LSTM" zitiert. Tatsächlich findet es klassische
   rekurrente Modelle **überlegen** gegenüber strukturierten Speichern und Transformern. Außerdem
   geht es um Memory *Rewriting*, nicht um *Abrufen über lange Horizonte*.
2. **[[testset-leakage]] war seit dem 11.06. behoben**, das Wiki nannte es offen und empfahl, es als
   Mangel zu beichten. `VAL_SEEDS = range(6000, 6050)` ist im Code. Das Wiki hätte die Arbeit
   **schlechter dargestellt, als sie ist**.
3. **Die "512-optimal"-LSTM-Studie ist ein Aktienhandels-Paper** (arXiv 2212.02721, Metriken:
   Cumulative Return, Sharpe Ratio). Für Navigations-RL nicht zitierfähig → [[literatur-lstm-groesse]].
4. **[[ng-1999]] fehlte** — Policy-Invarianz des PBRS wurde ohne Quelle behauptet, obwohl das die
   methodische Rechtfertigung der Reward-Konstruktion trägt.

## Weitere Korrekturen

| # | Was | Wo |
|---|---|---|
| 5 | Pineau-Statistik: nicht "reproduzierbare Paper 50→75 %", sondern **Code-Beilegungsquote** | [[pineau-2021]] |
| 6 | Berichtete ±Std sind **`ddof=0`**, nicht `ddof=1` (A wäre ± 8,3 statt ± 6,8) | [[eval-protokoll]], [[v12-final]] |
| 7 | **Bootstrap-95-%-CIs sind längst in der Doku**, standen als "offen" | [[reproduzierbarkeit]] |
| 8 | Git-Stamping überzeichnet: `v12_s1`/`E3` haben **kein** `_git_commit` | [[reproduzierbarkeit]] |
| 9 | n=7 mischt zwei Code-Stände (`97ab30d` vs. `3c8fa26-dirty`) | [[projekt-status]] |
| 10 | Gap-Overclaim — der CHANGELOG warnt **wörtlich** vor "kein Hyperparameter-Problem" | [[det-stoch-gap]] |
| 11 | `skip`-Flag vertauscht: `ppo_phase4` hat `skip=True`, nicht `ppo_no_bfs` | [[obs-shape-legacy]] |
| 12 | Gradientenschritte falsch (ohne `n_envs=16` gerechnet); Verhältnis 8× stimmt | [[batch-size-8]], [[v7-v9-rootcause]] |
| 13 | Pleines-"Trainings-Instabilität" nicht vom Abstract gedeckt | [[pleines-2022]] |
| 14 | Singh gilt für **memoryless** Policies — unser Agent hat ein LSTM | [[singh-1994]] |
| 15 | Zwei Singh-1994-Papers (ICML vs. NIPS) — Verwechslungsgefahr | [[singh-1994]] |
| 16 | Ablation A: CHANGELOG hat **100 % und 0 %** (verschiedene Bedingungen) | [[ablation-abc]] |
| 17 | B > A: bessere, belegte Erklärung in der Doku (A hat 38 % vs. 32 % Langweg-Welten) | [[v12-final]] |
| 18 | E3-`caffeinate`: `-w` hat die Stalls **behoben**; ~3–4× langsamer ergänzt | [[e3-lstm512]] |
| 19 | E2/E1: fehlende det-Werte + n=1-Belegkraft ergänzt | [[e2-curriculum-sanft]] |
| 20 | Temperatur-Sweep: exakte Tabelle + Hinweis 70,7 % vs. 73,3 % | [[temperatur-sweep]] |

## ⚠️ Nicht abschließend geklärt (vor Verwendung selbst nachschlagen)

Ehrlichkeit über die Grenzen dieser Prüfung — zwei Aussagen ließen sich nur im Abstract prüfen,
nicht im Volltext:

1. **[[ghosh-2021]]**: Dass die optimale Policy im epistemic POMDP **stochastisch** ist, steht nicht
   im Abstract. Über [[singh-1994]] gedeckt, aber wenn es *Ghosh zugeschrieben* wird: Stelle im
   Volltext suchen.
2. **[[popgym-2023]]**: "GRU ist das beste Allzweck-Gedächtnismodell" stammt aus einer Sekundärquelle
   und war im zugänglichen Text nicht wörtlich zu bestätigen.

## Woher die Fehler kamen

Fast alle stammen aus den **Session-Memories**, aus denen das Wiki kompiliert wurde — nicht aus
CHANGELOG oder Code. Die Memories enthielten Notizen aus Web-Recherchen (Domäne der Quelle nicht
mitgeprüft), Zuspitzungen ohne den Caveat des CHANGELOGs, und **veraltete Stände**: Der
`IMPROVEMENT_PLAN.md` listet Probleme vom 11.06., die noch am selben Tag behoben wurden.

**Regel daraus:** Notizen sind Hypothesen, nicht Fakten. Für das Wiki gilt die Reihenfolge
**Code → CHANGELOG → Doku → Primärquelle** — Memories nur als Fundstelle, nie als Beleg.
Bei jeder übernommenen Zahl gehört die **Domäne der Quelle** mit in die Notiz.
