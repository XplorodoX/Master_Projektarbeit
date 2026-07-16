---
id: pineau-2021
title: Pineau et al. — ML Reproducibility Checklist (NeurIPS)
type: reference
tags: [literatur, methodik, reproduzierbarkeit, checkliste]
bibkey: pineau2021repro
url: https://arxiv.org/abs/2003.12206
venue: arXiv 2003.12206
related: [reproduzierbarkeit, henderson-2018, train-curriculum, rebuild-pflicht]
updated: 2026-07-17
---

# Pineau et al. — ML Reproducibility Checklist

Die NeurIPS-Checkliste. Nach ihrer Einführung stieg der Anteil reproduzierbarer Paper von ~50 % auf
über 75 % — sie ist der Grund, warum die folgenden Punkte heute als Mindeststandard gelten.

## Was angegeben werden **muss**

| Anforderung | Status im Projekt |
|---|---|
| Verwendete Daten / Seeds | ✓ A 7000–7049, B 8000–8049, disjunkt und fest |
| Exakte Hyperparameter der berichteten Zahlen | ✓ `config.json` je Lauf |
| **Durchsuchter** Hyperparameter-Bereich (nicht nur der Gewinner) | ✓ über den CHANGELOG |
| Anzahl Random Seeds | ✓ n=3, Aufstockung auf n=7 läuft |
| Rechen-Infrastruktur | ✓ Systemumgebungs-Tabelle im Anhang |

Der dritte Punkt wird meist übersehen und ist hier ein unerwarteter Trumpf: Der **CHANGELOG
dokumentiert jede Änderung mit Problem/Lösung/Ergebnis inklusive aller Sackgassen** (v7–v10,
E1/E2/E3). Das deckt "durchsuchter Bereich" besser ab als die übliche Gewinner-Tabelle — und ist
ohnehin schon geschrieben.

## Umsetzung in der Arbeit

Der Reproducibility-Anhang in `docs/Projektdokumentation.tex` besteht aus Systemumgebungs-Tabelle
plus Pineau/Henderson-Checkliste (kompiliert, exit 0). Ergänzt um Env-spezifische Punkte, die die
Standardcheckliste nicht kennt: der C++-Build ist Teil der Umgebung ([[rebuild-pflicht]]), deshalb
Git-Commit-Hash je Lauf.

Details und offene Punkte: [[reproduzierbarkeit]]. BibTeX-Key `pineau2021repro`.
