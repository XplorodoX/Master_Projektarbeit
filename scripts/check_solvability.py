"""Solvability Check script for Stoneforge RL environment maps."""
from __future__ import annotations

import os
import sys
import random

_PYTHON_DIR  = os.path.join(os.path.dirname(__file__), "..", "python")
_BUILD_DIR   = os.path.join(os.path.dirname(__file__), "..", "build")
for _d in (_PYTHON_DIR, _BUILD_DIR):
    if _d not in sys.path:
        sys.path.insert(0, _d)

from stoneforge_env import StoneforgeWorldEnv

def check_seeds(seeds: list[int], exit_min: int, exit_max: int, name: str) -> float:
    env = StoneforgeWorldEnv(exit_min=exit_min, exit_max=exit_max)
    solvable_count = 0
    for seed in seeds:
        env.reset(seed=seed)
        # Immediately after reset, player is at spawn (0, 0).
        # We check if there is a valid path to the exit.
        if env.core.is_path_to_exit_reachable():
            solvable_count += 1
    
    rate = solvable_count / len(seeds)
    print(f"[{name}] exit={exit_min}-{exit_max} | Solvable: {solvable_count}/{len(seeds)} ({rate:.1%})")
    return rate

def main() -> None:
    print("Starte Lösbarkeitsprüfung der Welten (force_guaranteed_path=False)...")
    
    # 1. Val-Seeds
    val_seeds = list(range(6000, 6050))
    check_seeds(val_seeds, 35, 45, "Validation Seeds 6000-6049")
    
    # 2. Testset A
    test_a_seeds = list(range(7000, 7050))
    check_seeds(test_a_seeds, 35, 45, "Testset A Seeds 7000-7049")
    
    # 3. Holdout B
    holdout_b_seeds = list(range(8000, 8049)) # Holdout B is 8000-8049 (seeds-b default in eval_comparison.py uses end=8050)
    # let's do 8000 to 8050: range(8000, 8050)
    check_seeds(list(range(8000, 8050)), 35, 45, "Holdout B Seeds 8000-8049")
    
    # 4. Phase 1 random seeds (1000 seeds)
    random.seed(42)
    phase1_seeds = [random.randint(0, 1000000) for _ in range(1000)]
    check_seeds(phase1_seeds, 5, 12, "Phase 1 Random Seeds (1000)")
    
    # 5. Phase 2 random seeds (1000 seeds)
    phase2_seeds = [random.randint(0, 1000000) for _ in range(1000)]
    check_seeds(phase2_seeds, 12, 25, "Phase 2 Random Seeds (1000)")
    
    # 6. Phase 3 random seeds (1000 seeds)
    phase3_seeds = [random.randint(0, 1000000) for _ in range(1000)]
    check_seeds(phase3_seeds, 25, 45, "Phase 3 Random Seeds (1000)")

if __name__ == "__main__":
    main()
