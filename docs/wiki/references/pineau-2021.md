---
id: pineau-2021
title: Pineau et al. (2021) — Improving Reproducibility in ML Research (ML Reproducibility Checklist)
type: reference
tags: [literatur, methodik, reproduzierbarkeit, checkliste]
bibkey: pineau2021repro
url: https://arxiv.org/abs/2003.12206
jmlr: https://www.jmlr.org/papers/v22/20-303.html
venue: "JMLR 22(164), S. 1–20, 2021 (arXiv 2003.12206, 2020)"
authors: "Joelle Pineau, Philippe Vincent-Lamarre, Koustuv Sinha, Vincent Larivière, Alina Beygelzimer, Florence d'Alché-Buc, Emily Fox, Hugo Larochelle"
verified: 2026-07-17
related: [reproduzierbarkeit, henderson-2018, train-curriculum, rebuild-pflicht]
updated: 2026-07-17
---

# Pineau et al. (2021) — ML Reproducibility Checklist

**Vollständig:** Joelle Pineau, Philippe Vincent-Lamarre, Koustuv Sinha, Vincent Larivière, Alina
Beygelzimer, Florence d'Alché-Buc, Emily Fox, Hugo Larochelle: *Improving Reproducibility in Machine
Learning Research (A Report from the NeurIPS 2019 Reproducibility Program).* Journal of Machine
Learning Research 22(164), S. 1–20, 2021. arXiv:2003.12206 (2020). (Verifiziert 17.07.2026.)

Der Report beschreibt das NeurIPS-2019-Reproduzierbarkeitsprogramm mit drei Komponenten:
Code-Submission-Policy, community-weite Reproducibility Challenge und die **ML Reproducibility
Checklist** als Teil des Einreichungsprozesses.

## ⚠️ Zahlenkorrektur vom 17.07.2026

Die frühere Fassung behauptete: *"Nach Einführung stieg der Anteil **reproduzierbarer Paper** von
~50 % auf über 75 %."*

**Das ist falsch.** Gestiegen ist die **Code-Beilegungsquote**, nicht die Reproduzierbarkeit:

| | Wert |
|---|---|
| Angenommene Paper mit Code-Link, NeurIPS **2018** | ~50 % |
| Angenommene Paper mit Code-Link bei finaler Einreichung, NeurIPS **2019** | **75 %** |
| Autoren, die schon **zum Einreichungszeitpunkt** einen Code-Link angaben (2019) | ~40 % |

Der Unterschied ist nicht kosmetisch: "Code liegt bei" und "Ergebnis ist reproduzierbar" sind
verschiedene Dinge — Code-Verfügbarkeit ist eine **Voraussetzung**, kein Nachweis. Die alte
Formulierung hätte die Wirkung des Programms überzeichnet, und zwar in einem Abschnitt, in dem es
gerade um methodische Redlichkeit geht. Peinlich, wenn ausgerechnet dort eine Zahl frisiert ist.

## Was angegeben werden muss

| Anforderung | Status im Projekt |
|---|---|
| Verwendete Daten / Seeds | ✓ Val 6000–6049, Testset A 7000–7049, Holdout B 8000–8049, disjunkt |
| Exakte Hyperparameter der berichteten Zahlen | ✓ `config.json` je Lauf |
| **Durchsuchter** Hyperparameter-Bereich (nicht nur der Gewinner) | ✓ über den CHANGELOG |
| Anzahl Random Seeds | ✓ n=3 berichtet, Aufstockung auf n=7 läuft |
| Rechen-Infrastruktur | ✓ Systemumgebungs-Tabelle im Anhang |

Der dritte Punkt wird meist übersehen und ist hier ein unerwarteter Trumpf: Der **CHANGELOG
dokumentiert jede Änderung mit Problem/Lösung/Ergebnis inklusive aller Sackgassen** (v7–v10,
E1/E2/E3). Das deckt "durchsuchter Bereich" besser ab als die übliche Gewinner-Tabelle — und ist
ohnehin schon geschrieben.

## Umsetzung in der Arbeit

Reproducibility-Anhang in `docs/Projektdokumentation.tex`: Systemumgebungs-Tabelle +
Pineau/Henderson-Checkliste (kompiliert, exit 0). Ergänzt um einen Punkt, den die
Standardcheckliste nicht kennt: Der C++-Build ist Teil der Umgebung ([[rebuild-pflicht]]), deshalb
Git-Commit-Hash je Lauf.

Details und offene Punkte: [[reproduzierbarkeit]]. BibTeX-Key `pineau2021repro`.
