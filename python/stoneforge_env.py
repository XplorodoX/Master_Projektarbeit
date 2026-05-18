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
#  [gs+3]        exitDx       — (exit.x - player.x) / 64   Luftlinie Richtung X
#  [gs+4]        exitDy       — (exit.y - player.y) / 64   Luftlinie Richtung Y
#
#  BFS-Gradientenfeld: 5 Features, Index gs+5 .. gs+9
#  [gs+5]  bfs_cur        — aktueller Pfad-Abstand / 128  ∈ [0, 1]
#  [gs+6]  delta_up       — (bfs_up   - bfs_cur) / 2, clip[-1,1]
#  [gs+7]  delta_down     — (bfs_down - bfs_cur) / 2, clip[-1,1]
#  [gs+8]  delta_left     — (bfs_left - bfs_cur) / 2, clip[-1,1]
#  [gs+9]  delta_right    — (bfs_right- bfs_cur) / 2, clip[-1,1]
#
#  Delta-Semantik: -0.5 = Schritt näher (gut), 0 = neutral, +0.5 = Schritt weiter, +1 = Wand
#  Vorteil gegenüber Absolutwerten: Richtungssignal ist ±0.5 statt ≈0.016 Differenz.
#  Wände (sentinel 9999) clippen sauber auf +1.0 (distinct von "ein Schritt weiter" = +0.5).
#
#  Stagnation-Feature: 1 Feature, Index gs+10
#  [gs+10] stuck          — min(stepsWithoutProgress / 60, 1.0)
#
#  Total: gs + 5 + 5 + 1 = 236  (für observationRadius=7 → gs=225)

_N_ACTIONS = 4
_BFS_CUR_MAX   = 128.0  # Divisor für absoluten BFS-Abstand (verhindert Sättigung >64)
_BFS_DELTA_DIV = 2.0    # Deltas ∈ {-1, 0, +1, ~9959}; /2 → {-0.5, 0, +0.5}; Wand→clip+1.0


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    """Stoneforge PointGoal environment mit BFS-Kraftfeld-Observation.

    Observation enthält neben dem 15×15-Grid und exitDx/exitDy auch das
    4-direktionale BFS-Gradientenfeld: Der Agent sieht direkt welche Richtung
    seinen Pfad zum Exit wirklich verkürzt — auch wenn Wände die Luftlinie blockieren.
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

        n_base = self.core.observation_size()   # gs + 5 = 230
        self._gs = n_base - 5                   # 225 für radius=7
        self._n_obs = n_base + 6                # +5 BFS-Kraftfeld + 1 Stagnation = 236

        self.action_space = spaces.Discrete(_N_ACTIONS)
        self.observation_space = spaces.Box(
            low=-1.0, high=1.0, shape=(self._n_obs,), dtype=np.float32
        )

    # ------------------------------------------------------------------

    def _bfs_field(self) -> np.ndarray:
        """5 BFS-Gradientenfeatures: cur (absolut) + 4 Richtungs-Deltas.

        Deltas statt Absolutwerte: bei 40-60 Tiles Distanz wären alle 5
        Absolutwerte ~0.7-1.0 mit einer Differenz von nur 1/64≈0.016 —
        für den MLP kaum unterscheidbar. Als Delta ist das Richtungssignal
        klar ±0.5, und Wände (sentinel 9999) clippen sauber auf +1.0.
        """
        cur   = self.core.current_bfs_distance_to_exit()
        up    = self.core.bfs_distance_at_offset( 0, -1)
        down  = self.core.bfs_distance_at_offset( 0,  1)
        left  = self.core.bfs_distance_at_offset(-1,  0)
        right = self.core.bfs_distance_at_offset( 1,  0)
        cur_norm = np.clip(cur / _BFS_CUR_MAX, 0.0, 1.0)
        deltas = np.clip(
            np.array([up - cur, down - cur, left - cur, right - cur], dtype=np.float32)
            / _BFS_DELTA_DIV,
            -1.0, 1.0,
        )
        return np.concatenate([[cur_norm], deltas])

    def _stuck_feature(self) -> np.ndarray:
        """1 Feature: wie lange kein BFS-Fortschritt (normalisiert auf [0,1])."""
        swp = self.core.steps_without_progress()
        return np.array([min(swp / 60.0, 1.0)], dtype=np.float32)

    def _normalize(self, raw: list[int]) -> np.ndarray:
        arr = np.asarray(raw, dtype=np.float32)
        gs = self._gs
        arr[:gs]   /= 30.0                                            # grid
        arr[gs]    /= 10.0                                            # hp
        arr[gs+1]  /= 100.0                                           # energy
        arr[gs+2]   = np.clip(arr[gs+2], 0.0, 64.0) / 64.0          # inventory
        arr[gs+3]  /= 64.0                                            # exitDx
        arr[gs+4]  /= 64.0                                            # exitDy
        return np.concatenate([arr, self._bfs_field(), self._stuck_feature()])

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
