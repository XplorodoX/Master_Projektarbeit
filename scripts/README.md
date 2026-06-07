# scripts/ — Ausführbare Skripte

Alle direkt ausführbaren Programme des Projekts.  
Importierbare Bibliotheksdateien liegen in `../python/`.

---

## Übersicht

| Datei | Beschreibung | Aufruf |
|-------|-------------|--------|
| `launcher_gui.py` | **Haupt-Einstiegspunkt** — GUI für Training, Abspielen, Eval, Build | `python scripts/launcher_gui.py` |
| `train.py` | Training starten (PPO / DQN / A2C + Curriculum) | `python scripts/train.py --algo ppo --timesteps 1000000` |
| `watch_agent.py` | KI-Agent grafisch beobachten (auto-erkennt PPO/A2C/DQN) | `python scripts/watch_agent.py --model models/ppo_phase5/best_model.zip` |
| `analyze_agent.py` | Verhaltensanalyse: Trajektorien, Loops, BFS-Abstand | `python scripts/analyze_agent.py --model models/ppo_phase5/best_model.zip` |
| `eval_hard_world.py` | Eval auf schwierigen Welten (dichte Wände + Cellular Smoothing) | `python scripts/eval_hard_world.py --model models/ppo_phase5/best_model.zip` |
| `eval_temperature.py` | Temperatur-Sampling-Benchmark (deterministisch vs. stochastisch) | `python scripts/eval_temperature.py --model models/ppo_phase5/best_model.zip` |
| `setup_env.sh` | PYTHONPATH setzen + venv aktivieren | `source scripts/setup_env.sh` |

---

## Voraussetzungen

```bash
# 1. Bauen
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j

# 2. Umgebung aktivieren
source scripts/setup_env.sh
# oder manuell:
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
source .venv/bin/activate
pip install -r requirements.txt
```

---

## 50-Seed-Standard-Eval (Seeds 7000–7049)

```bash
python - <<'PY'
import numpy as np
from stable_baselines3 import PPO
from stoneforge_env import StoneforgeWorldEnv

seeds = list(range(7000, 7050))
env = StoneforgeWorldEnv()
model = PPO.load("models/ppo_phase5/best_model.zip")
succ, lens, rets = 0, [], []
for seed in seeds:
    obs, _ = env.reset(seed=seed)
    done, ep_ret, steps, reached = False, 0.0, 0, False
    while not done and steps < 4000:
        action, _ = model.predict(obs, deterministic=True)
        obs, r, term, trunc, info = env.step(int(action))
        ep_ret += float(r); steps += 1
        if info.get("reached_exit", False): reached = True
        done = term or trunc
    succ += int(reached); lens.append(steps); rets.append(ep_ret)
print(f"PPO: success={succ}/50 ({succ/50:.1%}), mean_len={np.mean(lens):.1f}, mean_return={np.mean(rets):.2f}")
PY
```
