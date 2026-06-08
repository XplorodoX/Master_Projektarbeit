Launcher usage

Interaktives Terminal-Menü für Build, Training und Playback.

Quick start

```bash
# Im Projektverzeichnis (Master_Projektarbeit/):
python scripts/launcher.py
```

Kein Aktivieren des venv nötig — der Launcher erkennt .venv automatisch.

Menü-Optionen

1. DQN trainieren  (empfohlen)  — Timesteps + Curriculum Learning wählbar
2. PPO trainieren               — wie oben
3. Modell abspielen             — Modellpfad, Seed, Geschwindigkeit
4. Zwei Modelle vergleichen     — Dual-Modus (DQN vs. PPO nebeneinander)
5. 50-Seed Evaluation           — Standardtest Seeds 7000–7049
6. Python Bindings bauen        — CMake Release-Build erzwingen
7. Beenden

Hinweise

- PYTHONPATH wird automatisch gesetzt (build/ und python/).
- Build immer im Release-Modus für schnelle Simulation.
- Curriculum Learning: Exit startet bei 5–12 Tiles, steigt auf 35–45.
- Eval-Env beim Training bleibt immer auf 35–45 Tiles (misst echten Fortschritt).
