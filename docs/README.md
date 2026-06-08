# docs/ — Projektdokumentation

## Hauptdokumente

| Datei | Beschreibung |
|-------|-------------|
| `Expose.pdf` | Projekthexposé (Aufgabenstellung, Ziele) |
| `Expose.tex` | LaTeX-Quellcode des Exposés |
| `Projektarbeit_RL_Dokumentation.md` | Vollständige Projektdokumentation (Markdown) |
| `report.html` | HTML-Report mit Experimentergebnissen |
| `references.bib` | BibTeX-Literaturreferenzen |

## Forschungsliteratur  (`papers/`)

| Datei | Inhalt |
|-------|--------|
| `2312.09906v1.pdf` | RL / Navigation Paper |
| `2410.03618v3.pdf` | RL / Navigation Paper |
| `2604.10812v1.pdf` | RL / Navigation Paper |

## Experiment-Changelog

Der laufende Experiment-Changelog liegt im Projektwurzel:

```
../CHANGELOG.md     ← Alle Experimente, Ergebnisse, Änderungen
```

## Projektdokumentation als PDF erstellen

```bash
# Markdown → PDF (benötigt pandoc + LaTeX)
pandoc docs/Projektarbeit_RL_Dokumentation.md -o docs/Projektdokumentation.pdf

# Alternativ mit wkhtmltopdf
wkhtmltopdf docs/report.html docs/report.pdf
```
