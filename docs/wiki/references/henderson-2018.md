---
id: henderson-2018
title: Henderson et al. (2018) — Deep Reinforcement Learning that Matters
type: reference
tags: [literatur, methodik, reproduzierbarkeit]
bibkey: henderson2018matters
url: https://arxiv.org/abs/1709.06560
aaai: https://ojs.aaai.org/index.php/AAAI/article/view/11694
venue: "AAAI 2018 (32nd AAAI Conference on Artificial Intelligence), arXiv 1709.06560"
authors: "Peter Henderson, Riashat Islam, Philip Bachman, Joelle Pineau, Doina Precup, David Meger"
verified: 2026-07-17
related: [reproduzierbarkeit, v12-final, zielkriterium, pineau-2021, batch-size-8]
updated: 2026-07-17
---

# Henderson et al. (2018) — Deep RL that Matters

**Vollständig:** Peter Henderson, Riashat Islam, Philip Bachman, Joelle Pineau, Doina Precup, David
Meger: *Deep Reinforcement Learning that Matters.* AAAI 2018, arXiv:1709.06560.
(Verifiziert 17.07.2026. Joelle Pineau ist auch Erstautorin von [[pineau-2021]] — die beiden
Methodik-Quellen dieser Arbeit haben eine gemeinsame Autorin.)

**Kernkritik:** Deep-RL-Ergebnisse werden routinemäßig unsauber berichtet. Nicht-Determinismus in
Standard-Benchmarks plus die den Methoden eigene Varianz machen berichtete Ergebnisse schwer
interpretierbar.

- **Nie best-of-runs berichten.** Die Seed-Varianz ist massiv — der beste von fünf Läufen sagt
  nichts über die Methode.
- **Mehrere Seeds + Mittelwert ± Std**, möglichst mit Signifikanzaussage.
- **Quellen der Zufälligkeit dokumentieren:** Env-Reset, Env-Transitionen, Policy-Sampling,
  Gewichts-Initialisierung, (bei off-policy: Replay-Buffer).

## Was das Projekt schon erfüllt

Das ist der Teil, der im Text **aktiv verkauft** gehört, statt ihn stillschweigend richtig zu machen:

- ✓ **3 Seeds, Mittelwert ± Std** ([[v12-final]]) — genau Hendersons Forderung, kein Best-of.
  Aufstockung auf n=7 läuft ([[projekt-status]]).
- ✓ **`ddof=1`** — Stichproben-Std, nicht Grundgesamtheit.
- ✓ **Negativergebnisse berichtet** ([[e1-critic]], [[e2-curriculum-sanft]], [[e3-lstm512]],
  [[v7-v9-rootcause]]) statt nur der Gewinner-Konfiguration.

## Die unbequeme Zusatz-Lektion des Projekts

Henderson warnt vor Best-of-**runs**. Dieses Projekt hat die zeitliche Variante gelernt: Best-of-
**snapshots**. Die `batch_size=64`-Freigabe beruhte auf einem zufällig guten Eval-Punkt eines
chaotischen Prozesses ([[batch-size-forensik]]). Derselbe Fehler, nur über die Zeitachse statt über
Seeds — und genauso irreführend.

Daher gilt hier: Kurven statt Punkte, Seeds statt Läufe.

BibTeX-Key `henderson2018matters`. Siehe auch [[pineau-2021]].
