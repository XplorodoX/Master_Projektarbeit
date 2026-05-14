#!/usr/bin/env python3
"""
50-Seed Baseline Evaluation mit BFS-Implementation
Seeds 7000-7049, deterministisch, Mining geblockt
"""
import numpy as np
from stable_baselines3 import DQN
from stoneforge_env import StoneforgeWorldEnv

print("=" * 70)
print("50-Seed Baseline Evaluation (BFS Implementation)")
print("=" * 70)

seeds = list(range(7000, 7050))
from stoneforge_env import StoneforgeWorldEnv, ExitPotentialFieldWrapper
env = ExitPotentialFieldWrapper(StoneforgeWorldEnv())

# Prüfe welches Modell vorhanden ist
import os
dqn_path = "best_models_dqn/best_model.zip"
ppo_path = "best_models_ppo/best_model.zip"

if not os.path.exists(dqn_path):
    print(f"❌ DQN Modell nicht vorhanden: {dqn_path}")
    print("   Bitte erst trainieren oder GUI-Launcher verwenden")
    exit(1)

print(f"✓ DQN Modell geladen: {dqn_path}")

model = DQN.load(dqn_path)

succ = 0
lens = []
rets = []

print("\nEvaluiere 50 Seeds...")
for i, seed in enumerate(seeds):
    obs, _ = env.reset(seed=seed)
    done, ep_ret, steps, reached = False, 0.0, 0, False
    
    while not done and steps < 4000:
        action, _ = model.predict(obs, deterministic=True)
        # Mining blocken: Action 4 → Idle
        a = int(action)
        a = 7 if a == 4 else a
        
        obs, r, term, trunc, info = env.step(a)
        ep_ret += float(r)
        steps += 1
        
        if info.get("reached_exit", False):
            reached = True
        
        done = term or trunc
    
    succ += int(reached)
    lens.append(steps)
    rets.append(ep_ret)
    
    # Progress
    progress = (i + 1) / len(seeds) * 100
    status = "✓ EXIT" if reached else "✗ FAIL"
    print(f"  [{i+1:2d}/50] Seed {seed} | Steps: {steps:4d} | Return: {ep_ret:7.2f} | {status}")

print("\n" + "=" * 70)
print("RESULTS (BFS Baseline)")
print("=" * 70)
print(f"Success Rate: {succ}/50 ({succ/50*100:5.1f}%)")
print(f"Mean Episode Length: {np.mean(lens):7.1f} (std: {np.std(lens):6.1f})")
print(f"Mean Return: {np.mean(rets):7.2f} (std: {np.std(rets):6.2f})")
print(f"Min Return: {np.min(rets):7.2f}")
print(f"Max Return: {np.max(rets):7.2f}")
print("=" * 70)
print("\n✓ Eval abgeschlossen. Ergebnisse bereit zur Dokumentation.")
