from __future__ import annotations
# pyright: reportMissingImports=false

from dataclasses import dataclass
import importlib
from typing import Any

import gymnasium as gym
import numpy as np
from gymnasium import spaces

stoneforge_sim = importlib.import_module("stoneforge_sim")


@dataclass
class StoneforgeConfig:
    base_seed: int = 42


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    metadata = {"render_modes": []}

    def __init__(self, config: StoneforgeConfig | None = None) -> None:
        super().__init__()
        self.config = config or StoneforgeConfig()
        self.core = stoneforge_sim.StoneforgeCoreEnv(self.config.base_seed)

        self.action_space = spaces.Discrete(self.core.action_space_n())
        self.observation_space = spaces.Box(
            low=0,
            high=255,
            shape=(self.core.observation_size(),),
            dtype=np.int32,
        )

    def reset(self, *, seed: int | None = None, options: dict[str, Any] | None = None):
        super().reset(seed=seed)
        actual_seed = seed if seed is not None else self.config.base_seed
        obs = np.array(self.core.reset(int(actual_seed)), dtype=np.int32)
        return obs, {}

    def step(self, action: int):
        obs, reward, terminated, truncated, info = self.core.step(int(action))
        return np.array(obs, dtype=np.int32), float(reward), bool(terminated), bool(truncated), dict(info)
