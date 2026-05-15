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
    os.path.join(_ROOT, "build"),
    os.path.join(_ROOT, "build", "Release"),
    os.path.join(_ROOT, "build", "Debug"),
    _ROOT,
    os.path.join(_ROOT, "python"),
]

for _path in reversed(_BINDING_SEARCH_PATHS):
    if _path not in sys.path and os.path.isdir(_path):
        sys.path.insert(0, _path)

stoneforge_sim = importlib.import_module("stoneforge_sim")

# Pre-compute unit vectors for the 8 compass directions once.
_FIELD_ANGLES = np.linspace(0.0, 2.0 * np.pi, 8, endpoint=False)
_FIELD_COS = np.cos(_FIELD_ANGLES).astype(np.float32)
_FIELD_SIN = np.sin(_FIELD_ANGLES).astype(np.float32)

# Permutation indices for potential-field direction samples under symmetry flips.
# Directions in order: E(0°), NE(45°), N(90°), NW(135°), W(180°), SW(225°), S(270°), SE(315°)
_LR_FLIP_DIRS = np.array([4, 3, 2, 1, 0, 7, 6, 5], dtype=int)  # negate x: E↔W, NE↔NW, SE↔SW
_UD_FLIP_DIRS = np.array([0, 7, 6, 5, 4, 3, 2, 1], dtype=int)  # negate y: NE↔SE, N↔S, NW↔SW

_GRID_SIDE = 15        # observationRadius=7 → side=15
_N_GRID = _GRID_SIDE * _GRID_SIDE  # 225
_N_VISITED = _N_GRID                # 225 — visited mask gleiche Größe wie Grid

# Observation-Layout (464 Features nach ExitPotentialFieldWrapper):
#   grid(225) | visited_mask(225) | hp | energy | inventory | exitDx | exitDy | field(8) | proximity
# exitDx/exitDy Indizes in der vollen 464-Feature-Observation.
_IDX_VISITED_START = _N_GRID                     # 225
_IDX_DX = _N_GRID + _N_VISITED + 3              # 453
_IDX_DY = _N_GRID + _N_VISITED + 4              # 454
_IDX_FIELD_START = _N_GRID + _N_VISITED + 5     # 455 — erster von 8 Richtungssamples


@dataclass
class StoneforgeConfig:
    base_seed: int = 42
    exit_min_distance: int = 35
    exit_max_distance: int = 45
    force_guaranteed_path: bool = True
    disable_mobs: bool = True  # Standard: Mobs aus fuer sauberes Navigationstraining
    disable_energy: bool = True  # Disable energy drain / starvation for navigation training
    disable_potential_field: bool = True  # Disable euclidean potential field (conflicts with BFS reward)


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    metadata = {"render_modes": []}

    def __init__(self, config: StoneforgeConfig | None = None) -> None:
        super().__init__()
        self.config = config or StoneforgeConfig()
        self.core = stoneforge_sim.StoneforgeCoreEnv(self.config.base_seed)
        self._current_seed = int(self.config.base_seed)
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
            bool(self.config.disable_energy),
        )

    # Maximale Rohwerte aus der C++ Simulation.
    _GRID_MAX = 30.0   # Spieler-Marker (hoechster Wert im Grid)
    _HP_MAX = 10.0
    _ENERGY_MAX = 100.0
    _INVENTORY_MAX = 24.0   # grobe obere Schranke (1 Item pro Slot)

    def _normalize_observation(self, obs: np.ndarray) -> np.ndarray:
        out = obs.astype(np.float32, copy=True)

        # Grid (0=floor, 10=wall, 15=exit, 20=mob, 30=player) → /30 → [0, 1]
        out[:_N_GRID] /= self._GRID_MAX

        # Visited mask [225:450] ist bereits binär 0/1 — keine Normalisierung nötig.

        # Skalare an fixen Positionen nach grid + visited_mask
        _BASE = _N_GRID + _N_VISITED  # 450
        out[_BASE] /= self._HP_MAX
        out[_BASE + 1] /= self._ENERGY_MAX
        out[_BASE + 2] /= self._INVENTORY_MAX
        out[_BASE + 3] /= 128.0   # exitDx
        out[_BASE + 4] /= 128.0   # exitDy
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
        self._current_seed = actual_seed
        obs = np.array(self.core.reset(int(actual_seed)), dtype=np.float32)

        # Query BFS distance from the simulation before normalization so the
        # value is available as the auxiliary supervision signal. If the
        # built extension does not expose `current_bfs_distance_to_exit`,
        # fall back to the exitDx/exitDy proxy from the raw observation.
        player_x, player_y = self.core.player_pos()
        try:
            distance_to_exit = int(self.core.current_bfs_distance_to_exit())
        except Exception:
            # raw observation layout: grid(225) then hp,energy,inventory,exitDx,exitDy
            raw_exit_dx = int(obs[_N_GRID + 3]) if obs.shape[0] > (_N_GRID + 3) else 0
            raw_exit_dy = int(obs[_N_GRID + 4]) if obs.shape[0] > (_N_GRID + 4) else 0
            distance_to_exit = abs(raw_exit_dx) + abs(raw_exit_dy)
        
        # Now normalize observation
        obs = self._normalize_observation(obs)
        
        return obs, {
            "world_seed": actual_seed,
            "player_x": player_x,
            "player_y": player_y,
            "distance_to_exit": distance_to_exit,
            "bfs_distance": distance_to_exit,
        }

    def step(self, action: int):
        obs, reward, terminated, truncated, info = self.core.step(int(action))
        obs_array = np.array(obs, dtype=np.float32)
        
        # Add player position to info dict for debugging and analysis
        player_x, player_y = self.core.player_pos()
        info_dict = dict(info)
        info_dict['player_x'] = player_x
        info_dict['player_y'] = player_y
        info_dict['world_seed'] = self._current_seed
        
        # Use the true BFS distance from the C++ simulation rather than the
        # straight-line proxy from exitDx/exitDy.
        try:
            distance_to_exit = int(info_dict.get('bfs_distance', self.core.current_bfs_distance_to_exit()))
        except Exception:
            # fall back to raw observation proxy if C++ API not present
            raw_exit_dx = int(obs[ _N_GRID + 3 ]) if len(obs) > (_N_GRID + 3) else 0
            raw_exit_dy = int(obs[ _N_GRID + 4 ]) if len(obs) > (_N_GRID + 4) else 0
            distance_to_exit = abs(raw_exit_dx) + abs(raw_exit_dy)
        info_dict['distance_to_exit'] = distance_to_exit
        info_dict['bfs_distance'] = distance_to_exit
        # Record extrinsic reward separately so Curriculum callback can use success-rate
        info_dict['extrinsic_reward'] = float(reward)
        
        # Now normalize observation for network input
        obs_array = self._normalize_observation(obs_array)
        
        return obs_array, float(reward), bool(terminated), bool(truncated), info_dict


class OneHotGridWrapper(gym.ObservationWrapper):
    """Converts the flattened grid into a 4-channel one-hot-like representation.

    Channels: walls, exit, player, visited (each 15x15 flattened). The rest of
    the observation (scalars + any appended potential-field features) is kept
    in-order. This wrapper preserves the end-of-array positions of exitDx/exitDy
    so it is safe to place before the `ExitPotentialFieldWrapper` or after it.
    """

    def __init__(self, env: gym.Env) -> None:
        super().__init__(env)
        old_size = int(env.observation_space.shape[0])  # type: ignore[index]
        # New size: replace 1*225 grid with 4*225 channels -> +225*3 = +675? Actually +450
        # old = 225(grid) + 225(visited) + rest
        # new = 4*225(channels) + rest => increase by 450
        new_size = old_size + (_N_GRID * 2)
        self.observation_space = spaces.Box(low=-1_000_000.0, high=1_000_000.0, shape=(new_size,), dtype=np.float32)

    def observation(self, obs: np.ndarray) -> np.ndarray:
        a = obs.astype(np.float32, copy=True)
        grid = a[:_N_GRID]
        visited = a[_IDX_VISITED_START:_IDX_VISITED_START + _N_VISITED]

        # Masks derived from normalized grid values (0..1):
        # floor ~0.0, wall ~0.333, exit ~0.5, mob ~0.667, player ~1.0
        walls = ((grid >= 0.20) & (grid < 0.45)).astype(np.float32)
        exit_mask = ((grid >= 0.45) & (grid < 0.9)).astype(np.float32)
        player = (grid > 0.9).astype(np.float32)

        # Compose channels: walls, exit, player, visited
        channels = np.concatenate([walls, exit_mask, player, visited]).astype(np.float32)

        # Scalars + tail (everything after grid+visited)
        tail = a[_N_GRID + _N_VISITED:]

        return np.concatenate([channels, tail])


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
        # If the environment config requests disabling the potential field (recommended
        # for BFS-shaped reward training), append neutral zeros and keep the
        # observation size identical to avoid breaking pretrained models.
        env_config = getattr(self.env, "config", None)
        if env_config is not None and getattr(env_config, "disable_potential_field", False):
            extra = np.zeros(9, dtype=np.float32)
            return np.concatenate([obs, extra])

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


class SymmetryAugmentationWrapper(gym.Wrapper):
    """Spiegelt Observation und Aktion zufällig — analog zur ETH-Labyrinth-Paper Methode.

    Augmentierung wird einmal pro Episode bei reset() gewürfelt und dann
    für alle Steps dieser Episode konstant gehalten.  Das ergibt effektiv
    4× mehr Trainingsdaten ohne neue Simulator-Calls:
      - Original
      - Links/Rechts gespiegelt  (exitDx negiert, Grid-Spalten, Aktionen ←↔→)
      - Oben/Unten gespiegelt    (exitDy negiert, Grid-Zeilen,  Aktionen ↑↔↓)
      - Beide kombiniert

    Nur für Training aktiv (augment=True).  Eval-Env bekommt augment=False.
    """

    def __init__(self, env: gym.Env, augment: bool = True) -> None:
        super().__init__(env)
        self._augment = augment
        self._flip_lr = False
        self._flip_ud = False

    def reset(self, **kwargs: Any):
        obs, info = self.env.reset(**kwargs)
        if self._augment:
            self._flip_lr = bool(np.random.randint(2))
            self._flip_ud = bool(np.random.randint(2))
        else:
            self._flip_lr = False
            self._flip_ud = False
        return self._transform_obs(obs), info

    def step(self, action: int):
        real_action = self._untransform_action(int(action))
        obs, reward, terminated, truncated, info = self.env.step(real_action)
        return self._transform_obs(obs), reward, terminated, truncated, info

    def _transform_obs(self, obs: np.ndarray) -> np.ndarray:
        if not self._flip_lr and not self._flip_ud:
            return obs
        obs = obs.copy()
        grid = obs[:_N_GRID].reshape(_GRID_SIDE, _GRID_SIDE)
        visited = obs[_IDX_VISITED_START:_IDX_VISITED_START + _N_VISITED].reshape(_GRID_SIDE, _GRID_SIDE)
        if self._flip_lr:
            grid = grid[:, ::-1]
            visited = visited[:, ::-1]
            obs[_IDX_DX] = -obs[_IDX_DX]
            obs[_IDX_FIELD_START:_IDX_FIELD_START + 8] = \
                obs[_IDX_FIELD_START:_IDX_FIELD_START + 8][_LR_FLIP_DIRS]
        if self._flip_ud:
            grid = grid[::-1, :]
            visited = visited[::-1, :]
            obs[_IDX_DY] = -obs[_IDX_DY]
            obs[_IDX_FIELD_START:_IDX_FIELD_START + 8] = \
                obs[_IDX_FIELD_START:_IDX_FIELD_START + 8][_UD_FLIP_DIRS]
        obs[:_N_GRID] = grid.flatten()
        obs[_IDX_VISITED_START:_IDX_VISITED_START + _N_VISITED] = visited.flatten()
        return obs

    # Actions: 0=up, 1=down, 2=left, 3=right (ReducedActionEnv-Raum)
    def _untransform_action(self, action: int) -> int:
        if self._flip_lr and action == 2:
            action = 3
        elif self._flip_lr and action == 3:
            action = 2
        if self._flip_ud and action == 0:
            action = 1
        elif self._flip_ud and action == 1:
            action = 0
        return action
