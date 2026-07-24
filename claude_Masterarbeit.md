# CLAUDE.md — [Projektname]

Diese Datei ist der Einstiegspunkt für Claude (Chat oder Claude Code) in diesem Projekt. Sie beschreibt den Projektkontext, das generelle Vorgehen für eine Masterarbeit und **verpflichtet Claude dazu, parallel zur eigentlichen Arbeit sauber zu dokumentieren** — statt Wissen nur im Chat-Verlauf verpuffen zu lassen oder alles erst am Ende zusammenzuschreiben.

---

## Projektkontext

- **Projekt:** [Titel/Thema der Masterarbeit — noch offen]
- **Ziel:** [kurz beschreiben, sobald Thema steht]
- **Stack:** [z.B. Python/PyTorch, MLflow self-hosted, Podman/NAS — anpassen]
- **Betreuer/Ansprechpartner:** [Name]

---

## Grundprinzip: wissenschaftlich sauber + parallel dokumentiert

Zwei Dinge gehören zusammen und werden hier absichtlich nicht getrennt behandelt:

1. **Wissenschaftlicher Anspruch:** Das Vorgehen muss so dokumentiert sein, dass ein Dritter es nachvollziehen könnte — das ist am Ende auch die Grundlage für das Methodik-Kapitel.
2. **Praktischer Anspruch:** Drei Ebenen sauber trennen, damit nichts im Kopf hängen bleibt:
   - **Code & Experimente** — wo Ergebnisse entstehen
   - **Laufendes Log** — wo Entscheidungen und Verlauf chronologisch festgehalten werden
   - **Wissensbasis (Wiki)** — Literatur, Methoden, Konzepte, thematisch sortiert

**Wichtigste Regel dabei:** parallel dokumentieren, nicht nachträglich aufräumen. Auch Sackgassen gehören rein, nicht nur das, was am Ende funktioniert hat — die erklären später, warum Methode X statt Y gewählt wurde, und das braucht kein Mensch mühsam zu rekonstruieren.

---

## Wissenschaftliche Anforderungen (Checkliste, unabhängig vom Thema)

Das hier gilt, sobald das Thema steht — Claude sollte bei der Konzeption aktiv danach fragen/erinnern:

- **Forschungsfrage zuerst:** klar und präzise formulieren, keine vagen Fragen — die bestimmt später alles andere (Methode, Datenerhebung, Auswertung). Gehört als eigener `wiki/entscheidungen/`-Eintrag angelegt, sobald sie steht.
- **Methodik schon im Exposé mitdenken:** dient als Fahrplan für die ganze Arbeit, nicht erst im Nachhinein rechtfertigen.
- **Gütekriterien nennen und sicherstellen:** Objektivität, Reliabilität, Validität (bei empirischer Arbeit) bzw. Nachvollziehbarkeit/Reproduzierbarkeit (bei technischer/experimenteller Arbeit) — explizit im Methodik-Kapitel benennen, nicht implizit voraussetzen.
- **Ethik/DSGVO, falls mit Personen/personenbezogenen Daten gearbeitet wird:** informierte Einwilligung, Anonymisierung, Datenschutz von Anfang an mitdenken, nicht erst wenn Daten schon erhoben sind.
- **Literaturrecherche systematisch dokumentieren:** welche Datenbanken, welcher Zeitraum, nach jeder Sichtung sofort die Liste ergänzen (nicht sammeln und "später sortieren"). Bei einem Systematic Review als Methode: PRISMA-Ansatz (Suchstrategie, Ein-/Ausschlusskriterien, Screening-Prozess) verwenden und dokumentieren.
- **Forschungsdesign explizit begründen, nicht nur wählen:** warum quantitativ/qualitativ/Mixed Methods, warum dieses Sampling, was sind die Grenzen des Designs (Generalisierbarkeit). Auch das gehört in `wiki/entscheidungen/`.

---

## Zeitmanagement & Betreuung

- **Zwei Zeitpläne führen:** ein detaillierter interner (für dich, mit Puffer) und ein vereinfachter externer (für Betreuungsgespräche) — niemand braucht 47 Einzelpunkte im Gespräch mit dem Betreuer.
- **Puffer einplanen:** 33–50 % pro Phase, nicht nur am Ende. Der teuerste Fehler ist kein fehlender Puffer, sondern gar kein Zeitplan.
- **Betreuertermine als feste Meilensteine eintragen**, nicht lose "irgendwann mal fragen" — 3–4 Termine über die Laufzeit sind üblich, früh im Kalender festmachen.
- **Regelmäßig zum Betreuer gehen, auch bei Verzug** — gerade dann, nicht erst wenn alles fertig ist.
- **Backup nicht vergessen:** Git lokal reicht nicht — Remote-Repo (eigener Git-Server auf NAS oder zusätzlich privates GitHub/GitLab), damit ein Festplattenausfall nicht die ganze Arbeit kostet.

---

## Literaturverwaltung vs. Wiki — nicht verwechseln

`wiki/literatur/` ersetzt **keinen** Zitier-Manager, sondern ergänzt ihn:

- **Zotero** (kostenlos, Windows/Mac/Linux, Daten optional komplett lokal) für die eigentliche Referenzverwaltung, Zitierstil und automatisches Literaturverzeichnis im Enddokument.
- **`wiki/literatur/<autor-jahr-slug>.md`** für die inhaltliche Zusammenfassung/Bewertung einer Quelle — das `resource`-Feld verweist auf den Zotero-Eintrag (Link oder Citation-Key), nicht umgekehrt.

Beide Systeme parallel pflegen: Zotero für die Formalie (Zitat, Bibliographie), Wiki für "warum ist das relevant für mich".

---

## Projektstruktur (Code & Daten)

```
projekt/
├── CLAUDE.md
├── README.md                # Setup, wie man's zum Laufen bringt
├── wiki/                    # Wissensbasis, siehe Abschnitt weiter unten
├── data/
│   ├── raw/                 # unangetastet, read-only
│   ├── interim/             # Zwischenschritte
│   └── processed/           # fertige Datensätze
├── src/
│   ├── data/                # Preprocessing, Loader
│   ├── models/              # Architektur-Definitionen
│   └── train.py / eval.py
├── experiments/             # Configs pro Lauf
├── reports/
│   └── figures/             # Plots, die später 1:1 in die Arbeit wandern
└── mlruns/                  # MLflow-Tracking-Daten (lokal oder NAS)
```

Git von Tag 1 an, auch wenn's nur eine Person ist — die Commit-Historie ist ein kostenloses Zeitprotokoll der Entwicklung.

### Experiment-Tracking statt Excel-Tabelle

Für Experimente mit Parametern/Metriken: **MLflow**, wenn möglich self-hosted (z.B. auf NAS/Podman) statt Cloud-Abhängigkeit. Jeder Lauf loggt Hyperparameter, Metriken, Modell- und Datensatzversion automatisch. Der `resource`-Link in einem `wiki/experimente/`-Eintrag zeigt dann direkt auf den MLflow-Run.

---

## Grober Fahrplan (Phasen)

| Phase | Fokus | Was ins Log/Wiki wandert |
|---|---|---|
| 1. Einarbeitung | Thema/Literatur/Datensatz sichten, Exposé schärfen | `wiki/literatur/`, erste Forschungsfrage |
| 2. Konzept/Methodik | Architektur- und Methodenwahl | `wiki/entscheidungen/` füllt sich mit Begründungen |
| 3. Umsetzung/Experimente | Training, Tuning, Auswertung | `log.md` täglich, MLflow-Runs, Sackgassen dokumentieren |
| 4. Auswertung | Ergebnisse verdichten | Plots in `reports/figures`, Statistik |
| 5. Schreiben | Kapitel aus Wiki/Log destillieren | nur noch lesen, nicht neu schreiben |

Das Methodik-Kapitel macht in vielen Fächern 10–15 % des Textteils aus — die laufende Doku liefert dafür quasi Rohtext statt Mehrarbeit am Ende.

---

## Wiki-Pflicht: paralleles LLM-Wiki

Hintergrund: Google hat im Juni 2026 mit dem **Open Knowledge Format (OKF)** eine formale Konvention für genau das Muster veröffentlicht, das du (Flo) beim Stoneforge- und CampusNow-Projekt schon informell mit `CLAUDE.md`/Changelogs gemacht hast — ein Verzeichnis aus Markdown-Dateien mit kleinem YAML-Frontmatter-Block, das sowohl von Menschen lesbar als auch von Agenten direkt konsumierbar ist, ohne SDK oder Zusatztool. Diese CLAUDE.md formalisiert das für dieses Projekt.

**Kernregel für Claude:** Nach jeder inhaltlich relevanten Aktion (Paper gelesen, Experiment gefahren, Design-Entscheidung getroffen, Sackgasse entdeckt, Sensor/Tool konfiguriert) wird **automatisch** ein passender Wiki-Eintrag angelegt oder aktualisiert — nicht erst auf Nachfrage.

### Verzeichnisstruktur

```
wiki/
├── index.md                # Einstiegspunkt, Navigation zu allen Bereichen
├── log.md                  # Append-only, chronologisch: was wurde wann gemacht/entschieden
├── literatur/
│   ├── index.md
│   └── <autor-jahr-slug>.md
├── methoden/
│   ├── index.md
│   └── <methode-slug>.md
├── experimente/
│   ├── index.md
│   └── <experiment-id-slug>.md
├── entscheidungen/
│   ├── index.md
│   └── <entscheidung-slug>.md
├── datensaetze/
│   └── <dataset-slug>.md
└── glossar/
    └── <begriff-slug>.md
```

Ein Konzept = eine Datei. Der Dateipfad ist die Identität des Konzepts — keine IDs in einer separaten Datenbank nötig.

### Frontmatter-Konvention (jede Wiki-Datei)

Nur **`type`** ist zwingend, der Rest ist empfohlen, damit die Dateien durchsuchbar/filterbar bleiben:

```yaml
---
type: Paper | Methode | Experiment | Entscheidung | Datensatz | Tool | Konzept
title: 
description: 
resource: <Link/DOI/Pfad/Commit-Hash/MLflow-Run — die Quelle>
tags: []
timestamp: 2026-07-24T10:00:00Z
---
```

### Body-Struktur (Vorlage pro Eintrag)

```markdown
# <Titel>

## Kernaussage
Ein bis drei Sätze — worum geht's, warum relevant für die Arbeit.

## Details
Ausführlicher Inhalt, Formeln, Ergebnisse, Vergleiche.

## Bezug zur Arbeit
Wo genau fließt das ein? (Kapitel, Experiment, Entscheidung)

## Quellen
- Immer angeben — extern (Paper/DOI/URL) oder intern (Experiment-Run-ID, Commit-Hash, Log-Datum).
- Ohne Quelle kein Eintrag — das ist die wichtigste Regel hier.
```

Beispiel für einen `log.md`-Eintrag (chronologisch, append-only):

```markdown
## 2026-07-28
- Confocal-Sensor Datensatz erste Sichtung: 4200 Samples, 12% Anomalien
- Versuch: Autoencoder Latent-Dim 32 → Reconstruction Loss nicht aussagekräftig
  → vermutlich zu viel Kapazität, Modell rekonstruiert Anomalien zu gut
- Nächster Schritt: Latent-Dim runter auf 8, Vergleich morgen
- Quelle: experiments/run_013, run_014
```

### Verlinkung

Konzepte verlinken sich gegenseitig über normale Markdown-Links (`[Autoencoder](../methoden/autoencoder.md)`). Das macht aus dem Verzeichnis einen Graphen, der reichhaltiger ist als reine Ordner-Hierarchie. `index.md`-Dateien dienen der Navigation (progressive disclosure), `log.md` ist die einzige Datei, die rein chronologisch und append-only ist (nie umschreiben, nur ergänzen).

### Typen-Taxonomie für dieses Projekt (anpassen)

| type | wofür |
|---|---|
| `Paper` | Zusammenfassung + Bewertung einer gelesenen Quelle |
| `Methode` | Beschreibung einer verwendeten Methode/Architektur |
| `Experiment` | Ein Trainingslauf/Test mit Parametern und Ergebnis |
| `Entscheidung` | Design-Entscheidung mit Begründung (→ wird Methodik-Kapitel) |
| `Datensatz` | Beschreibung eines Datensatzes, Herkunft, Struktur |
| `Tool` | Setup-Notiz zu einem verwendeten Tool (z.B. MLflow-Config) |

---

## Regeln für Claude in diesem Projekt

1. **Wiki vor Antwort:** Bevor du eine inhaltliche Frage zum Projekt beantwortest, prüfe `wiki/index.md` und relevante Unterordner — nicht aus dem Gedächtnis raten, wenn's schon dokumentiert ist.
2. **Wiki nach Aktion:** Nach jedem Experiment/jeder Literaturrecherche/jeder Entscheidung: leg die passende Datei an oder aktualisiere sie, plus einen Einzeiler in `log.md`.
3. **Keine Quelle, kein Eintrag:** Jeder Wiki-Eintrag braucht ein `resource`-Feld oder einen Quellen-Abschnitt. Interne Quelle (Commit, Run-ID) zählt auch.
4. **Nichts löschen, nur versionieren:** Git trackt Code und Wiki mit — Verlauf ist Teil des Werts, nicht nur der aktuelle Stand.
5. **Minimal-invasiv:** Nur `type` ist Pflichtfeld. Lieber ein knapper Eintrag mit `type` + Quelle als gar keiner, weil das Format zu aufwändig wirkt.
6. **Sackgassen dokumentieren:** Ein Ansatz, der verworfen wurde, gehört genauso ins Log/Wiki wie ein erfolgreicher — das ist später Diskussions-/Limitations-Material.

---

## Nutzen fürs Schreiben später

- Methodik-Kapitel ← `wiki/entscheidungen/` + `wiki/methoden/`
- Stand der Forschung ← `wiki/literatur/`
- Ergebnisse ← `wiki/experimente/` (+ MLflow-Runs)
- Diskussion/Limitationen ← Sackgassen aus `log.md` und den Entscheidungs-Dateien

Alles reines Markdown — lesbar in jedem Editor, renderbar auf GitHub, greppable, kein Vendor-Lock-in, kein zusätzliches Tool nötig.

---

## Quellen / Hintergrund zu diesem Vorgehen

- Cookiecutter Data Science (Projektstruktur-Vorlage): https://cookiecutter-data-science.drivendata.org/
- Good enough practices in scientific computing (Wilson et al.): https://www.ncbi.nlm.nih.gov/pmc/articles/PMC5480810/
- Ten Simple Rules for Reproducible Research in Jupyter Notebooks: https://arxiv.org/pdf/1810.08055
- Best Practices for reproducible computational research (GenePattern): https://notebook.genepattern.org/best-practices/
- Best Practices for Developing Computational and Data-Intensive Applications: https://arxiv.org/pdf/2406.01780
- MLflow vs. Weights & Biases vs. ZenML: https://www.zenml.io/blog/mlflow-vs-weights-and-biases
- Experiment Tracking mit MLflow (Lamarr-Institut): https://lamarr-institute.org/blog/experiment-tracking/
- Methodik-Anforderungen an Masterarbeiten: https://www.1a-studi.de/masterarbeit/methodik, https://www.bachelorhero.de/wissensdatenbank/masterarbeit/forschungsdesign-masterarbeit
- Forschungsstand/Literaturrecherche systematisch dokumentieren (inkl. PRISMA): https://www.bachelorhero.de/wissensdatenbank/masterarbeit/forschungsstand-masterarbeit, https://www.studytexter.com/de/wissenschaftliche-arbeit/literaturrecherche-dokumentieren
- Gütekriterien, Ethik/DSGVO bei empirischer Arbeit: https://business-and-science.de/empirische-masterarbeit-schreiben/
- Zeitplanung & Betreuungsrhythmus: https://manuskriptmentor.com/blog/zeitplan-masterarbeit, https://akad-eule.de/blog/zeitplan-masterarbeit/
- Literaturverwaltung (Zotero vs. Citavi): https://www.bachelorhero.de/wissensdatenbank/masterarbeit/literaturverwaltung-masterarbeit
- Open Knowledge Format (OKF), Google Cloud Blog, 12. Juni 2026 — vorgestellt von Sam McVeety & Amir Hormati
