---
id: singh-1994
title: Singh, Jaakkola & Jordan (1994) — Learning Without State-Estimation in POMDPs
type: reference
tags: [literatur, pomdp, kernzitat]
bibkey: singh1994pomdp
url: https://www.cs.utexas.edu/~shivaram/readings/b2hd-SinghJJ1994.html
venue: "ICML 1994 (Proc. 11th Int. Conf. on Machine Learning), S. 284–292, Morgan Kaufmann"
authors: "Satinder P. Singh, Tommi Jaakkola, Michael I. Jordan"
verified: 2026-07-17
related: [pomdp-charakter, det-stoch-gap, stochastische-eval, temperatur-sweep, recurrent-ppo]
updated: 2026-07-17
---

# Singh, Jaakkola & Jordan (1994)

**Vollständig:** Satinder P. Singh, Tommi Jaakkola, Michael I. Jordan: *Learning Without
State-Estimation in Partially Observable Markovian Decision Processes.* ICML 1994, S. 284–292,
Morgan Kaufmann. (Verifiziert 17.07.2026.)

**Kernaussage:** Der Ansatz nimmt **stochastische Policies in den Suchraum auf** und zeigt, dass die
optimale Policy im POMDP **häufig stochastisch** ist — sie kann erheblich besser sein als jede
deterministische. Im MDP ist stets eine deterministische Policy optimal; im POMDP nicht.

## Warum das das wichtigste Zitat der Arbeit ist

Es verwandelt den größten Angriffspunkt in einen Befund. Ohne dieses Resultat sieht "73 %
stochastisch, 32 % deterministisch" aus wie ein kaputtes Modell plus günstige Metrikwahl. Mit ihm
ist der [[det-stoch-gap]] die **erwartete Signatur der Problemklasse**, und [[stochastische-eval]]
folgt aus der Theorie statt aus Bequemlichkeit.

Der [[temperatur-sweep]] liefert die empirische Entsprechung: Die monotone Kurve ist genau das, was
man erwartet, wenn Stochastizität Teil der guten Policy ist und nicht Rauschen darauf.

## ⚠️ Zwei Fallstricke beim Zitieren

**1. Es gibt zwei Singh/Jaakkola/Jordan-Arbeiten von 1994.** Nicht verwechseln:

| | Venue | Autorenreihenfolge |
|---|---|---|
| *Learning Without State-Estimation in POMDPs* ← **diese hier** | ICML 1994, S. 284–292 | Singh, Jaakkola, Jordan |
| *Reinforcement Learning Algorithm for Partially Observable Markov Decision Problems* | NIPS 7 (1994) | **Jaakkola**, Singh, Jordan |

Beide behandeln stochastische Policies in POMDPs, beide werden für dieselbe Aussage zitiert. Der
BibTeX-Key `singh1994pomdp` meint die **ICML**-Arbeit.

**2. Das Resultat gilt für *memoryless* Policies — unser Agent hat ein LSTM.**

Das ist die wichtigere Feinheit und sie gehört in die Arbeit, weil ein aufmerksamer Prüfer sie
findet. Singh et al. betrachten Policies **ohne Zustandsschätzung** (daher der Titel). Unser Agent
ist nicht memoryless: Er approximiert über den LSTM-Hidden-State einen Belief. **Wäre dieser Belief
perfekt**, wäre der Belief-State-MDP wieder ein MDP — und dort wäre argmax optimal.

Die Übertragung ist also kein Automatismus, sondern ein **Argument über den Grad der
Approximation**: Unser Agent liegt zwischen memoryless und perfektem Belief. Je unvollständiger der
Belief, desto mehr greift Singhs Regime. Genau dazu passt der Befund, dass der Gap **mit der
Weglänge wächst** ([[det-stoch-gap]]) — auf kurzen Wegen ist der Belief gut (det 79 %), auf langen
verschwimmt er (det 31 %), und dort wird stochastisches Handeln wertvoll.

**So formulieren** — nicht: "Singh 1994 beweist, dass unsere stochastische Eval korrekt ist."
Sondern: "Singh 1994 zeigt, dass Determinismus im POMDP seine Optimalitätsgarantie verliert, sobald
der Zustand nicht vollständig geschätzt wird; unser LSTM leistet diese Schätzung nur näherungsweise,
und der weglängenabhängige Gap ist damit konsistent."

Ergänzend deckt [[ghosh-2021]] den Fall ab, dass die Unsicherheit gar nicht aus der Sicht, sondern
aus der Generalisierung stammt — dort greift das Argument unabhängig vom Gedächtnis.

## Verwendung

BibTeX-Key `singh1994pomdp` in `docs/references.bib`, integriert in `Projektdokumentation.tex`,
§"Det/Stoch-Gap und POMDP-These". Zusammen mit [[ghosh-2021]] die zwei Kernzitate.
