from __future__ import annotations

import importlib
import random
import threading
from collections import deque
from typing import Any

import gymnasium as gym
import numpy as np
from gymnasium import spaces

stoneforge_sim = importlib.import_module("stoneforge_sim")


class SwarmSeedPool:
    """Thread-sicherer Pool erfolgreicher Seeds für Swarm-Training.

    Jeder erfolgreiche Seed wird nur EINMAL gespeichert (Set-Semantik).
    Beim Reset: mit swarm_prob einen zufälligen Erfolgs-Seed verwenden.
    """

    def __init__(self, maxlen: int = 500, swarm_prob: float = 0.3, plr_mode: bool = False) -> None:
        self._seeds: set[int] = set()
        self._maxlen = maxlen
        self._lock = threading.Lock()
        self.swarm_prob = swarm_prob
        self.plr_mode = plr_mode
        self.total_added = 0
        self.total_sampled = 0

    def add(self, seed: int) -> None:
        with self._lock:
            if seed in self._seeds:
                return   # kein Duplikat
            # Pool-Größe begrenzen: ältesten (zufälligen) Eintrag entfernen
            if len(self._seeds) >= self._maxlen:
                self._seeds.discard(next(iter(self._seeds)))
            self._seeds.add(seed)
            self.total_added += 1

    def remove(self, seed: int) -> None:
        with self._lock:
            self._seeds.discard(seed)

    def sample(self) -> int | None:
        with self._lock:
            if self._seeds and random.random() < self.swarm_prob:
                self.total_sampled += 1
                return random.choice(list(self._seeds))
        return None

    def stats(self) -> dict:
        with self._lock:
            return {
                "pool_size":     len(self._seeds),
                "total_added":   self.total_added,
                "total_sampled": self.total_sampled,
            }

# ─── Observation-Layout ───────────────────────────────────────────────────────
#
#  Ohne Visited Mask (use_visited_mask=False, default):
#    [0   : gs)    grid channel — {0,10,15,20,30}/30 → {0,0.33,0.5,0.67,1.0}
#    [gs+0]        hp / exitDx / exitDy / step_frac …
#    Total: 231  (gs=225, radius=7)
#
#  Mit Visited Mask (use_visited_mask=True, CNN-Variante):
#    [0   : gs)    Kanal 0 — Tile-Typen (wie oben)
#    [gs  : 2*gs)  Kanal 1 — Visited Mask: 1.0 wenn Tile in dieser Episode betreten
#    [2*gs: 2*gs+6) Extras — hp, energy, inventory, exitDx, exitDy, step_frac
#    Total: 456
#
#  Beide Layouts sind rückwärtskompatibel: Modelle mit 231 Dims ignorieren die
#  Visited-Mask-Variante; CNN-Modelle erwarten 456 Dims.

_N_ACTIONS = 4
_GRID_SIDE  = 15   # observationRadius=7 → 2*7+1=15


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    """Stoneforge PointGoal environment ohne BFS-Navigation.

    Observation: 15×15 lokales Grid + euklidischer Exit-Kompass.
    Optional: zweiter Grid-Kanal als Visited Mask (für CNN-Policy).
    """

    metadata = {"render_modes": []}

    def __init__(
        self,
        exit_min: int = 35,
        exit_max: int = 45,
        base_seed: int = 42,
        swarm_pool: SwarmSeedPool | None = None,
        use_visited_mask: bool = False,
        use_last_action_reward: bool = False,
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
        self.swarm_pool = swarm_pool
        self.use_visited_mask = use_visited_mask
        self.use_last_action_reward = use_last_action_reward
        self._current_seed: int = 0
        self._visited_tiles: set[tuple[int, int]] = set()
        self._visit_counts: dict[tuple[int, int], int] = {}
        self._steps_no_reward: int = 0

        n_base = self.core.observation_size()   # gs + 5 = 230
        self._gs = n_base - 5                   # 225 für radius=7
        self._radius = (_GRID_SIDE - 1) // 2    # 7
        self._step_count = 0
        self._max_steps = 4000

        # 231 ohne Visited Mask, 456 mit (2 Grid-Kanäle + 6 Extras)
        # Wenn use_last_action_reward aktiv, kommen 5 Features hinzu (4-dim one-hot Action + 1-dim geclippter Reward)
        if use_visited_mask:
            self._n_obs = self._gs * 2 + 6      # 450 + 6 = 456
        else:
            self._n_obs = n_base + 1            # 230 + 1 = 231
            if use_last_action_reward:
                self._n_obs += 5                # 231 + 5 = 236

        self.action_space = spaces.Discrete(_N_ACTIONS)
        self.observation_space = spaces.Box(
            low=-1.0, high=1.0, shape=(self._n_obs,), dtype=np.float32
        )

    # ------------------------------------------------------------------

    def _visited_mask_array(self, px: int, py: int) -> np.ndarray:
        """Baut die 225-dim Visited-Mask für die aktuelle Agenten-Position."""
        mask = np.zeros(self._gs, dtype=np.float32)
        r = self._radius
        side = _GRID_SIDE
        for idx in range(self._gs):
            row = idx // side
            col = idx % side
            wx = px + col - r
            wy = py + row - r
            if (wx, wy) in self._visited_tiles:
                mask[idx] = 1.0
        return mask

    def _normalize(self, raw: list[int]) -> np.ndarray:
        arr = np.asarray(raw, dtype=np.float32)
        gs = self._gs
        arr[:gs]   /= 30.0                                   # grid tiles
        arr[gs]    /= 10.0                                   # hp
        arr[gs+1]  /= 100.0                                  # energy
        arr[gs+2]   = np.clip(arr[gs+2], 0.0, 64.0) / 64.0 # inventory
        arr[gs+3]  /= 64.0                                   # exitDx
        arr[gs+4]  /= 64.0                                   # exitDy
        step_frac   = np.float32(self._step_count / self._max_steps)

        if not self.use_visited_mask:
            if self.use_last_action_reward:
                action_oh = np.zeros(4, dtype=np.float32)
                if self._last_action is not None and 0 <= self._last_action < 4:
                    action_oh[self._last_action] = 1.0
                reward_clipped = np.float32(np.clip(self._last_reward, -1.0, 1.0))
                return np.concatenate([arr, [step_frac], action_oh, [reward_clipped]])
            else:
                return np.append(arr, step_frac)                 # 231 dims

        # CNN-Variante: [grid(225) | visited(225) | extras(6)]
        px, py   = self.core.player_pos()
        visited  = self._visited_mask_array(px, py)
        extras   = np.array([
            arr[gs], arr[gs+1], arr[gs+2],   # hp, energy, inventory
            arr[gs+3], arr[gs+4],             # exitDx, exitDy
            step_frac,
        ], dtype=np.float32)
        return np.concatenate([arr[:gs], visited, extras])   # 456 dims

    # ------------------------------------------------------------------

    def reset(
        self,
        *,
        seed: int | None = None,
        options: dict[str, Any] | None = None,
    ) -> tuple[np.ndarray, dict]:
        super().reset(seed=seed)
        # Swarm: bei Auto-Reset (seed=None) ggf. erfolgreichen Seed wiederholen
        if seed is None and self.swarm_pool is not None:
            pool_seed = self.swarm_pool.sample()
            if pool_seed is not None:
                seed = pool_seed
        actual_seed = seed if seed is not None else int(
            self.np_random.integers(0, 2**31 - 1)
        )
        self._current_seed = actual_seed
        self._step_count = 0
        self._visited_tiles = {(0, 0)}         # Startposition vormarkieren
        self._visit_counts = {(0, 0): 1}
        self._steps_no_reward = 0
        self._last_action = None
        self._last_reward = 0.0
        raw = self.core.reset(actual_seed)
        return self._normalize(raw), {}

    def step(self, action: int) -> tuple[np.ndarray, float, bool, bool, dict]:
        self._step_count += 1
        raw, reward, terminated, truncated, info = self.core.step(int(action))

        # Position immer tracken (Stuck-Penalty braucht visit_counts)
        px, py = self.core.player_pos()
        self._visit_counts[(px, py)] = self._visit_counts.get((px, py), 0) + 1
        visit_count = self._visit_counts[(px, py)]
        if visit_count > 25:
            reward -= 0.03 * min(visit_count / 25.0, 2.0)

        # Visited Mask nur für CNN-Variante
        if self.use_visited_mask:
            self._visited_tiles.add((px, py))

        # Early Stopping: Truncate wenn 256 Schritte kein positiver Reward
        if reward > 0:
            self._steps_no_reward = 0
        else:
            self._steps_no_reward += 1
        if self._steps_no_reward >= 256 and not terminated:
            truncated = True
            info["early_stop"] = True

        # Last Action und Last Reward speichern
        self._last_action = action
        self._last_reward = reward

        # Swarm / PLR
        if self.swarm_pool is not None:
            if self.swarm_pool.plr_mode:
                if info.get("reached_exit"):
                    self.swarm_pool.remove(self._current_seed)
                elif terminated or truncated:
                    self.swarm_pool.add(self._current_seed)
            else:
                if info.get("reached_exit"):
                    self.swarm_pool.add(self._current_seed)

        return self._normalize(raw), float(reward), bool(terminated), bool(truncated), dict(info)
