#!/usr/bin/env python3
"""
Agent Behavior Analysis Script
Spielt ein einzelnes Spiel mit detailliertem Logging wo der Agent steckenbleibt.
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
sys.path.insert(0, os.path.dirname(__file__))

import numpy as np
from stable_baselines3 import DQN
from stoneforge_env import StoneforgeWorldEnv, ExitPotentialFieldWrapper

def analyze_agent_behavior(model_path, seed=7000, verbose=True):
    """
    Spiele ein Spiel und analysiere Agent-Verhalten Schritt für Schritt.
    """
    # Environment setup
    env = StoneforgeWorldEnv()
    env = ExitPotentialFieldWrapper(env)
    
    # Model laden
    try:
        model = DQN.load(model_path)
        print(f"✓ Model geladen: {model_path}")
    except Exception as e:
        print(f"✗ Fehler beim Laden des Models: {e}")
        return
    
    # Spiel starten
    obs, info = env.reset(seed=seed)
    print(f"\n{'='*60}")
    print(f"AGENT BEHAVIOR ANALYSIS — Seed {seed}")
    print(f"{'='*60}")
    print(f"Start Position: ({info.get('player_x', '?')}, {info.get('player_y', '?')})")
    print(f"Start Distance to Exit: {info.get('distance_to_exit', '?')} tiles")
    print(f"Max Steps: {info.get('max_steps', 4000)}\n")
    
    # Get base env for accessing raw observation
    base_env = env.env  # Unwrap to get base StoneforgeWorldEnv
    
    # Tracking
    done = False
    step = 0
    prev_pos = (info.get('player_x', 0), info.get('player_y', 0))
    stuck_steps = 0
    max_stuck = 0
    actions_taken = {0: 0, 1: 0, 2: 0, 3: 0}  # up, down, left, right
    total_reward = 0
    
    action_names = {0: "UP", 1: "DOWN", 2: "LEFT", 3: "RIGHT", 4: "MINE"}
    
    print(f"{'Step':>5} | {'Action':>6} | {'Pos':>12} | {'Dist':>5} | {'Reward':>7} | Status")
    print(f"{'-'*5}-+-{'-'*6}-+-{'-'*12}-+-{'-'*5}-+-{'-'*7}-+-{'-'*20}")
    
    while not done and step < 4100:
        # Predict action
        action, _ = model.predict(obs, deterministic=True)
        action_int = int(action)
        
        # Step environment
        obs, reward, term, trunc, info = env.step(action_int)
        done = term or trunc
        
        # Current position and distance
        curr_pos = (info.get('player_x', 0), info.get('player_y', 0))
        distance = info.get('distance_to_exit', 0)
        
        # Detect if stuck
        if curr_pos == prev_pos:
            stuck_steps += 1
            max_stuck = max(max_stuck, stuck_steps)
            status = f"STUCK ({stuck_steps})"
        else:
            stuck_steps = 0
            status = "moving"
        
        # Check if reached exit
        if info.get('reached_exit', False):
            status = "EXIT FOUND! ✓"
        
        # Track actions
        if action_int < 4:
            actions_taken[action_int] += 1
        
        total_reward += float(reward)
        
        # Print every Nth step for readability
        if step % 10 == 0 or done or info.get('reached_exit', False):
            print(f"{step:>5} | {action_names[action_int]:>6} | {str(curr_pos):>12} | {distance:>5} | {reward:>7.2f} | {status:>20}")
        
        prev_pos = curr_pos
        step += 1
    
    # Summary
    print(f"{'-'*5}-+-{'-'*6}-+-{'-'*12}-+-{'-'*5}-+-{'-'*7}-+-{'-'*20}")
    print(f"\n{'='*60}")
    print(f"SUMMARY")
    print(f"{'='*60}")
    print(f"Total Steps: {step}")
    print(f"Total Reward: {total_reward:.2f}")
    print(f"Exit Found: {'YES ✓' if info.get('reached_exit', False) else 'NO ✗'}")
    print(f"Max Consecutive Stuck: {max_stuck} steps")
    print(f"\nAction Distribution:")
    for action_id, name in action_names.items():
        if action_id < 4:
            pct = (actions_taken[action_id] / max(step, 1)) * 100
            print(f"  {name:>6}: {actions_taken[action_id]:>4} ({pct:>5.1f}%)")
    
    print(f"{'='*60}\n")

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Analyze agent behavior in a single episode")
    parser.add_argument("--model", default="best_models_dqn/best_model.zip", help="Path to model")
    parser.add_argument("--seed", type=int, default=7000, help="Seed to play")
    args = parser.parse_args()
    
    analyze_agent_behavior(args.model, seed=args.seed)
