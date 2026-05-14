#!/usr/bin/env python3
import numpy as np
from stoneforge_env import StoneforgeWorldEnv

print("=" * 80)
print("DIAGNOSE: BFS Implementation Verification")
print("=" * 80)

env = StoneforgeWorldEnv()

# Test 1: Einfacher Seed
print("\n[TEST 1] Einfacher Seed: 1")
obs, info = env.reset(seed=1)
# we can't access core attributes directly easily if they are not exposed, 
# but we can look at the observation.
# The grid is the first observations.
# We know the size from core.observation_size(). 
grid_size = env.core.observation_size() - 5
grid = obs[:grid_size]
print(f"  Grid values in observation (first 20): {grid[:20]}")
print(f"  Observed exit relative: dx={obs[-2]*128:.2f}, dy={obs[-1]*128:.2f}")

# Test 2: Eval-Seed: 7000
print("\n[TEST 2] Eval-Seed: 7000")
obs, info = env.reset(seed=7000)
print(f"  Observed exit relative: dx={obs[-2]*128:.2f}, dy={obs[-1]*128:.2f}")
print(f"  Manhattan Distance Approx: {abs(obs[-2]*128) + abs(obs[-1]*128):.2f}")

# Test 3: Grid-Encoding Check
print("\n[TEST 3] Grid-Encoding Check")
# 0=Luft, 1-6=Tile-Typen, 20=Mob, 30=Spieler -> normalized by 30
# 30/30 = 1.0 (Player)
# 0/30 = 0.0 (Air)
# Wall? usually 1 or 2. 1/30 = 0.033, 2/30 = 0.066
# Exit? Let's find it in the grid.
player_pos = np.where(grid == 1.0)[0]
if len(player_pos) > 0:
    print(f"  Player found in grid at index: {player_pos[0]}")
    # Center of a 11x11 grid is usually 60 (if 0-indexed and 121 total)
    # Let's check grid size
    print(f"  Grid total elements: {len(grid)}")

# Test 4: Manueller Step - Check Reward Signal
print("\n[TEST 4] Manual Step - Check Reward Signal")
obs, info = env.reset(seed=7000)
# Action 1 = UP, 2 = DOWN, 3 = LEFT, 4 = RIGHT (usually)
# Let's try to move and see reward
for a in range(5):
    obs, reward, term, trunc, info = env.step(a)
    print(f"  Action {a}: Reward={reward}, Terminated={term}, Info={info}")

print("\n" + "=" * 80)
print("DIAGNOSE completed.")
