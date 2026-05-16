from __future__ import annotations

import importlib
from typing import Any

import gymnasium as gym
import numpy as np
from gymnasium import spaces

stoneforge_sim = importlib.import_module("stoneforge_sim")

# Observation layout from flattenObservation() in py_module.cpp:
#   [0  : gs)   grid channel  — values {0, 10, 15, 20, 30}  → /30 → {0.0, 0.33, 0.5, 0.67, 1.0}
#   [gs : 2gs)  visited mask  — values {0, 1}               → unchanged
#   [2gs]       hp            — 0..10                        → /10
#   [2gs+1]     energy        — 0..100                       → /100
#   [2gs+2]     inventory     — 0..64+                       → clipped /64
#   [2gs+3]     exitDx        — -(max) .. +(max)             → /64
#   [2gs+4]     exitDy        — -(max) .. +(max)             → /64
#
# With disable_mobs=True and disable_energy=True the only informative dynamic
# features are the grid, the visited mask, and exitDx/exitDy.

# Actions 0–3 in the C++ Action enum are exactly MoveUp/Down/Left/Right.
_N_ACTIONS = 4


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    """Stoneforge PointGoal environment.

    Wraps StoneforgeCoreEnv with:
    - 4-action movement-only space (no mining/wait/noop)
    - Manual obs normalisation → all features in [-1, 1]
    - Mobs and energy system disabled (clean navigation signal)
    - Seed-randomisation on every reset() for generalisation training
    """

    metadata = {"render_modes": []}

    def __init__(
        self,
        exit_min: int = 35,
        exit_max: int = 45,
        base_seed: int = 42,
    ) -> None:
        super().__init__()
        self.core = stoneforge_sim.StoneforgeCoreEnv(base_seed)
        self.core.configure_world_generation(
            exit_min_distance=exit_min,
            exit_max_distance=exit_max,
            force_guaranteed_path=True,
            disable_mobs=True,
            disable_energy=True,
        )
        self._base_seed = base_seed

        n = self.core.observation_size()  # = 2*gs + 5
        self._gs = (n - 5) // 2          # grid-channel size (225 for radius=7)

        self.action_space = spaces.Discrete(_N_ACTIONS)
        self.observation_space = spaces.Box(
            low=-1.0, high=1.0, shape=(n,), dtype=np.float32
        )

    # ------------------------------------------------------------------

    def _normalize(self, raw: list[int]) -> np.ndarray:
        arr = np.asarray(raw, dtype=np.float32)
        gs = self._gs
        arr[:gs] /= 30.0
        # arr[gs:2*gs] visited mask — already {0.0, 1.0}, no change needed
        arr[2 * gs]     /= 10.0              # hp
        arr[2 * gs + 1] /= 100.0            # energy
        arr[2 * gs + 2]  = np.clip(arr[2 * gs + 2], 0.0, 64.0) / 64.0  # inventory
        arr[2 * gs + 3] /= 64.0             # exitDx
        arr[2 * gs + 4] /= 64.0             # exitDy
        return arr

    # ------------------------------------------------------------------

    def reset(
        self,
        *,
        seed: int | None = None,
        options: dict[str, Any] | None = None,
    ) -> tuple[np.ndarray, dict]:
        super().reset(seed=seed)
        actual_seed = seed if seed is not None else (
            self.np_random.integers(0, 2**31 - 1)
        )
        raw = self.core.reset(int(actual_seed))
        return self._normalize(raw), {}

    def step(self, action: int) -> tuple[np.ndarray, float, bool, bool, dict]:
        raw, reward, terminated, truncated, info = self.core.step(int(action))
        return self._normalize(raw), float(reward), bool(terminated), bool(truncated), dict(info)
