"""Stoneforge RL — docs/RESULTS.md neu generieren.

Liest alle gespeicherten Ergebnisse und schreibt eine saubere Übersichtstabelle:
  models/*/config.json        → Trainings-Konfigurationen
  models/*/results.json       → Trainings-Ergebnisse
  models/*/eval_history.json  → SR-Verlauf pro Modell
  logs/eval_results/*.json    → Ablation-Vergleiche (eval_comparison.py)

Verwendung:
    python scripts/generate_results_table.py

Wird automatisch aufgerufen am Ende von:
  - scripts/train.py
  - scripts/train_curriculum.py
"""
from __future__ import annotations

import os
import sys

_PYTHON_DIR = os.path.join(os.path.dirname(__file__), "..", "python")
if _PYTHON_DIR not in sys.path:
    sys.path.insert(0, _PYTHON_DIR)

from doc_logger import generate_results_md

if __name__ == "__main__":
    out = generate_results_md()
    print(f"\nRESULTS.md aktualisiert: {out}")
