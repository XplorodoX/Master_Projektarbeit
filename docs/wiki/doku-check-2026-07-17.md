---
id: doku-check-2026-07-17
title: Doku-Gegenprüfung — sind die Falschangaben in der Arbeit gelandet?
type: project
tags: [qualitaet, protokoll, doku, korrekturen]
verified: 2026-07-17
related: [faktencheck-2026-07-17, pleines-2022, literatur-lstm-groesse, ng-1999, pineau-2021, memory-rewriting-2026, e3-lstm512, projekt-status]
updated: 2026-07-17
---

# Doku-Gegenprüfung 17.07.2026

Nach dem [[faktencheck-2026-07-17]] die Anschlussfrage: Sind die im Wiki gefundenen Falschangaben
auch schon in `docs/Projektdokumentation.tex` gelandet?

**Antwort: fast keine. Die Arbeit ist deutlich sauberer als das Wiki es war.** Eine Stelle muss
korrigiert werden, eine ist Geschmackssache — und ein "Fehler" war keiner, sondern mein Irrtum.

## ✅ Entwarnung — nie in der Doku angekommen

| Falschangabe im Wiki | In der Arbeit? |
|---|---|
| Pineau-Statistik "reproduzierbare Paper 50 → 75 %" | **Nein.** Pineau wird nur als Checklisten-Referenz genannt (Z. 406, 779), ohne die Zahl. |
| "512 optimal, 1024 schlechter" (Aktienhandels-Paper, arXiv 2212.02721) | **Nein.** Nicht zitiert. |
| "Memory Retention Is Not Enough" (arXiv 2601.15086), falsch gelesen | **Nein.** Gar nicht zitiert. |
| Pleines-"Trainings-Instabilität" als Fremdbeleg | **Nein.** Nur **ein** Pleines-Zitat, und nicht dafür. |
| Gap-Overclaim "kein Hyperparameter-Problem" | **Nein** — die Arbeit ist hier **besser als das Wiki war**: Z. 729–731 nennt ausdrücklich "Nicht-architektonische Hebel wie Hilfs- bzw. Repräsentationsverluste oder gezielt mehr Langdistanz-Training bleiben ergänzend denkbar". Genau der Caveat aus dem CHANGELOG. |

## ❌ Mein Fehler: [[ng-1999]] war nie eine Lücke in der Arbeit

Der Faktencheck meldete "Ng 1999 fehlt komplett". **Das galt nur fürs Wiki.** Die Arbeit hat:

- `ng1999policy` in `docs/references.bib` (Z. 140)
- Zitiert in `Projektdokumentation.tex` **Z. 176 und Z. 291** — beide Male im PBRS-Kontext,
  einmal explizit mit "…ohne die optimale Politik zu verändern".

Der Eintrag [[ng-1999]] ist korrigiert. **Nichts zu tun.**

## 🔴 Die eine Stelle, die zu korrigieren ist: Fehlzuschreibung an Pleines

`Projektdokumentation.tex`, **Z. 696–697** (Abschnitt zu E1/E2/E3):

> "…mehr Gedächtnis-*Kapazität* bedeutet nicht mehr Gedächtnis-*Nutzen*, konsistent mit den
> **bekannten abnehmenden Grenzerträgen rekurrenter Netze** \cite{pleines2022recurrent}."

**Pleines et al. (2022) zeigt das nicht.** Das Paper handelt von Implementierungsdetails der
Rekurrenz in PPO und von zwei neuen Benchmark-Umgebungen. Was im Abstract steht:

- Umgebungen, die das Gedächtnis "**beyond solely capacity** and distraction tasks" fordern
- Generalisierung springt um, wenn man die **Zahl der Trainings-Seeds** skaliert
- Der Agent **scheitert** auf Searing Spotlights

"Aufgaben, die über reine Kapazität hinausgehen" ist **nicht dasselbe** wie "mehr Kapazität bringt
abnehmende Grenzerträge". Die zweite Aussage steht dort nicht.

**Wie der Fehler entstand:** Die *Behauptung* ("bekannte abnehmende Grenzerträge") stammt aus der
verworfenen Aktienhandels-Quelle. Beim Schreiben wurde die schlechte Quelle ersetzt — aber durch
eine, die die Behauptung nicht trägt. Die Quelle wurde ausgetauscht, die Behauptung blieb stehen.
Das ist der subtilere Fehler: Ein echtes, passend klingendes Paper an einer Stelle, an der es nichts
beweist.

### Formulierungsvorschlag

Die eigene Messung trägt die Aussage bereits vollständig — die Fremdreferenz ist gar nicht nötig:

> "…mehr Gedächtnis-*Kapazität* bedeutet in dieser Umgebung nicht mehr Gedächtnis-*Nutzen*. Der
> Befund passt zu den bekannten Grenzen rekurrenter Politiken in prozeduraler Navigation, deren
> Gedächtnisanforderungen über reine Kapazität hinausgehen \cite{pleines2022recurrent}."

Damit sagt der Satz, was Pleines wirklich hergibt, und die Beweislast liegt bei E3 — wo sie
hingehört ([[e3-lstm512]], [[literatur-lstm-groesse]]).

## 🟡 Geschmackssache: der Ausblick

Der Ausblick ist **gut abgesichert**. Z. 764–766:

> "Transformer-basierte oder In-Context-Architekturen (z. B. AMAGO) sind die **meistgenannte, wenn
> auch aufwändigste Richtung**."

"Meistgenannt" ist genau die richtige Hedge — es behauptet keine Überlegenheit, sondern beschreibt
den Diskurs. Das hält auch gegen [[memory-rewriting-2026]] und [[popgym-2023]] stand, die
rekurrente Modelle konkurrenzfähig sehen.

Etwas stärker ist Z. 733: "das *harte*, fundamentale Langhorizont-Belief-Tracking ist jedoch **am
ehesten** über strukturiertes statt größeres Gedächtnis anzugehen". Auch gehedged, aber angesichts
der gemischten Evidenz wäre ein Halbsatz zur Gegenposition ein Gewinn — er kostet nichts und nimmt
einer Nachfrage die Spitze. **Optional.**

## 🟡 Kleinigkeit: Singh-Zitat ist präzise, die Folgerung minimal großzügig

Z. 667–670 zitiert Singh **korrekt mit dem entscheidenden Qualifier**:

> "…können in POMDPs stochastische **gedächtnislose** Politiken einen erheblich höheren erwarteten
> Ertrag erzielen als jede deterministische \cite{singh1994pomdp}."

Das ist besser als die meisten Zitate dieses Resultats. Der Folgesatz ("Der beobachtete Gap ist
**damit** die erwartbare Signatur…") überspringt nur, dass der eigene Agent eben **nicht**
gedächtnislos ist ([[singh-1994]]). Ein Zwischensatz — das LSTM approximiert den Belief nur, und
der weglängenabhängige Gap ist genau dazu konsistent — würde die Kette schließen. **Optional, aber
billig.**

## Fazit

Eine Pflichtkorrektur (Z. 696–697), zwei optionale Präzisierungen, sonst sauber. Die Doku hat die
Fehler des Wikis **nicht** geerbt — sie ist an mehreren Stellen sogar vorsichtiger formuliert. Das
Wiki war das schwächste Glied, nicht die Arbeit.
