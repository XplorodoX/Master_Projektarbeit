---
id: wissenschaftliche-doku-aufbau
title: Aufbau einer wissenschaftlichen Projektarbeit (Leitfaden)
type: concept
tags: [dokumentation, schreiben, hochschule, roter-faden, formalia]
related: [zielkriterium, projekt-status, det-stoch-gap, v12-final]
status: bestätigt
created: 2026-07-25
updated: 2026-07-25
sources:
  - https://www.bachelorprint.de/projektarbeit/gliederung-aufbau-projektarbeit/
  - https://gwriters.de/blog/imrad-schema
  - https://www.tu-chemnitz.de/informatik/ce/files/Leitfaden-Bachelor-Arbeit.pdf
  - https://www.hs-aalen.de/theses
  - https://www.bachelorhero.de/wissensdatenbank/hausarbeit/roter-faden-hausarbeit
---

# Aufbau einer wissenschaftlichen Projektarbeit

Recherchiert und angewendet am 25.07.2026 beim Umbau von
`docs/Projektdokumentation.tex`. Enthält das, was tatsächlich umgesetzt wurde,
nicht nur Theorie.

## 1 — Reihenfolge der Bestandteile (Pflicht)

Diese Reihenfolge ist über Hochschulen hinweg praktisch identisch:

| # | Bestandteil | Seitenzahl | Pflicht? |
|---|-------------|-----------|----------|
| 1 | **Deckblatt** | keine | ja |
| 2 | Kurzfassung / Abstract | römisch (ii) | ja |
| 3 | **Inhaltsverzeichnis** | römisch | ja |
| 4 | Abbildungsverzeichnis | römisch | wenn Abbildungen |
| 5 | Tabellenverzeichnis | römisch | wenn Tabellen |
| 6 | Abkürzungs-/Symbolverzeichnis | römisch | wenn Fachbegriffe |
| 7 | **Hauptteil** (Einleitung → Fazit) | **arabisch, beginnt bei 1** | ja |
| 8 | Literaturverzeichnis | arabisch | ja |
| 9 | Anhang | arabisch | optional |
| 10 | **Eidesstattliche Erklärung** | meist ohne | ja |

**Fallstrick:** Die Seitennummerierung wechselt. Vorspann römisch (I, II, III …),
Hauptteil arabisch neu ab 1. In LaTeX:

```latex
\pagenumbering{roman}\setcounter{page}{2}   % nach dem Deckblatt
...
\pagenumbering{arabic}\setcounter{page}{1}  % vor \section{Einleitung}
```

Verzeichnisse, die selbst keine `\section` sind, müssen manuell ins
Inhaltsverzeichnis: `\addcontentsline{toc}{section}{Abbildungsverzeichnis}`.

### Deckblatt — Pflichtangaben

Hochschule · Fakultät · Studiengang · Art der Arbeit („Projektarbeit") · Titel ·
ggf. Untertitel · Verfasser · Betreuer · Abgabedatum. Keine Seitenzahl.

## 2 — Gliederung des Hauptteils: IMRaD

IMRaD = **I**ntroduction, **M**ethods, **R**esults **a**nd **D**iscussion. Das
Standardschema empirischer Arbeiten, in MINT universell verstanden.

| IMRaD | Kapitel hier | Leitfrage |
|-------|--------------|-----------|
| Introduction | Einleitung | *Was* wurde untersucht und **warum**? |
| (Theorie) | Grundlagen + Verwandte Arbeiten | Was muss man **wissen**, um mitzukommen? |
| Methods | Umgebung + Methodik + Technische Umsetzung | **Wie** wurde untersucht? |
| Results | Evaluation | Was wurde **gefunden**? (neutral, ohne Deutung) |
| Discussion | Fazit und Ausblick | Was **bedeutet** das? |

**Grundlagen gehören vor Methodik, nicht in die Einleitung.** In Bachelor-/
Projektarbeiten ist ein eigenes Theoriekapitel Standard; in Papern steckt es in
der Introduction. Nicht verwechseln.

**Gliederungstiefe: maximal drei Ebenen.** Und: eine Unterüberschrift braucht
mindestens eine Schwester — wer 2.1 hat, braucht 2.2.

### Was die Einleitung enthalten muss

1. **Motivation** — warum ist das relevant? (weckt Interesse, kein Fachjargon)
2. **Ausgangssituation / die Aufgabe anschaulich** — der Leser braucht ein
   mentales Bild, *bevor* Theorie kommt
3. **Problemstellung und Forschungsfrage** — als Frage formuliert, hervorgehoben
4. **Zielsetzung / Erfolgskriterien** — woran wird gemessen?
5. **Aufbau der Arbeit** — Kapitel für Kapitel, ein Satz je Kapitel

Punkt 5 ist das wichtigste einzelne Element für den roten Faden und wird am
häufigsten vergessen.

## 3 — Roter Faden: konkrete Techniken

Der rote Faden ist kein Stilmittel, sondern eine Struktur aus wiederkehrenden
Bauteilen. Die vier, die am meisten bringen:

### a) Sandwich-Prinzip (Kapitelklammer)

Jedes Kapitel wird von zwei kurzen Meta-Absätzen eingerahmt:

- **Vorschau am Anfang:** Was kommt in diesem Kapitel, in welcher Reihenfolge,
  warum diese Reihenfolge.
- **Zwischenfazit am Ende:** Was hat dieses Kapitel zur Forschungsfrage
  beigetragen (1–2 Sätze) + Überleitung zum nächsten.

Beispiel aus der Doku (Ende Grundlagen → Verwandte Arbeiten):

> **Zwischenfazit.** Damit sind die Bausteine benannt: Die Aufgabe ist wegen des
> begrenzten Sichtfelds ein POMDP, weshalb eine Politik mit Gedächtnis nötig
> ist […]. Bevor dieser Aufbau im Detail beschrieben wird, ordnet das nächste
> Kapitel ihn in bestehende Forschungsumgebungen ein.

### b) Metakommunikation

Passagen, in denen man **über die Arbeit** spricht, nicht über das Thema. Sie
rechtfertigen die Struktur, ohne Inhalte zu wiederholen. Besonders wertvoll,
wenn etwas an einer ungewöhnlichen Stelle steht:

> „Der letzte Abschnitt begründet, warum die Erfolgsschwellen revidiert wurden;
> er steht bewusst hier und nicht in der Einleitung, weil die Begründung
> Kenntnis der Umgebung voraussetzt."

### c) Vorwärts- und Rückverweise

Konsequent `\label`/`\ref` statt „siehe oben". Erlaubt dem Leser, an jeder
Stelle abzubiegen, ohne den Faden zu verlieren.

### d) Leitbegriffe konstant halten

Ein Begriff, ein Wort. Nicht abwechselnd „Erfolgsquote", „SR", „Trefferrate".
Beim ersten Auftreten definieren, dann durchhalten, und ins
Abkürzungsverzeichnis.

## 4 — Kurzfassung / Abstract

| Regel | Wert |
|-------|------|
| Länge | **150–250 Wörter** (½ bis 1 Seite) |
| Aufbau | Kontext → Fragestellung → Methode → **Ergebnis** |
| Zeitform | Präsens oder Perfekt |
| Verboten | Zitate, Quellenangaben, Literaturdiskussion, Abkürzungen ohne Einführung |
| Stil | kurze Aktivsätze |

**Der häufigste Fehler:** Der Abstract erzählt die Geschichte der Arbeit statt
ihr Ergebnis. Er muss allein lesbar sein und die Zahlen nennen.

*In diesem Projekt:* Der Abstract war 392 Wörter lang und enthielt Zitate →
gekürzt auf 192 Wörter, Zitate raus, Ergebniszahlen rein.

## 5 — Verständlichkeit für Fachfremde

Die Prüfungsfrage lautet nicht „ist es korrekt?", sondern „kann ein Leser ohne
Vorwissen folgen?". Drei Regeln:

1. **Bild vor Formel.** Erst anschaulich beschreiben, was passiert, dann
   formalisieren. Ein `\begin{quote}`-Kasten mit der Aufgabe in Alltagssprache
   kostet eine halbe Seite und trägt das ganze Dokument.
2. **Jede Abkürzung bei erster Nennung ausschreiben** — auch die vermeintlich
   bekannten (RL, PPO, MDP). Zusätzlich ins Abkürzungsverzeichnis.
3. **Fachbegriffe in Klammern übersetzen:** „Reinforcement Learning (RL,
   bestärkendes Lernen)", „Success Rate (Erfolgsquote)".

## 6 — Checkliste vor Abgabe

- [ ] Deckblatt vollständig (Hochschule, Fakultät, Studiengang, Betreuer, Datum)
- [ ] Kurzfassung 150–250 Wörter, ohne Zitate, mit Ergebniszahlen
- [ ] Inhalts-, Abbildungs-, Tabellen-, Abkürzungsverzeichnis vorhanden
- [ ] Seitenzahlen: Vorspann römisch, Hauptteil arabisch ab 1
- [ ] Einleitung enthält Motivation, Aufgabe anschaulich, Forschungsfrage, Ziele, **Aufbau der Arbeit**
- [ ] Jedes Kapitel hat Vorschau + Zwischenfazit
- [ ] Gliederungstiefe ≤ 3, keine einzelne Unterüberschrift ohne Schwester
- [ ] Alle Abbildungen/Tabellen im Text referenziert und mit aussagekräftiger Caption
- [ ] Eidesstattliche Erklärung mit Unterschriftsfeldern
- [ ] `pdflatex` → `bibtex` → `pdflatex` ×2, **0 undefined references**

## 7 — Projektspezifisch: was hier umgesetzt wurde

Umbau am 25.07.2026, `Projektdokumentation.tex` 30 → 47 Seiten:

- Deckblatt (vorher nur `\maketitle`)
- Kurzfassung 392 → 192 Wörter
- Inhalts-, Abbildungs-, Tabellen- und Abkürzungsverzeichnis ergänzt
- Seitennummerierung römisch/arabisch getrennt
- „Zielsetzung" → vollwertige **Einleitung** mit 5 Unterabschnitten, darunter
  [Die Aufgabe in Kürze](#) (Stoneforge in Alltagssprache) und
  „Aufbau der Arbeit"
- 6 Kapitelklammern (Zwischenfazit + Überleitung) eingefügt
- Kapitel-Vorschauen für Grundlagen, Methodik, Evaluation
- Eidesstattliche Erklärung

**Bewusst nicht gemacht:** keine Umstellung der Kapitelreihenfolge. Die
Reihenfolge Grundlagen → Verwandte Arbeiten → Umgebung entspricht der
Konvention; das Verständnisproblem („man weiß noch nicht, worum es geht")
wurde stattdessen über den Abschnitt „Die Aufgabe in Kürze" in der Einleitung
gelöst. Das ist der billigere und weniger fehleranfällige Eingriff.

## Siehe auch

- [Zielkriterium](zielkriterium.md) — die Erfolgsschwellen und ihre Revision
- [Projekt-Status](../projekt-status.md) — aktueller Stand
