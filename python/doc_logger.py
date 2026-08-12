"""Stoneforge — Dokumentations-Logging-Utilities.

Zentrales Modul für automatisches Speichern aller Trainings- und Eval-Daten.
Wird von train.py, train_curriculum.py und eval_comparison.py importiert.

Speicherstruktur:
  models/{name}/config.json        — Hyperparameter + Trainings-Setup
  models/{name}/results.json       — Ergebnisse (SR, Zeiten, Phasen)
  models/{name}/eval_history.json  — SR-Verlauf über alle Eval-Checkpoints
  logs/eval_results/{name}.json    — Ablation / Vergleichs-Evaluationen
  logs/eval_results/{name}.csv     — Gleiche Daten als CSV (für Excel / Thesis)
  docs/RESULTS.md                  — Auto-generierte Ergebnistabelle (alle Runs)
"""
from __future__ import annotations

import csv
import json
import os
import subprocess
from datetime import datetime
from typing import Any

_ROOT       = os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))
RESULTS_DIR = os.path.join(_ROOT, "logs", "eval_results")
MODELS_DIR  = os.path.join(_ROOT, "models")
DOCS_DIR    = os.path.join(_ROOT, "docs")


def _now() -> str:
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def _today() -> str:
    return datetime.now().strftime("%d.%m.%Y")


def _git_commit() -> str:
    """Kurzer Commit-Hash des Repos (+ '-dirty' bei uncommitteten Änderungen).

    Für die Reproduzierbarkeit: jede config.json wird an den Code-Stand gebunden,
    der sie erzeugt hat. Gibt 'unknown' zurück, falls kein git verfügbar ist.
    """
    try:
        h = subprocess.check_output(
            ["git", "-C", _ROOT, "rev-parse", "--short", "HEAD"],
            stderr=subprocess.DEVNULL,
        ).decode().strip()
        dirty = subprocess.call(
            ["git", "-C", _ROOT, "diff", "--quiet", "--ignore-submodules"],
            stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
        )
        return f"{h}-dirty" if dirty else h
    except Exception:
        return "unknown"


# ── Trainings-Konfiguration ───────────────────────────────────────────────────

def save_run_config(save_dir: str, config: dict[str, Any]) -> str:
    """Speichert Hyperparameter + Setup als config.json neben das Modell."""
    os.makedirs(save_dir, exist_ok=True)
    config["_saved_at"] = _now()
    config["_git_commit"] = _git_commit()
    path = os.path.join(save_dir, "config.json")
    with open(path, "w", encoding="utf-8") as f:
        json.dump(config, f, indent=2, ensure_ascii=False)
    print(f"  [doc] config.json → {path}")
    return path


# ── Trainings-Ergebnisse ──────────────────────────────────────────────────────

def save_run_results(save_dir: str, results: dict[str, Any]) -> str:
    """Speichert / aktualisiert results.json neben das Modell.

    Mehrfache Aufrufe mergen in dieselbe Datei (z.B. Phase 1 + Phase 2).
    """
    os.makedirs(save_dir, exist_ok=True)
    path = os.path.join(save_dir, "results.json")
    existing: dict = {}
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            existing = json.load(f)
    results["_saved_at"] = _now()
    existing.update(results)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(existing, f, indent=2, ensure_ascii=False)
    print(f"  [doc] results.json → {path}")
    return path


# ── Eval-History (wird bei jedem Callback-Eval geschrieben) ──────────────────

def append_eval_history(
    save_dir: str,
    step: int,
    sr: float,
    successes: int,
    n: int,
    label: str = "",
) -> None:
    """Hängt einen Eval-Eintrag an eval_history.json an.

    Jeder Callback-Eval-Schritt wird so dauerhaft festgehalten — kein Datenverlust
    mehr wenn das Training abbricht oder das Terminal geschlossen wird.
    """
    os.makedirs(save_dir, exist_ok=True)
    path = os.path.join(save_dir, "eval_history.json")
    history: list = []
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            history = json.load(f)
    history.append({
        "step":      step,
        "sr":        round(sr, 4),
        "successes": successes,
        "n":         n,
        "label":     label,
        "timestamp": _now(),
    })
    with open(path, "w", encoding="utf-8") as f:
        json.dump(history, f, indent=2, ensure_ascii=False)


# ── Ablation / Vergleichs-Evaluation ─────────────────────────────────────────

def save_eval_results(
    name: str,
    metadata: dict[str, Any],
    conditions: list[dict[str, Any]],
    out_dir: str | None = None,
) -> tuple[str, str]:
    """Speichert Eval-Vergleichstabelle als JSON + CSV.

    Args:
        name:       Dateiname (ohne Extension), z.B. "ablation_20260625_det"
        metadata:   Globale Infos: seeds, exit_range, deterministic, date
        conditions: Liste von Dicts, je eine Tabellenzeile:
                    {"label", "model_path", "sr", "successes", "n",
                     "mean_len", "mean_ret", "note"}
        out_dir:    Zielordner, default: logs/eval_results/

    Returns:
        (json_path, csv_path)
    """
    out_dir = out_dir or RESULTS_DIR
    os.makedirs(out_dir, exist_ok=True)

    data = {
        "name":       name,
        "metadata":   metadata,
        "conditions": conditions,
        "_saved_at":  _now(),
    }

    json_path = os.path.join(out_dir, f"{name}.json")
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    # CSV — eine Zeile pro Bedingung, Metadata-Felder vorne angehängt
    csv_path = os.path.join(out_dir, f"{name}.csv")
    if conditions:
        meta_prefix = {
            "eval_date":    metadata.get("date", _today()),
            "seeds":        metadata.get("seeds_label", ""),
            "exit_range":   metadata.get("exit_range", ""),
            "deterministic": metadata.get("deterministic", True),
        }
        rows = [{**meta_prefix, **c} for c in conditions]
        with open(csv_path, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            writer.writeheader()
            writer.writerows(rows)

    print(f"  [doc] eval_results → {json_path}")
    print(f"  [doc] eval_results → {csv_path}")
    return json_path, csv_path


# ── RESULTS.md auto-generieren ────────────────────────────────────────────────

def generate_results_md() -> str:
    """Liest alle results.json + eval_results/*.json und schreibt docs/RESULTS.md.

    Kann jederzeit aufgerufen werden: python -c "from doc_logger import generate_results_md; generate_results_md()"
    Oder direkt: python scripts/generate_results_table.py
    """
    lines: list[str] = []
    lines.append("# Stoneforge RL — Ergebnistabelle")
    lines.append(f"\n*Automatisch generiert am {_today()} — nicht manuell bearbeiten.*")
    lines.append("*Quelle: `models/*/results.json` + `logs/eval_results/*.json`*\n")
    lines.append("Regenerieren: `python scripts/generate_results_table.py`\n")

    # ── Abschnitt 1: Trainings-Runs ──────────────────────────────────────────
    lines.append("---\n")
    lines.append("## 1 — Trainings-Runs\n")
    lines.append(
        "> ⚠️ **Die SR-Spalten sind Maxima über den gesamten Eval-Verlauf, keine "
        "Endergebnisse.** Ein Lauf kann bei Step 25k einmalig 94 % erreichen und "
        "bei Step 1,5 Mio. auf 4 % stehen; beides steht in derselben Zeile. Diese "
        "Werte dienen ausschließlich der Verlaufsdiagnose. Die berichtsfähigen "
        "Endergebnisse (bestes Modell nach Validierungs-SR, ausgewertet auf "
        "Testset A / Holdout B) stehen in `docs/Projektdokumentation.tex`, "
        "Tabelle „Endergebnis (v12, n=7)\".\n"
    )

    run_rows: list[dict] = []
    if os.path.isdir(MODELS_DIR):
        for model_name in sorted(os.listdir(MODELS_DIR)):
            results_path = os.path.join(MODELS_DIR, model_name, "results.json")
            config_path  = os.path.join(MODELS_DIR, model_name, "config.json")
            if not os.path.exists(results_path):
                continue
            with open(results_path, encoding="utf-8") as f:
                r = json.load(f)
            c: dict = {}
            if os.path.exists(config_path):
                with open(config_path, encoding="utf-8") as f:
                    c = json.load(f)
            run_rows.append({
                "Modell":        model_name,
                "Algo":          c.get("algo", r.get("algo", "—")),
                "Timesteps":     f"{c.get('timesteps', r.get('timesteps', '—')):,}" if isinstance(c.get('timesteps', r.get('timesteps')), int) else c.get('timesteps', r.get('timesteps', '—')),
                "n_envs":        c.get("n_envs", r.get("n_envs", "—")),
                "Exit-Range":    c.get("exit_range", r.get("exit_range", "—")),
                "Device":        c.get("device", r.get("device", "—")),
                # ACHTUNG: Maximum über den gesamten Eval-Verlauf (Val-Seeds),
                # NICHT das Endergebnis des Laufs. Nur zur Verlaufsdiagnose
                # brauchbar; berichtsfähige Zahlen stehen in der Projektdoku.
                "Max. Val-SR im Verlauf (A-Proto.)": _fmt_sr(r.get("best_sr_testset_a")),
                "Max. Val-SR im Verlauf (B-Proto.)": _fmt_sr(r.get("best_sr_testset_b")),
                "Training-Zeit": r.get("training_duration", "—"),
                "Datum":         c.get("date", r.get("date", r.get("_saved_at", "—")))[:10],
            })

    if run_rows:
        lines.append(_md_table(run_rows))
    else:
        lines.append("*Noch keine results.json Einträge vorhanden.*\n")
        lines.append("*Nach dem nächsten Training werden hier automatisch Einträge erscheinen.*\n")

    # ── Abschnitt 2: Ablation-Vergleiche ─────────────────────────────────────
    lines.append("\n---\n")
    lines.append("## 2 — Ablation-Vergleiche\n")

    if os.path.isdir(RESULTS_DIR):
        eval_files = sorted(f for f in os.listdir(RESULTS_DIR) if f.endswith(".json"))
    else:
        eval_files = []

    if not eval_files:
        lines.append("*Noch keine Ablation-Ergebnisse vorhanden.*\n")
        lines.append("*Werden automatisch gespeichert wenn `eval_comparison.py` läuft.*\n")
    else:
        for fname in eval_files:
            with open(os.path.join(RESULTS_DIR, fname), encoding="utf-8") as f:
                data = json.load(f)
            # Skip files that are not dictionaries (e.g., world_geometry.json is a list)
            if not isinstance(data, dict):
                continue
            meta       = data.get("metadata", {})
            conditions = data.get("conditions", [])
            run_name   = data.get("name", fname[:-5])

            lines.append(f"### {run_name}\n")
            lines.append(f"- **Seeds:** {meta.get('seeds_label', '—')}")
            lines.append(f"- **Exit-Range:** {meta.get('exit_range', '—')}")
            det = meta.get("deterministic", True)
            lines.append(f"- **Modus:** {'Deterministisch' if det else 'Stochastisch'}")
            lines.append(f"- **Datum:** {meta.get('date', data.get('_saved_at', '—'))[:10]}\n")

            if conditions:
                rows = []
                for cond in conditions:
                    sr = cond.get("sr")
                    rows.append({
                        "Bedingung":  cond.get("label", "—"),
                        "SR":         f"{sr:.1%}" if isinstance(sr, float) else "—",
                        "Erfolge":    f"{cond.get('successes', '—')}/{cond.get('n', '—')}",
                        "Ø Schritte": f"{cond.get('mean_len', 0):.1f}" if cond.get('mean_len') else "—",
                        "Ø Return":   f"{cond.get('mean_ret', 0):+.2f}" if cond.get('mean_ret') is not None else "—",
                        "Notiz":      cond.get("note", ""),
                    })
                lines.append(_md_table(rows))
            lines.append("")

    # ── Abschnitt 3: Eval-Histories ───────────────────────────────────────────
    lines.append("\n---\n")
    lines.append("## 3 — Eval-Verläufe pro Modell\n")

    history_found = False
    if os.path.isdir(MODELS_DIR):
        for model_name in sorted(os.listdir(MODELS_DIR)):
            hist_path = os.path.join(MODELS_DIR, model_name, "eval_history.json")
            if not os.path.exists(hist_path):
                continue
            history_found = True
            with open(hist_path, encoding="utf-8") as f:
                history = json.load(f)
            if not history:
                continue
            best_entry = max(history, key=lambda e: e["sr"])
            lines.append(f"**{model_name}** — {len(history)} Eval-Punkte,")
            lines.append(
                f"Maximum im Verlauf (kein Endergebnis): {best_entry['sr']:.1%} "
                f"@ Step {best_entry['step']:,}\n"
            )

            rows = [
                {
                    "Step":       f"{e['step']:,}",
                    "SR":         f"{e['sr']:.1%}",
                    "Erfolge":    f"{e['successes']}/{e['n']}",
                    "Label":      e.get("label", ""),
                    "Zeitstempel":e.get("timestamp", "")[:16],
                }
                for e in history[-10:]  # letzte 10 Einträge
            ]
            if len(history) > 10:
                lines.append(f"*(Zeige letzte 10 von {len(history)} Einträgen)*\n")
            lines.append(_md_table(rows))
            lines.append("")

    if not history_found:
        lines.append("*Noch keine Eval-Histories vorhanden.*\n")

    # Schreiben
    os.makedirs(DOCS_DIR, exist_ok=True)
    out_path = os.path.join(DOCS_DIR, "RESULTS.md")
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f"  [doc] RESULTS.md → {out_path}")
    return out_path


# ── Hilfsfunktionen ───────────────────────────────────────────────────────────

def _fmt_sr(val: Any) -> str:
    if val is None:
        return "—"
    if isinstance(val, float):
        return f"{val:.1%}"
    return str(val)


def _md_table(rows: list[dict]) -> str:
    if not rows:
        return ""
    headers = list(rows[0].keys())
    widths  = [max(len(h), max(len(str(r.get(h, ""))) for r in rows)) for h in headers]

    def fmt_row(values: list[str]) -> str:
        return "| " + " | ".join(str(v).ljust(w) for v, w in zip(values, widths)) + " |"

    sep = "| " + " | ".join("-" * w for w in widths) + " |"
    result = [fmt_row(headers), sep]
    for row in rows:
        result.append(fmt_row([str(row.get(h, "")) for h in headers]))
    return "\n".join(result) + "\n"
