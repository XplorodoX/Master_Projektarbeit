# Stoneforge RL — Ergebnistabelle

*Automatisch generiert am 25.06.2026 — nicht manuell bearbeiten.*
*Quelle: `models/*/results.json` + `logs/eval_results/*.json`*

Regenerieren: `python scripts/generate_results_table.py`

---

## 1 — Trainings-Runs

| Modell              | Algo | Timesteps | n_envs | Exit-Range        | Device | Beste SR (A) | SR Holdout (B) | Training-Zeit | Datum      |
| ------------------- | ---- | --------- | ------ | ----------------- | ------ | ------------ | -------------- | ------------- | ---------- |
| ppo_lstm_curriculum | rppo | 2,200,000 | 16     | 5-45 (curriculum) | mps    | 86.0%        | —              | 3h 12m 44s    | 25.06.2026 |


---

## 2 — Ablation-Vergleiche

*Noch keine Ablation-Ergebnisse vorhanden.*

*Werden automatisch gespeichert wenn `eval_comparison.py` läuft.*


---

## 3 — Eval-Verläufe pro Modell

**ppo_lstm_curriculum** — 4 Eval-Punkte,
Beste SR: 86.0% @ Step 2,000,000

| Step      | SR    | Erfolge | Label                | Zeitstempel      |
| --------- | ----- | ------- | -------------------- | ---------------- |
| 250,000   | 34.0% | 17/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |
| 500,000   | 58.0% | 29/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |
| 1,000,000 | 76.0% | 38/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |
| 2,000,000 | 86.0% | 43/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |


