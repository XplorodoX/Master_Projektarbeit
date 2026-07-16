---
id: stochastic-deterministic-minds-2025
title: "Stochastic Policies, Deterministic Minds (2025) — der Auslöser des Temperatur-Sweeps"
type: reference
tags: [literatur, evaluation, temperatur, gegenprobe]
url: https://thesai.org/Downloads/Volume16No10/Paper_99-Stochastic_Policies_Deterministic_Minds.pdf
venue: "IJACSA (International Journal of Advanced Computer Science and Applications), Vol. 16, No. 10, 2025"
verified: 2026-07-17
related: [temperatur-sweep, det-stoch-gap, stochastische-eval, singh-1994]
updated: 2026-07-17
---

# "Stochastic Policies, Deterministic Minds" (2025)

**Untertitel:** *A Calibrated …* — IJACSA, Vol. 16, No. 10 (2025).

Diese Quelle **fehlte im Wiki**, obwohl der CHANGELOG sie zweimal nennt (`v2026-07-08.3` und
`v2026-07-09`). Nachgetragen beim Faktencheck 17.07.2026.

## Warum sie wichtig ist

**Sie ist der Auslöser des [[temperatur-sweep]]s.** Der CHANGELOG sagt das ausdrücklich: Die Frage
"Ist der Gap teilweise nur eine schlechte Wahl der Eval-Politik? Zwischen argmax und vollem Sampling
könnte eine kalibrierte Temperatur liegen, die beide schlägt" kam aus dieser Arbeit.

Das Paper beobachtet, dass Deep RL Agenten typischerweise **stochastisch trainiert und
deterministisch evaluiert** werden, und untersucht Softmax-Temperaturen T ∈ {0.1, 0.5, 1.0, 2.0, 10.0}
auf Held-out-Seeds. Befund dort: In manchen Umgebungen (z. B. Qbert) schlägt die **deterministische**
Politik das stochastische Standardvorgehen deutlich.

## Der methodische Wert für die Arbeit

Es ist die **Gegenhypothese**, und genau deshalb gehört es zitiert. Die Kette wird dadurch sauber:

1. Die Literatur legt nahe, dass eine kalibrierte Zwischentemperatur beide Extreme schlagen könnte.
2. Das Projekt hat das **getestet** statt es anzunehmen ([[temperatur-sweep]]).
3. Ergebnis: monoton — für **diese** Umgebung gibt es kein Zwischenoptimum.

Das ist der Unterschied zwischen "wir haben stochastisch gewählt, weil es besser aussieht" und
"wir haben die naheliegende Alternative geprüft und sie trägt hier nicht". Ohne dieses Zitat wirkt
[[stochastische-eval]] wie eine Setzung; mit ihm ist es eine beantwortete Frage.

Dass das Paper Fälle findet, in denen deterministisch gewinnt, ist **kein Widerspruch** zu
[[singh-1994]]: Qbert ist weitgehend vollständig beobachtbar, Stoneforge nicht. Der Kontrast stützt
die POMDP-These sogar — die Eval-Wahl hängt an der Beobachtbarkeit der Umgebung, nicht an der
Bequemlichkeit.

## ⚠️ Einordnung der Quelle

IJACSA ist ein **Journal mit deutlich geringerem Renommee** als die anderen hier zitierten Venues
(ICML, NeurIPS, ICLR, AAAI). Entsprechend gewichten: als **Motivation für ein eigenes Experiment**
zitieren — nicht als tragenden Beleg. Die Beweislast trägt der eigene Sweep, nicht diese Arbeit.

## To-do

Genauen Volltitel und Autorenliste aus dem PDF ergänzen (oben nur der Kurztitel gesichert), dann in
`docs/references.bib` aufnehmen.
