# Stoneforge RL — Ergebnistabelle

*Automatisch generiert am 05.08.2026 — nicht manuell bearbeiten.*
*Quelle: `models/*/results.json` + `logs/eval_results/*.json`*

Regenerieren: `python scripts/generate_results_table.py`

---

## 1 — Trainings-Runs

> ⚠️ **Die SR-Spalten sind Maxima über den gesamten Eval-Verlauf, keine Endergebnisse.** Ein Lauf kann bei Step 25k einmalig 94 % erreichen und bei Step 1,5 Mio. auf 4 % stehen; beides steht in derselben Zeile. Diese Werte dienen ausschließlich der Verlaufsdiagnose. Die berichtsfähigen Endergebnisse (bestes Modell nach Validierungs-SR, ausgewertet auf Testset A / Holdout B) stehen in `docs/Projektdokumentation.tex`, Tabelle „Endergebnis (v12, n=7)".

| Modell                     | Algo | Timesteps | n_envs | Exit-Range        | Device | Max. Val-SR im Verlauf (A-Proto.) | Max. Val-SR im Verlauf (B-Proto.) | Training-Zeit | Datum      |
| -------------------------- | ---- | --------- | ------ | ----------------- | ------ | --------------------------------- | --------------------------------- | ------------- | ---------- |
| exp_E1_critic              | rppo | —         | 16     | 5-45 (curriculum) | —      | 56.0%                             | —                                 | 5h 58m 15s    | 08.07.2026 |
| exp_E2_curric              | rppo | —         | 16     | 5-45 (curriculum) | —      | 38.0%                             | —                                 | 6h 0m 13s     | 08.07.2026 |
| exp_E3_lstm512             | rppo | —         | 16     | 5-45 (curriculum) | —      | 92.0%                             | —                                 | 22h 54m 41s   | 08.07.2026 |
| ppo_lstm_curriculum        | rppo | 2,200,000 | 16     | 5-45 (curriculum) | mps    | 86.0%                             | —                                 | 3h 12m 44s    | 25.06.2026 |
| ppo_lstm_curriculum_v11    | rppo | —         | 16     | —                 | —      | —                                 | —                                 | —             | 07.07.2026 |
| ppo_lstm_curriculum_v12_s1 | rppo | —         | 16     | 5-45 (curriculum) | —      | 92.0%                             | —                                 | 7h 48m 20s    | 07.07.2026 |
| ppo_lstm_curriculum_v12_s2 | rppo | —         | 16     | 5-45 (curriculum) | —      | 92.0%                             | —                                 | 7h 39m 50s    | 07.07.2026 |
| ppo_lstm_curriculum_v12_s3 | rppo | —         | 16     | 5-45 (curriculum) | —      | 88.0%                             | —                                 | 8h 32m 57s    | 07.07.2026 |
| ppo_lstm_curriculum_v12_s4 | rppo | —         | 16     | 5-45 (curriculum) | —      | 94.0%                             | —                                 | 36h 11m 32s   | 15.07.2026 |
| ppo_lstm_curriculum_v12_s5 | rppo | —         | 16     | 5-45 (curriculum) | —      | 72.0%                             | —                                 | 36h 18m 16s   | 15.07.2026 |
| ppo_lstm_curriculum_v12_s6 | rppo | —         | 16     | 5-45 (curriculum) | —      | 88.0%                             | —                                 | 29h 31m 52s   | 15.07.2026 |
| ppo_lstm_curriculum_v12_s7 | rppo | —         | 16     | 5-45 (curriculum) | —      | 90.0%                             | —                                 | 36h 13m 2s    | 15.07.2026 |
| ppo_mlp_curriculum_v12_s1  | ppo  | —         | 16     | 5-45 (curriculum) | —      | 54.0%                             | —                                 | 0h 34m 10s    | 05.08.2026 |
| ppo_mlp_curriculum_v12_s2  | ppo  | —         | 16     | 5-45 (curriculum) | —      | 86.0%                             | —                                 | 0h 28m 0s     | 05.08.2026 |
| ppo_mlp_curriculum_v12_s3  | ppo  | —         | 16     | 5-45 (curriculum) | —      | 58.0%                             | —                                 | 0h 45m 22s    | 05.08.2026 |
| ppo_mlp_curriculum_v12_s4  | ppo  | —         | 16     | 5-45 (curriculum) | —      | 58.0%                             | —                                 | 0h 51m 10s    | 05.08.2026 |
| ppo_mlp_curriculum_v12_s5  | ppo  | —         | 16     | 5-45 (curriculum) | —      | 100.0%                            | —                                 | 0h 17m 39s    | 05.08.2026 |
| ppo_mlp_curriculum_v12_s6  | ppo  | —         | 16     | 5-45 (curriculum) | —      | 82.0%                             | —                                 | 0h 50m 43s    | 05.08.2026 |
| ppo_mlp_curriculum_v12_s7  | ppo  | —         | 16     | 5-45 (curriculum) | —      | 88.0%                             | —                                 | 0h 26m 4s     | 05.08.2026 |


---

## 2 — Ablation-Vergleiche

### baselines

- **Seeds:** —
- **Exit-Range:** —
- **Modus:** Deterministisch
- **Datum:** —


### baselines_and_models

- **Seeds:** —
- **Exit-Range:** —
- **Modus:** Deterministisch
- **Datum:** —



---

## 3 — Eval-Verläufe pro Modell

**exp_E1_critic** — 24 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 56.0% @ Step 624,800

*(Zeige letzte 10 von 24 Einträgen)*

| Step    | SR    | Erfolge | Label                                                | Zeitstempel      |
| ------- | ----- | ------- | ---------------------------------------------------- | ---------------- |
| 899,712 | 20.0% | 10/50   | Phase 3 (exit=25–45, eval 35–45) [det=20%|stoch=42%] | 2026-07-08 10:36 |
| 924,704 | 28.0% | 14/50   | Phase 3 (exit=25–45, eval 35–45) [det=28%|stoch=72%] | 2026-07-08 10:46 |
| 649,792 | 44.0% | 22/50   | Phase 4 (Greedy Fine-Tune) [det=44%|stoch=68%]       | 2026-07-08 13:18 |
| 674,784 | 30.0% | 15/50   | Phase 4 (Greedy Fine-Tune) [det=30%|stoch=68%]       | 2026-07-08 13:33 |
| 699,776 | 20.0% | 10/50   | Phase 4 (Greedy Fine-Tune) [det=20%|stoch=32%]       | 2026-07-08 13:40 |
| 724,768 | 22.0% | 11/50   | Phase 4 (Greedy Fine-Tune) [det=22%|stoch=36%]       | 2026-07-08 13:47 |
| 749,760 | 6.0%  | 3/50    | Phase 4 (Greedy Fine-Tune) [det=6%|stoch=14%]        | 2026-07-08 13:54 |
| 774,752 | 18.0% | 9/50    | Phase 4 (Greedy Fine-Tune) [det=18%|stoch=30%]       | 2026-07-08 14:01 |
| 799,744 | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=28%]       | 2026-07-08 14:08 |
| 824,736 | 16.0% | 8/50    | Phase 4 (Greedy Fine-Tune) [det=16%|stoch=40%]       | 2026-07-08 14:16 |


**exp_E2_curric** — 24 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 38.0% @ Step 624,800

*(Zeige letzte 10 von 24 Einträgen)*

| Step    | SR    | Erfolge | Label                                               | Zeitstempel      |
| ------- | ----- | ------- | --------------------------------------------------- | ---------------- |
| 899,712 | 6.0%  | 3/50    | Phase 3 (exit=25–45, eval 35–45) [det=6%|stoch=32%] | 2026-07-08 10:41 |
| 924,704 | 0.0%  | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=10%] | 2026-07-08 10:49 |
| 649,792 | 8.0%  | 4/50    | Phase 4 (Greedy Fine-Tune) [det=8%|stoch=14%]       | 2026-07-08 13:30 |
| 674,784 | 30.0% | 15/50   | Phase 4 (Greedy Fine-Tune) [det=30%|stoch=50%]      | 2026-07-08 13:36 |
| 699,776 | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=54%]      | 2026-07-08 13:44 |
| 724,768 | 8.0%  | 4/50    | Phase 4 (Greedy Fine-Tune) [det=8%|stoch=8%]        | 2026-07-08 13:51 |
| 749,760 | 28.0% | 14/50   | Phase 4 (Greedy Fine-Tune) [det=28%|stoch=58%]      | 2026-07-08 13:58 |
| 774,752 | 16.0% | 8/50    | Phase 4 (Greedy Fine-Tune) [det=16%|stoch=30%]      | 2026-07-08 14:05 |
| 799,744 | 24.0% | 12/50   | Phase 4 (Greedy Fine-Tune) [det=24%|stoch=72%]      | 2026-07-08 14:11 |
| 824,736 | 0.0%  | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=20%]       | 2026-07-08 14:18 |


**exp_E3_lstm512** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 92.0% @ Step 74,976

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR    | Erfolge | Label                                              | Zeitstempel      |
| --------- | ----- | ------- | -------------------------------------------------- | ---------------- |
| 1,149,632 | 2.0%  | 1/50    | Phase 3 (exit=25–45, eval 35–45) [det=2%|stoch=4%] | 2026-07-09 05:51 |
| 1,174,624 | 2.0%  | 1/50    | Phase 3 (exit=25–45, eval 35–45) [det=2%|stoch=4%] | 2026-07-09 06:01 |
| 224,928   | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=42%]     | 2026-07-09 06:11 |
| 249,920   | 4.0%  | 2/50    | Phase 4 (Greedy Fine-Tune) [det=4%|stoch=16%]      | 2026-07-09 06:20 |
| 274,912   | 8.0%  | 4/50    | Phase 4 (Greedy Fine-Tune) [det=8%|stoch=16%]      | 2026-07-09 06:28 |
| 299,904   | 12.0% | 6/50    | Phase 4 (Greedy Fine-Tune) [det=12%|stoch=18%]     | 2026-07-09 06:37 |
| 324,896   | 2.0%  | 1/50    | Phase 4 (Greedy Fine-Tune) [det=2%|stoch=70%]      | 2026-07-09 06:46 |
| 349,888   | 18.0% | 9/50    | Phase 4 (Greedy Fine-Tune) [det=18%|stoch=56%]     | 2026-07-09 06:54 |
| 374,880   | 20.0% | 10/50   | Phase 4 (Greedy Fine-Tune) [det=20%|stoch=44%]     | 2026-07-09 07:03 |
| 399,872   | 4.0%  | 2/50    | Phase 4 (Greedy Fine-Tune) [det=4%|stoch=12%]      | 2026-07-09 07:12 |


**ppo_lstm_curriculum** — 4 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 86.0% @ Step 2,000,000

| Step      | SR    | Erfolge | Label                | Zeitstempel      |
| --------- | ----- | ------- | -------------------- | ---------------- |
| 250,000   | 34.0% | 17/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |
| 500,000   | 58.0% | 29/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |
| 1,000,000 | 76.0% | 38/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |
| 2,000,000 | 86.0% | 43/50   | Phase 3 (exit=25-45) | 2026-06-25 13:26 |


**ppo_lstm_curriculum_v11** — 59 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 48.0% @ Step 524,832

*(Zeige letzte 10 von 59 Einträgen)*

| Step    | SR   | Erfolge | Label                                               | Zeitstempel      |
| ------- | ---- | ------- | --------------------------------------------------- | ---------------- |
| 774,752 | 2.0% | 1/50    | Phase 3 (exit=25–45, eval 35–45) [det=2%|stoch=10%] | 2026-07-07 10:40 |
| 799,744 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=4%]  | 2026-07-07 10:42 |
| 824,736 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=16%] | 2026-07-07 10:45 |
| 849,728 | 2.0% | 1/50    | Phase 3 (exit=25–45, eval 35–45) [det=2%|stoch=8%]  | 2026-07-07 10:47 |
| 874,720 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=8%]  | 2026-07-07 10:49 |
| 899,712 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=4%]  | 2026-07-07 10:52 |
| 924,704 | 2.0% | 1/50    | Phase 3 (exit=25–45, eval 35–45) [det=2%|stoch=0%]  | 2026-07-07 10:54 |
| 949,696 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=6%]  | 2026-07-07 10:56 |
| 974,688 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=6%]  | 2026-07-07 10:59 |
| 999,680 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=0%]  | 2026-07-07 11:01 |


**ppo_lstm_curriculum_v11_s1** — 19 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 38.0% @ Step 449,856

*(Zeige letzte 10 von 19 Einträgen)*

| Step    | SR    | Erfolge | Label                                   | Zeitstempel      |
| ------- | ----- | ------- | --------------------------------------- | ---------------- |
| 249,920 | 34.0% | 17/50   | Phase 1 (exit=5–12) [det=2%|stoch=34%]  | 2026-07-07 16:58 |
| 274,912 | 14.0% | 7/50    | Phase 1 (exit=5–12) [det=2%|stoch=14%]  | 2026-07-07 17:00 |
| 299,904 | 22.0% | 11/50   | Phase 1 (exit=5–12) [det=2%|stoch=22%]  | 2026-07-07 17:03 |
| 324,896 | 8.0%  | 4/50    | Phase 1 (exit=5–12) [det=0%|stoch=8%]   | 2026-07-07 17:05 |
| 349,888 | 18.0% | 9/50    | Phase 1 (exit=5–12) [det=2%|stoch=18%]  | 2026-07-07 17:08 |
| 374,880 | 24.0% | 12/50   | Phase 1 (exit=5–12) [det=2%|stoch=24%]  | 2026-07-07 17:11 |
| 399,872 | 20.0% | 10/50   | Phase 1 (exit=5–12) [det=2%|stoch=20%]  | 2026-07-07 17:14 |
| 424,864 | 24.0% | 12/50   | Phase 1 (exit=5–12) [det=4%|stoch=24%]  | 2026-07-07 17:17 |
| 449,856 | 38.0% | 19/50   | Phase 1 (exit=5–12) [det=12%|stoch=38%] | 2026-07-07 17:20 |
| 474,848 | 34.0% | 17/50   | Phase 1 (exit=5–12) [det=10%|stoch=34%] | 2026-07-07 17:24 |


**ppo_lstm_curriculum_v11_s1_cap600** — 11 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 16.0% @ Step 49,984

*(Zeige letzte 10 von 11 Einträgen)*

| Step    | SR    | Erfolge | Label                                  | Zeitstempel      |
| ------- | ----- | ------- | -------------------------------------- | ---------------- |
| 49,984  | 16.0% | 8/50    | Phase 1 (exit=5–12) [det=0%|stoch=16%] | 2026-07-07 11:07 |
| 74,976  | 10.0% | 5/50    | Phase 1 (exit=5–12) [det=0%|stoch=10%] | 2026-07-07 11:09 |
| 99,968  | 12.0% | 6/50    | Phase 1 (exit=5–12) [det=2%|stoch=12%] | 2026-07-07 11:11 |
| 124,960 | 14.0% | 7/50    | Phase 1 (exit=5–12) [det=0%|stoch=14%] | 2026-07-07 11:14 |
| 149,952 | 12.0% | 6/50    | Phase 1 (exit=5–12) [det=4%|stoch=12%] | 2026-07-07 11:16 |
| 174,944 | 12.0% | 6/50    | Phase 1 (exit=5–12) [det=0%|stoch=12%] | 2026-07-07 11:18 |
| 199,936 | 12.0% | 6/50    | Phase 1 (exit=5–12) [det=0%|stoch=12%] | 2026-07-07 11:21 |
| 224,928 | 8.0%  | 4/50    | Phase 1 (exit=5–12) [det=2%|stoch=8%]  | 2026-07-07 11:23 |
| 249,920 | 10.0% | 5/50    | Phase 1 (exit=5–12) [det=0%|stoch=10%] | 2026-07-07 11:26 |
| 274,912 | 14.0% | 7/50    | Phase 1 (exit=5–12) [det=6%|stoch=14%] | 2026-07-07 11:28 |


**ppo_lstm_curriculum_v11_s1_nostream** — 3 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 18.0% @ Step 24,992

| Step   | SR    | Erfolge | Label                                  | Zeitstempel      |
| ------ | ----- | ------- | -------------------------------------- | ---------------- |
| 24,992 | 18.0% | 9/50    | Phase 1 (exit=5–12) [det=4%|stoch=18%] | 2026-07-07 17:18 |
| 49,984 | 14.0% | 7/50    | Phase 1 (exit=5–12) [det=2%|stoch=14%] | 2026-07-07 17:21 |
| 74,976 | 12.0% | 6/50    | Phase 1 (exit=5–12) [det=2%|stoch=12%] | 2026-07-07 17:25 |


**ppo_lstm_curriculum_v11_s1_noswarm** — 12 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 40.0% @ Step 274,912

*(Zeige letzte 10 von 12 Einträgen)*

| Step    | SR    | Erfolge | Label                                  | Zeitstempel      |
| ------- | ----- | ------- | -------------------------------------- | ---------------- |
| 74,976  | 12.0% | 6/50    | Phase 1 (exit=5–12) [det=2%|stoch=12%] | 2026-07-07 16:58 |
| 99,968  | 20.0% | 10/50   | Phase 1 (exit=5–12) [det=0%|stoch=20%] | 2026-07-07 17:00 |
| 124,960 | 22.0% | 11/50   | Phase 1 (exit=5–12) [det=0%|stoch=22%] | 2026-07-07 17:02 |
| 149,952 | 26.0% | 13/50   | Phase 1 (exit=5–12) [det=0%|stoch=26%] | 2026-07-07 17:05 |
| 174,944 | 16.0% | 8/50    | Phase 1 (exit=5–12) [det=0%|stoch=16%] | 2026-07-07 17:07 |
| 199,936 | 20.0% | 10/50   | Phase 1 (exit=5–12) [det=0%|stoch=20%] | 2026-07-07 17:10 |
| 224,928 | 28.0% | 14/50   | Phase 1 (exit=5–12) [det=4%|stoch=28%] | 2026-07-07 17:13 |
| 249,920 | 26.0% | 13/50   | Phase 1 (exit=5–12) [det=2%|stoch=26%] | 2026-07-07 17:17 |
| 274,912 | 40.0% | 20/50   | Phase 1 (exit=5–12) [det=2%|stoch=40%] | 2026-07-07 17:20 |
| 299,904 | 26.0% | 13/50   | Phase 1 (exit=5–12) [det=4%|stoch=26%] | 2026-07-07 17:24 |


**ppo_lstm_curriculum_v12_s1** — 71 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 94.0% @ Step 24,992

*(Zeige letzte 10 von 71 Einträgen)*

| Step      | SR    | Erfolge | Label                                                | Zeitstempel      |
| --------- | ----- | ------- | ---------------------------------------------------- | ---------------- |
| 1,499,520 | 10.0% | 5/50    | Phase 3 (exit=25–45, eval 35–45) [det=10%|stoch=22%] | 2026-07-08 02:32 |
| 1,524,512 | 2.0%  | 1/50    | Phase 3 (exit=25–45, eval 35–45) [det=2%|stoch=4%]   | 2026-07-08 02:40 |
| 774,752   | 30.0% | 15/50   | Phase 4 (Greedy Fine-Tune) [det=30%|stoch=40%]       | 2026-07-08 02:47 |
| 799,744   | 36.0% | 18/50   | Phase 4 (Greedy Fine-Tune) [det=36%|stoch=40%]       | 2026-07-08 02:54 |
| 824,736   | 36.0% | 18/50   | Phase 4 (Greedy Fine-Tune) [det=36%|stoch=64%]       | 2026-07-08 03:01 |
| 849,728   | 42.0% | 21/50   | Phase 4 (Greedy Fine-Tune) [det=42%|stoch=60%]       | 2026-07-08 03:07 |
| 874,720   | 36.0% | 18/50   | Phase 4 (Greedy Fine-Tune) [det=36%|stoch=52%]       | 2026-07-08 03:13 |
| 899,712   | 40.0% | 20/50   | Phase 4 (Greedy Fine-Tune) [det=40%|stoch=62%]       | 2026-07-08 03:20 |
| 924,704   | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=28%]       | 2026-07-08 03:26 |
| 949,696   | 20.0% | 10/50   | Phase 4 (Greedy Fine-Tune) [det=20%|stoch=52%]       | 2026-07-08 03:31 |


**ppo_lstm_curriculum_v12_s2** — 71 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 92.0% @ Step 399,872

*(Zeige letzte 10 von 71 Einträgen)*

| Step      | SR    | Erfolge | Label                                                | Zeitstempel      |
| --------- | ----- | ------- | ---------------------------------------------------- | ---------------- |
| 1,399,552 | 22.0% | 11/50   | Phase 3 (exit=25–45, eval 35–45) [det=22%|stoch=56%] | 2026-07-08 02:23 |
| 1,424,544 | 4.0%  | 2/50    | Phase 3 (exit=25–45, eval 35–45) [det=4%|stoch=12%]  | 2026-07-08 02:30 |
| 674,784   | 30.0% | 15/50   | Phase 4 (Greedy Fine-Tune) [det=30%|stoch=76%]       | 2026-07-08 02:37 |
| 699,776   | 4.0%  | 2/50    | Phase 4 (Greedy Fine-Tune) [det=4%|stoch=24%]        | 2026-07-08 02:44 |
| 724,768   | 30.0% | 15/50   | Phase 4 (Greedy Fine-Tune) [det=30%|stoch=60%]       | 2026-07-08 02:50 |
| 749,760   | 6.0%  | 3/50    | Phase 4 (Greedy Fine-Tune) [det=6%|stoch=34%]        | 2026-07-08 02:57 |
| 774,752   | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=32%]       | 2026-07-08 03:03 |
| 799,744   | 20.0% | 10/50   | Phase 4 (Greedy Fine-Tune) [det=20%|stoch=44%]       | 2026-07-08 03:10 |
| 824,736   | 8.0%  | 4/50    | Phase 4 (Greedy Fine-Tune) [det=8%|stoch=18%]        | 2026-07-08 03:16 |
| 849,728   | 18.0% | 9/50    | Phase 4 (Greedy Fine-Tune) [det=18%|stoch=36%]       | 2026-07-08 03:23 |


**ppo_lstm_curriculum_v12_s3** — 82 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 88.0% @ Step 499,840

*(Zeige letzte 10 von 82 Einträgen)*

| Step      | SR    | Erfolge | Label                                               | Zeitstempel      |
| --------- | ----- | ------- | --------------------------------------------------- | ---------------- |
| 1,799,424 | 8.0%  | 4/50    | Phase 3 (exit=25–45, eval 35–45) [det=8%|stoch=44%] | 2026-07-08 03:31 |
| 1,824,416 | 4.0%  | 2/50    | Phase 3 (exit=25–45, eval 35–45) [det=4%|stoch=18%] | 2026-07-08 03:37 |
| 1,349,568 | 26.0% | 13/50   | Phase 4 (Greedy Fine-Tune) [det=26%|stoch=76%]      | 2026-07-08 03:42 |
| 1,374,560 | 10.0% | 5/50    | Phase 4 (Greedy Fine-Tune) [det=10%|stoch=46%]      | 2026-07-08 03:47 |
| 1,399,552 | 18.0% | 9/50    | Phase 4 (Greedy Fine-Tune) [det=18%|stoch=54%]      | 2026-07-08 03:52 |
| 1,424,544 | 16.0% | 8/50    | Phase 4 (Greedy Fine-Tune) [det=16%|stoch=42%]      | 2026-07-08 03:57 |
| 1,449,536 | 2.0%  | 1/50    | Phase 4 (Greedy Fine-Tune) [det=2%|stoch=4%]        | 2026-07-08 04:02 |
| 1,474,528 | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=52%]      | 2026-07-08 04:07 |
| 1,499,520 | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=54%]      | 2026-07-08 04:11 |
| 1,524,512 | 6.0%  | 3/50    | Phase 4 (Greedy Fine-Tune) [det=6%|stoch=8%]        | 2026-07-08 04:16 |


**ppo_lstm_curriculum_v12_s4** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 94.0% @ Step 224,928

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR    | Erfolge | Label                                                | Zeitstempel      |
| --------- | ----- | ------- | ---------------------------------------------------- | ---------------- |
| 1,699,456 | 22.0% | 11/50   | Phase 3 (exit=25–45, eval 35–45) [det=22%|stoch=60%] | 2026-07-17 04:23 |
| 1,724,448 | 16.0% | 8/50    | Phase 3 (exit=25–45, eval 35–45) [det=16%|stoch=64%] | 2026-07-17 06:14 |
| 1,599,488 | 2.0%  | 1/50    | Phase 4 (Greedy Fine-Tune) [det=2%|stoch=12%]        | 2026-07-17 07:02 |
| 1,624,480 | 30.0% | 15/50   | Phase 4 (Greedy Fine-Tune) [det=30%|stoch=48%]       | 2026-07-17 07:58 |
| 1,649,472 | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=48%]       | 2026-07-17 08:58 |
| 1,674,464 | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=38%]       | 2026-07-17 09:03 |
| 1,699,456 | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=46%]       | 2026-07-17 09:09 |
| 1,724,448 | 32.0% | 16/50   | Phase 4 (Greedy Fine-Tune) [det=32%|stoch=58%]       | 2026-07-17 09:14 |
| 1,749,440 | 42.0% | 21/50   | Phase 4 (Greedy Fine-Tune) [det=42%|stoch=76%]       | 2026-07-17 09:19 |
| 1,774,432 | 10.0% | 5/50    | Phase 4 (Greedy Fine-Tune) [det=10%|stoch=40%]       | 2026-07-17 09:25 |


**ppo_lstm_curriculum_v12_s5** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 72.0% @ Step 374,880

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR    | Erfolge | Label                                                | Zeitstempel      |
| --------- | ----- | ------- | ---------------------------------------------------- | ---------------- |
| 1,449,536 | 28.0% | 14/50   | Phase 3 (exit=25–45, eval 35–45) [det=28%|stoch=74%] | 2026-07-17 06:42 |
| 1,474,528 | 26.0% | 13/50   | Phase 3 (exit=25–45, eval 35–45) [det=26%|stoch=52%] | 2026-07-17 07:20 |
| 849,728   | 18.0% | 9/50    | Phase 4 (Greedy Fine-Tune) [det=18%|stoch=54%]       | 2026-07-17 08:56 |
| 874,720   | 22.0% | 11/50   | Phase 4 (Greedy Fine-Tune) [det=22%|stoch=56%]       | 2026-07-17 09:01 |
| 899,712   | 14.0% | 7/50    | Phase 4 (Greedy Fine-Tune) [det=14%|stoch=34%]       | 2026-07-17 09:07 |
| 924,704   | 28.0% | 14/50   | Phase 4 (Greedy Fine-Tune) [det=28%|stoch=62%]       | 2026-07-17 09:12 |
| 949,696   | 26.0% | 13/50   | Phase 4 (Greedy Fine-Tune) [det=26%|stoch=46%]       | 2026-07-17 09:17 |
| 974,688   | 12.0% | 6/50    | Phase 4 (Greedy Fine-Tune) [det=12%|stoch=60%]       | 2026-07-17 09:23 |
| 999,680   | 26.0% | 13/50   | Phase 4 (Greedy Fine-Tune) [det=26%|stoch=64%]       | 2026-07-17 09:27 |
| 1,024,672 | 10.0% | 5/50    | Phase 4 (Greedy Fine-Tune) [det=10%|stoch=54%]       | 2026-07-17 09:32 |


**ppo_lstm_curriculum_v12_s6** — 78 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 88.0% @ Step 374,880

*(Zeige letzte 10 von 78 Einträgen)*

| Step      | SR    | Erfolge | Label                                                | Zeitstempel      |
| --------- | ----- | ------- | ---------------------------------------------------- | ---------------- |
| 1,349,568 | 28.0% | 14/50   | Phase 3 (exit=25–45, eval 35–45) [det=28%|stoch=54%] | 2026-07-17 00:35 |
| 1,374,560 | 20.0% | 10/50   | Phase 3 (exit=25–45, eval 35–45) [det=20%|stoch=36%] | 2026-07-17 00:42 |
| 1,224,608 | 30.0% | 15/50   | Phase 4 (Greedy Fine-Tune) [det=30%|stoch=54%]       | 2026-07-17 00:50 |
| 1,249,600 | 24.0% | 12/50   | Phase 4 (Greedy Fine-Tune) [det=24%|stoch=56%]       | 2026-07-17 00:56 |
| 1,274,592 | 6.0%  | 3/50    | Phase 4 (Greedy Fine-Tune) [det=6%|stoch=22%]        | 2026-07-17 01:02 |
| 1,299,584 | 4.0%  | 2/50    | Phase 4 (Greedy Fine-Tune) [det=4%|stoch=6%]         | 2026-07-17 01:08 |
| 1,324,576 | 34.0% | 17/50   | Phase 4 (Greedy Fine-Tune) [det=34%|stoch=56%]       | 2026-07-17 01:14 |
| 1,349,568 | 20.0% | 10/50   | Phase 4 (Greedy Fine-Tune) [det=20%|stoch=44%]       | 2026-07-17 01:20 |
| 1,374,560 | 36.0% | 18/50   | Phase 4 (Greedy Fine-Tune) [det=36%|stoch=58%]       | 2026-07-17 01:26 |
| 1,399,552 | 24.0% | 12/50   | Phase 4 (Greedy Fine-Tune) [det=24%|stoch=62%]       | 2026-07-17 02:28 |


**ppo_lstm_curriculum_v12_s7** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 90.0% @ Step 299,904

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR    | Erfolge | Label                                                | Zeitstempel      |
| --------- | ----- | ------- | ---------------------------------------------------- | ---------------- |
| 1,674,464 | 18.0% | 9/50    | Phase 3 (exit=25–45, eval 35–45) [det=18%|stoch=44%] | 2026-07-17 04:58 |
| 1,699,456 | 2.0%  | 1/50    | Phase 3 (exit=25–45, eval 35–45) [det=2%|stoch=32%]  | 2026-07-17 06:42 |
| 1,274,592 | 26.0% | 13/50   | Phase 4 (Greedy Fine-Tune) [det=26%|stoch=62%]       | 2026-07-17 07:20 |
| 1,299,584 | 26.0% | 13/50   | Phase 4 (Greedy Fine-Tune) [det=26%|stoch=54%]       | 2026-07-17 08:28 |
| 1,324,576 | 12.0% | 6/50    | Phase 4 (Greedy Fine-Tune) [det=12%|stoch=38%]       | 2026-07-17 09:00 |
| 1,349,568 | 10.0% | 5/50    | Phase 4 (Greedy Fine-Tune) [det=10%|stoch=56%]       | 2026-07-17 09:06 |
| 1,374,560 | 8.0%  | 4/50    | Phase 4 (Greedy Fine-Tune) [det=8%|stoch=30%]        | 2026-07-17 09:11 |
| 1,399,552 | 6.0%  | 3/50    | Phase 4 (Greedy Fine-Tune) [det=6%|stoch=54%]        | 2026-07-17 09:16 |
| 1,424,544 | 12.0% | 6/50    | Phase 4 (Greedy Fine-Tune) [det=12%|stoch=64%]       | 2026-07-17 09:21 |
| 1,449,536 | 4.0%  | 2/50    | Phase 4 (Greedy Fine-Tune) [det=4%|stoch=14%]        | 2026-07-17 09:26 |


**ppo_mlp_curriculum_v12_s1** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 54.0% @ Step 49,984

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR   | Erfolge | Label                                               | Zeitstempel      |
| --------- | ---- | ------- | --------------------------------------------------- | ---------------- |
| 1,499,520 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=22%] | 2026-08-05 13:40 |
| 1,524,512 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=8%]  | 2026-08-05 13:41 |
| 574,816   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=18%]       | 2026-08-05 13:41 |
| 599,808   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=8%]        | 2026-08-05 13:42 |
| 624,800   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]        | 2026-08-05 13:42 |
| 649,792   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=0%]        | 2026-08-05 13:43 |
| 674,784   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=4%]        | 2026-08-05 13:43 |
| 699,776   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=4%]        | 2026-08-05 13:44 |
| 724,768   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]        | 2026-08-05 13:44 |
| 749,760   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=4%]        | 2026-08-05 13:45 |


**ppo_mlp_curriculum_v12_s2** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 86.0% @ Step 799,744

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR   | Erfolge | Label                                               | Zeitstempel      |
| --------- | ---- | ------- | --------------------------------------------------- | ---------------- |
| 1,774,432 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=24%] | 2026-08-05 14:00 |
| 1,799,424 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=12%] | 2026-08-05 14:00 |
| 849,728   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=56%]       | 2026-08-05 14:01 |
| 874,720   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=52%]       | 2026-08-05 14:01 |
| 899,712   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=66%]       | 2026-08-05 14:01 |
| 924,704   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=70%]       | 2026-08-05 14:01 |
| 949,696   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=52%]       | 2026-08-05 14:02 |
| 974,688   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=48%]       | 2026-08-05 14:02 |
| 999,680   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=60%]       | 2026-08-05 14:02 |
| 1,024,672 | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=60%]       | 2026-08-05 14:02 |


**ppo_mlp_curriculum_v12_s3** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 58.0% @ Step 24,992

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR   | Erfolge | Label                                               | Zeitstempel      |
| --------- | ---- | ------- | --------------------------------------------------- | ---------------- |
| 1,474,528 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=14%] | 2026-08-05 14:03 |
| 1,499,520 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=20%] | 2026-08-05 14:03 |
| 549,824   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=30%]       | 2026-08-05 14:04 |
| 574,816   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=16%]       | 2026-08-05 14:11 |
| 599,808   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=24%]       | 2026-08-05 14:14 |
| 624,800   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=22%]       | 2026-08-05 14:18 |
| 649,792   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=30%]       | 2026-08-05 14:19 |
| 674,784   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=18%]       | 2026-08-05 14:19 |
| 699,776   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=54%]       | 2026-08-05 14:19 |
| 724,768   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=46%]       | 2026-08-05 14:20 |


**ppo_mlp_curriculum_v12_s4** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 58.0% @ Step 24,992

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR   | Erfolge | Label                                              | Zeitstempel      |
| --------- | ---- | ------- | -------------------------------------------------- | ---------------- |
| 1,024,672 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=0%] | 2026-08-05 14:22 |
| 1,049,664 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=0%] | 2026-08-05 14:22 |
| 99,968    | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]       | 2026-08-05 14:22 |
| 124,960   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]       | 2026-08-05 14:23 |
| 149,952   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=0%]       | 2026-08-05 14:23 |
| 174,944   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=4%]       | 2026-08-05 14:24 |
| 199,936   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]       | 2026-08-05 14:24 |
| 224,928   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]       | 2026-08-05 14:25 |
| 249,920   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]       | 2026-08-05 14:25 |
| 274,912   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=12%]      | 2026-08-05 14:26 |


**ppo_mlp_curriculum_v12_s5** — 66 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 100.0% @ Step 399,872

*(Zeige letzte 10 von 66 Einträgen)*

| Step      | SR   | Erfolge | Label                                               | Zeitstempel      |
| --------- | ---- | ------- | --------------------------------------------------- | ---------------- |
| 1,399,552 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=18%] | 2026-08-05 13:50 |
| 1,424,544 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=18%] | 2026-08-05 13:50 |
| 474,848   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=76%]       | 2026-08-05 13:50 |
| 499,840   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=68%]       | 2026-08-05 13:50 |
| 524,832   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=66%]       | 2026-08-05 13:51 |
| 549,824   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=62%]       | 2026-08-05 13:51 |
| 574,816   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=54%]       | 2026-08-05 13:51 |
| 599,808   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=52%]       | 2026-08-05 13:51 |
| 624,800   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=58%]       | 2026-08-05 13:52 |
| 649,792   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=70%]       | 2026-08-05 13:52 |


**ppo_mlp_curriculum_v12_s6** — 88 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 82.0% @ Step 49,984

*(Zeige letzte 10 von 88 Einträgen)*

| Step      | SR   | Erfolge | Label                                              | Zeitstempel      |
| --------- | ---- | ------- | -------------------------------------------------- | ---------------- |
| 1,399,552 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=0%] | 2026-08-05 14:21 |
| 1,424,544 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=2%] | 2026-08-05 14:21 |
| 474,848   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=6%]       | 2026-08-05 14:22 |
| 499,840   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]       | 2026-08-05 14:22 |
| 524,832   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=6%]       | 2026-08-05 14:23 |
| 549,824   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=8%]       | 2026-08-05 14:23 |
| 574,816   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=12%]      | 2026-08-05 14:24 |
| 599,808   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=2%]       | 2026-08-05 14:24 |
| 624,800   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=8%]       | 2026-08-05 14:25 |
| 649,792   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=4%]       | 2026-08-05 14:25 |


**ppo_mlp_curriculum_v12_s7** — 82 Eval-Punkte,
Maximum im Verlauf (kein Endergebnis): 88.0% @ Step 599,808

*(Zeige letzte 10 von 82 Einträgen)*

| Step      | SR   | Erfolge | Label                                               | Zeitstempel      |
| --------- | ---- | ------- | --------------------------------------------------- | ---------------- |
| 1,574,496 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=24%] | 2026-08-05 13:58 |
| 1,599,488 | 0.0% | 0/50    | Phase 3 (exit=25–45, eval 35–45) [det=0%|stoch=14%] | 2026-08-05 13:59 |
| 649,792   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=70%]       | 2026-08-05 13:59 |
| 674,784   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=66%]       | 2026-08-05 13:59 |
| 699,776   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=70%]       | 2026-08-05 13:59 |
| 724,768   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=64%]       | 2026-08-05 14:00 |
| 749,760   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=60%]       | 2026-08-05 14:00 |
| 774,752   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=60%]       | 2026-08-05 14:00 |
| 799,744   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=62%]       | 2026-08-05 14:00 |
| 824,736   | 0.0% | 0/50    | Phase 4 (Greedy Fine-Tune) [det=0%|stoch=72%]       | 2026-08-05 14:01 |


