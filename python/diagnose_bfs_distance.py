#!/usr/bin/env python3
"""
Diagnose BFS Distance Calculation
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
sys.path.insert(0, os.path.dirname(__file__))

import numpy as np
from stoneforge_env import StoneforgeWorldEnv, ExitPotentialFieldWrapper

def diagnose_bfs():
    """Diagnose BFS distance calculations."""
    env = StoneforgeWorldEnv()
    env = ExitPotentialFieldWrapper(env)
    
    obs, info = env.reset(seed=7000)
    core = env.env.core
    
    print("="*60)
    print("BFS DISTANCE DIAGNOSTICS — Seed 7000")
    print("="*60)
    
    # Get player position
    player_x, player_y = core.player_pos()
    print(f"\nPlayer Position: ({player_x}, {player_y})")
    
    # Denormalize observation to get exit dx/dy
    raw_obs = obs[:-9]  # Remove the 9 potential field features
    exit_dx = raw_obs[-2] * 128.0
    exit_dy = raw_obs[-1] * 128.0
    exit_x = player_x + exit_dx
    exit_y = player_y + exit_dy
    
    print(f"Exit Offset (dx, dy): ({exit_dx:.1f}, {exit_dy:.1f})")
    print(f"Exit Position: ({exit_x:.1f}, {exit_y:.1f})")
    print(f"Euclidean Distance: {np.hypot(exit_dx, exit_dy):.1f}")
    
    # Check what BFS thinks
    print(f"\nInfo from step:")
    print(f"  player_x: {info.get('player_x')}")
    print(f"  player_y: {info.get('player_y')}")
    print(f"  distance_to_exit: {info.get('distance_to_exit')}")
    
    # Do a few steps and track
    print(f"\n{'Step':>3} | {'Pos':>10} | {'Obs Exit':>15} | {'Distance':>8}")
    print(f"{'-'*3}-+-{'-'*10}-+-{'-'*15}-+-{'-'*8}")
    
    for step in range(5):
        action, _ = DQN.load("best_models_dqn/best_model.zip").predict(obs, deterministic=True)
        obs, reward, term, trunc, info = env.step(int(action))
        
        # Denormalize
        raw_obs = obs[:-9]
        exit_dx = raw_obs[-2] * 128.0
        exit_dy = raw_obs[-1] * 128.0
        dist = np.hypot(exit_dx, exit_dy)
        
        pos = (info.get('player_x'), info.get('player_y'))
        print(f"{step:>3} | {str(pos):>10} | ({exit_dx:>6.1f}, {exit_dy:>6.1f}) | {dist:>8.1f}")

if __name__ == "__main__":
    from stable_baselines3 import DQN
    diagnose_bfs()
