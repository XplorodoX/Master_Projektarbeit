from __future__ import annotations
# pyright: reportMissingImports=false

from dataclasses import dataclass
import importlib
import os
import platform
import sys
from typing import Any

import gymnasium as gym
import numpy as np
from gymnasium import spaces

_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_IS_WIN = platform.system() == "Windows"

_BINDING_SEARCH_PATHS = [
    os.path.join(_ROOT, "python"),
    os.path.join(_ROOT, "build"),
    os.path.join(_ROOT, "build", "Release"),
    os.path.join(_ROOT, "build", "Debug"),
]

for _path in _BINDING_SEARCH_PATHS:
    if _path not in sys.path and os.path.isdir(_path):
        sys.path.insert(0, _path)

stoneforge_sim = importlib.import_module("stoneforge_sim")

# Pre-compute unit vectors for the 8 compass directions once.
_FIELD_ANGLES = np.linspace(0.0, 2.0 * np.pi, 8, endpoint=False)
_FIELD_COS = np.cos(_FIELD_ANGLES).astype(np.float32)
_FIELD_SIN = np.sin(_FIELD_ANGLES).astype(np.float32)


@dataclass
class StoneforgeConfig:
    base_seed: int = 42
    exit_min_distance: int = 35
    exit_max_distance: int = 45
    force_guaranteed_path: bool = True
    disable_mobs: bool = True  # Standard: Mobs aus fuer sauberes Navigationstraining


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    metadata = {"render_modes": []}

    def __init__(self, config: StoneforgeConfig | None = None) -> None:
        super().__init__()
        self.config = config or StoneforgeConfig()
        self.core = stoneforge_sim.StoneforgeCoreEnv(self.config.base_seed)
        self._apply_worldgen_config()

        self.action_space = spaces.Discrete(self.core.action_space_n())
        self.observation_space = spaces.Box(
            low=-1_000_000.0,
            high=1_000_000.0,
            shape=(self.core.observation_size(),),
            dtype=np.float32,
        )

    def _apply_worldgen_config(self) -> None:
        self.core.configure_world_generation(
            int(self.config.exit_min_distance),
            int(self.config.exit_max_distance),
            bool(self.config.force_guaranteed_path),
            bool(self.config.disable_mobs),
        )

    # Maximale Rohwerte aus der C++ Simulation.
    _GRID_MAX = 30.0   # Spieler-Marker (hoechster Wert im Grid)
    _HP_MAX = 10.0
    _ENERGY_MAX = 100.0
    _INVENTORY_MAX = 24.0   # grobe obere Schranke (1 Item pro Slot)

    def _normalize_observation(self, obs: np.ndarray) -> np.ndarray:
        out = obs.astype(np.float32, copy=True)

        # Grid: 0=Luft, 1-6=Tile-Typen, 20=Mob, 30=Spieler → [0, 1]
        # Ohne Normalisierung sieht das Netz den Spieler (30) als 30× wichtiger
        # als eine leere Kachel (0), obwohl die Werte kategorisch sind.
        n_grid = len(out) - 5
        out[:n_grid] /= self._GRID_MAX

        # Skalare normalisieren → alle in [0, 1]
        out[-5] /= self._HP_MAX       # hp
        out[-4] /= self._ENERGY_MAX   # energy
        out[-3] /= self._INVENTORY_MAX  # inventory

        # Zielrichtung: exitDx und exitDy in Tiles → /128 fuer [-1, 1]
        out[-2:] /= 128.0
        return out

    def set_curriculum_stage(self, *, exit_min_distance: int, exit_max_distance: int,
                             force_guaranteed_path: bool,
                             disable_mobs: bool | None = None) -> None:
        self.config.exit_min_distance = int(exit_min_distance)
        self.config.exit_max_distance = int(exit_max_distance)
        self.config.force_guaranteed_path = bool(force_guaranteed_path)
        if disable_mobs is not None:
            self.config.disable_mobs = bool(disable_mobs)
        self._apply_worldgen_config()

    def reset(self, *, seed: int | None = None, options: dict[str, Any] | None = None):
        super().reset(seed=seed)
        self._apply_worldgen_config()
        if seed is None:
            # Use Gym's RNG to sample a fresh world seed every episode.
            actual_seed = int(self.np_random.integers(0, np.iinfo(np.int32).max))
        else:
            actual_seed = int(seed)
        obs = np.array(self.core.reset(int(actual_seed)), dtype=np.float32)
        
        # Add player position and distance to exit BEFORE normalization
        player_x, player_y = self.core.player_pos()
        exit_dx = float(obs[-2])  # raw value in tiles
        exit_dy = float(obs[-1])  # raw value in tiles
        distance_to_exit = int(np.hypot(exit_dx, exit_dy))
        
        # Now normalize observation
        obs = self._normalize_observation(obs)
        
        return obs, {"world_seed": actual_seed, "player_x": player_x, "player_y": player_y, "distance_to_exit": distance_to_exit}

    def step(self, action: int):
        obs, reward, terminated, truncated, info = self.core.step(int(action))
        obs_array = np.array(obs, dtype=np.float32)
        
        # Add player position to info dict for debugging and analysis
        player_x, player_y = self.core.player_pos()
        info_dict = dict(info)
        info_dict['player_x'] = player_x
        info_dict['player_y'] = player_y
        
        # Add distance to exit (from RAW observation exitDx and exitDy BEFORE normalization)
        # Raw observation last two values are exit offset in tiles
        exit_dx = float(obs[-2])  # raw value in tiles
        exit_dy = float(obs[-1])  # raw value in tiles
        distance_to_exit = int(np.hypot(exit_dx, exit_dy))
        info_dict['distance_to_exit'] = distance_to_exit
        
        # Now normalize observation for network input
        obs_array = self._normalize_observation(obs_array)
        
        return obs_array, float(reward), bool(terminated), bool(truncated), info_dict


class ExitPotentialFieldWrapper(gym.ObservationWrapper):
    """Appends 9 exit-potential-field features to the observation.

    The last two elements of the base observation are exitDx/128 and exitDy/128.
    This wrapper reconstructs the raw offsets and samples the potential field
    1/(1+dist) at 8 compass directions (radius SAMPLE_RADIUS tiles away) plus
    once at the agent's current position (proximity).  All values are in [0,1].

    The directional gradient tells the agent which way the exit "pulls" without
    the network having to learn the nonlinear transformation from raw dx/dy.
    """

    SAMPLE_RADIUS: float = 10.0
    FIELD_SCALE: float = 40.0  # distance (tiles) at which strength = 0.5

    def __init__(self, env: gym.Env) -> None:
        super().__init__(env)
        old_size = int(env.observation_space.shape[0])  # type: ignore[index]
        new_size = old_size + 9  # 8 directional samples + 1 proximity
        self.observation_space = spaces.Box(
            low=-1_000_000.0,
            high=1_000_000.0,
            shape=(new_size,),
            dtype=np.float32,
        )

    def observation(self, obs: np.ndarray) -> np.ndarray:
        # Undo the /128 normalisation applied by StoneforgeWorldEnv.
        raw_dx = obs[-2] * 128.0
        raw_dy = obs[-1] * 128.0

        # 8 directional samples: for each compass direction d with unit radius R,
        # the sample point sits at (player + d*R).  The exit is at (exitDx, exitDy)
        # relative to player, so relative to the sample point it is at
        # (exitDx - d.x*R, exitDy - d.y*R).
        sx = raw_dx - _FIELD_COS * self.SAMPLE_RADIUS  # shape (8,)
        sy = raw_dy - _FIELD_SIN * self.SAMPLE_RADIUS
        sample_dists = np.hypot(sx, sy)
        field_samples = (1.0 / (1.0 + sample_dists / self.FIELD_SCALE)).astype(np.float32)

        # Proximity: field strength at the agent's own position.
        current_dist = float(np.hypot(raw_dx, raw_dy))
        proximity = np.float32(1.0 / (1.0 + current_dist / self.FIELD_SCALE))

        extra = np.append(field_samples, proximity)
        return np.concatenate([obs, extra])
