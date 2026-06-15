import sys
import os
import numpy as np
from sb3_contrib import RecurrentPPO

_PYTHON_DIR  = os.path.join(os.path.dirname(__file__), "..", "python")
if _PYTHON_DIR not in sys.path:
    sys.path.insert(0, _PYTHON_DIR)

from stoneforge_env import StoneforgeWorldEnv

VAL_SEEDS = list(range(6000, 6050))
MAX_EVAL_STEPS = 4000

def eval_model(path, deterministic):
    model = RecurrentPPO.load(path)
    # Phase 2 is exit_min=12, exit_max=25
    env = StoneforgeWorldEnv(exit_min=12, exit_max=25, use_last_action_reward=True)
    
    successes = 0
    lengths = []
    
    for seed in VAL_SEEDS:
        obs, _ = env.reset(seed=seed)
        done, steps = False, 0
        lstm_states = None
        ep_start = np.ones((1,), dtype=bool)

        while not done and steps < MAX_EVAL_STEPS:
            action, lstm_states = model.predict(
                obs.reshape(1, -1), state=lstm_states,
                episode_start=ep_start, deterministic=deterministic,
            )
            obs, _, term, trunc, info = env.step(int(action[0]))
            ep_start = np.zeros((1,), dtype=bool)
            done = term or trunc; steps += 1
            if info.get("reached_exit"):
                successes += 1; break
        lengths.append(steps)
                
    sr = successes / len(VAL_SEEDS)
    mean_len = np.mean(lengths)
    return sr, mean_len

def main():
    model_path = "models/ppo_lstm_curriculum_v5_s0/phase2_best_model.zip"
    print(f"Loading {model_path}...")
    
    print("Evaluating deterministically...")
    det_sr, det_len = eval_model(model_path, deterministic=True)
    print(f"Deterministic: SR={det_sr:.1%} (Avg steps: {det_len:.1f})")
    
    print("Evaluating stochastically...")
    stoch_sr, stoch_len = eval_model(model_path, deterministic=False)
    print(f"Stochastic:    SR={stoch_sr:.1%} (Avg steps: {stoch_len:.1f})")

if __name__ == "__main__":
    main()
