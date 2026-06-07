from __future__ import annotations

import importlib
from typing import Any

import gymnasium as gym
import numpy as np
from gymnasium import spaces

stoneforge_sim = importlib.import_module("stoneforge_sim")

# Observation layout (after _normalize):
#
#  [0   : gs)    grid channel — {0,10,15,20,30}/30 → {0, 0.33, 0.5, 0.67, 1.0}
#  [gs]          hp           — /10
#  [gs+1]        energy       — /100
#  [gs+2]        inventory    — clip/64
#  [gs+3]        exitDx       — euklidische X-Richtung zum Exit, normalisiert /64  ∈ [-1, 1]
#  [gs+4]        exitDy       — euklidische Y-Richtung zum Exit, normalisiert /64  ∈ [-1, 1]
#
#  Total: gs + 5 = 230  (für observationRadius=7 → gs=225)
#
#  Kein BFS in der Observation. Der Agent kennt die Richtung zum Exit (Luftlinie),
#  muss aber selbst lernen um Wände zu navigieren — echter RL-Beitrag.

_N_ACTIONS = 4


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    """Stoneforge PointGoal environment ohne BFS-Navigation.

    Observation: 15×15 lokales Grid + euklidischer Exit-Kompass.
    Kein BFS-Gradientenfeld — der Agent muss Hindernisnavigation selbst lernen.
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
            force_guaranteed_path=False,
            disable_mobs=True,
            disable_energy=True,
        )
        self._base_seed = base_seed

        n_base = self.core.observation_size()   # gs + 5 = 230
        self._gs = n_base - 5                   # 225 für radius=7
        self._n_obs = n_base                    # 230, kein BFS-Anhang

        self.action_space = spaces.Discrete(_N_ACTIONS)
        self.observation_space = spaces.Box(
            low=-1.0, high=1.0, shape=(self._n_obs,), dtype=np.float32
        )

    # ------------------------------------------------------------------

    def _normalize(self, raw: list[int]) -> np.ndarray:
        arr = np.asarray(raw, dtype=np.float32)
        gs = self._gs
        arr[:gs]   /= 30.0                                   # grid
        arr[gs]    /= 10.0                                   # hp
        arr[gs+1]  /= 100.0                                  # energy
        arr[gs+2]   = np.clip(arr[gs+2], 0.0, 64.0) / 64.0 # inventory
        arr[gs+3]  /= 64.0                                   # exitDx (euklidisch)
        arr[gs+4]  /= 64.0                                   # exitDy (euklidisch)
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
