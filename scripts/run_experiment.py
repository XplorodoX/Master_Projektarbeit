"""Experiment runner to automate 3-runs protocol and run evaluation."""
from __future__ import annotations

import argparse
import os
import sys
import subprocess
import numpy as np
import torch
from stable_baselines3 import PPO
from sb3_contrib import RecurrentPPO

# Project paths setup
_PYTHON_DIR  = os.path.join(os.path.dirname(__file__), "..", "python")
_BUILD_DIR   = os.path.join(os.path.dirname(__file__), "..", "build")
for _d in (_PYTHON_DIR, _BUILD_DIR):
    if _d not in sys.path:
        sys.path.insert(0, _d)

from stoneforge_env import StoneforgeWorldEnv

MAX_STEPS = 4000

def sample_temperature_action_recurrent(model, obs: np.ndarray, lstm_states, episode_start: np.ndarray, temperature: float):
    if temperature <= 0:
        action, next_lstm_states = model.predict(obs, state=lstm_states, episode_start=episode_start, deterministic=True)
        return int(action[0]), next_lstm_states

    model.policy.set_training_mode(False)
    obs_tensor, vectorized_env = model.policy.obs_to_tensor(obs)
    n_envs = obs_tensor.shape[0]
    
    if lstm_states is None:
        state_np = np.concatenate([np.zeros(model.policy.lstm_hidden_state_shape) for _ in range(n_envs)], axis=1)
        lstm_states = (state_np, state_np)

    with torch.no_grad():
        states_tensor = (
            torch.tensor(lstm_states[0], dtype=torch.float32, device=model.device),
            torch.tensor(lstm_states[1], dtype=torch.float32, device=model.device)
        )
        episode_starts_tensor = torch.tensor(episode_start, dtype=torch.float32, device=model.device)
        
        distribution, next_states_tensor = model.policy.get_distribution(obs_tensor, states_tensor, episode_starts_tensor)
        
        logits = distribution.distribution.logits
        scaled_logits = logits / float(temperature)
        probs = torch.softmax(scaled_logits, dim=-1)
        actions = torch.multinomial(probs, num_samples=1).squeeze(-1)
            
        actions_np = actions.cpu().numpy()
        next_states_np = (next_states_tensor[0].cpu().numpy(), next_states_tensor[1].cpu().numpy())
        
    if not vectorized_env:
        return int(actions_np[0]), next_states_np
    else:
        return actions_np, next_states_np

def eval_model(model_path: str, seeds: list[int], exit_min: int, exit_max: int, 
               deterministic: bool = True, temperature: float = 0.0, use_last_action_reward: bool = True) -> dict:
    env = StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max, use_last_action_reward=use_last_action_reward)
    model = RecurrentPPO.load(model_path)
    
    successes, lengths, returns = 0, [], []
    
    for seed in seeds:
        obs, _ = env.reset(seed=seed)
        done, ep_ret, steps = False, 0.0, 0
        lstm_states = None
        ep_start = np.ones((1,), dtype=bool)
        
        while not done and steps < MAX_STEPS:
            if deterministic or temperature <= 0:
                action, lstm_states = model.predict(
                    obs.reshape(1, -1),
                    state=lstm_states,
                    episode_start=ep_start,
                    deterministic=True
                )
                action = int(action[0])
            else:
                action, lstm_states = sample_temperature_action_recurrent(
                    model, obs.reshape(1, -1), lstm_states, ep_start, temperature
                )
                
            obs, r, term, trunc, info = env.step(action)
            ep_start = np.zeros((1,), dtype=bool)
            ep_ret += float(r)
            steps += 1
            done = term or trunc
            if info.get("reached_exit"):
                successes += 1
                break
                
        lengths.append(steps)
        returns.append(ep_ret)
        
    return {
        "success_rate": successes / len(seeds),
        "mean_len": float(np.mean(lengths)),
        "mean_ret": float(np.mean(returns))
    }

def run_experiment(seeds_train: list[int], run_train: bool, subproc: bool, plr: bool, train_script: str) -> None:
    model_dirs = []
    
    for seed in seeds_train:
        save_dir = f"models/ppo_lstm_curriculum_s{seed}"
        if plr:
            save_dir += "_plr"
        model_dirs.append(save_dir)
        
        if run_train:
            print(f"\n==========================================")
            print(f" Starte Training für Seed {seed} -> {save_dir}")
            print(f"==========================================")
            cmd = [
                ".venv/bin/python", train_script,
                "--seed", str(seed),
                "--save-dir", save_dir,
                "--no-live-map"
            ]
            if subproc:
                cmd.append("--subproc")
            if plr:
                cmd.append("--plr")
                
            # Run training subprocess
            env = os.environ.copy()
            env["PYTHONPATH"] = f"{_BUILD_DIR}:{_PYTHON_DIR}:" + env.get("PYTHONPATH", "")
            subprocess.run(cmd, env=env, check=True)
            
    # Evaluation
    print("\n==========================================")
    print(" Starte Evaluation aller Läufe...")
    print("==========================================")
    
    eval_sets = {
        "VAL (6000-6049)": {"seeds": list(range(6000, 6050)), "exit_min": 35, "exit_max": 45},
        "Testset A (7000-7049)": {"seeds": list(range(7000, 7050)), "exit_min": 35, "exit_max": 45},
        "Holdout B (8000-8049)": {"seeds": list(range(8000, 8050)), "exit_min": 35, "exit_max": 45},
    }
    
    for set_name, set_cfg in eval_sets.items():
        print(f"\nEvaluating on {set_name}...")
        for mode_name, det, temp in [("deterministisch", True, 0.0), ("stochastisch (tau=0.2)", False, 0.2)]:
            print(f"  Modus: {mode_name}")
            srs, lens, rets = [], [], []
            for m_dir in model_dirs:
                model_path = os.path.join(m_dir, "best_model.zip")
                if not os.path.exists(model_path):
                    # Fallback to phase3_best_model
                    model_path = os.path.join(m_dir, "phase3_best_model.zip")
                if not os.path.exists(model_path):
                    print(f"    ⚠️  Kein bestes Modell unter {m_dir} gefunden! Überspringe.")
                    continue
                    
                res = eval_model(model_path, set_cfg["seeds"], set_cfg["exit_min"], set_cfg["exit_max"],
                                 deterministic=det, temperature=temp, use_last_action_reward=True)
                srs.append(res["success_rate"])
                lens.append(res["mean_len"])
                rets.append(res["mean_ret"])
                print(f"    Run {m_dir}: SR={res['success_rate']:.1%}, Len={res['mean_len']:.1f}, Ret={res['mean_ret']:.2f}")
                
            if srs:
                print(f"  ==> MITTELWERT ± STD auf {set_name} ({mode_name}):")
                print(f"      Success Rate: {np.mean(srs):.1%} ± {np.std(srs):.1%}")
                print(f"      Episode Length: {np.mean(lens):.1f} ± {np.std(lens):.1f}")
                print(f"      Return: {np.mean(rets):.2f} ± {np.std(rets):.2f}")

def main() -> None:
    parser = argparse.ArgumentParser(description="Automatisiertes 3-Läufe-Protokoll")
    parser.add_argument("--no-train", action="store_true", help="Überspringe das Training und führe nur die Evaluation aus")
    parser.add_argument("--subproc", action="store_true", help="Nutze SubprocVecEnv statt DummyVecEnv beim Training")
    parser.add_argument("--plr", action="store_true", help="Swarm-Pool mit PLR-Semantik trainieren")
    parser.add_argument("--train-script", default="scripts/train_curriculum.py", help="Trainingsskript")
    args = parser.parse_args()
    
    seeds = [0, 1, 2]
    run_experiment(seeds, run_train=not args.no_train, subproc=args.subproc, plr=args.plr, train_script=args.train_script)

if __name__ == "__main__":
    main()
