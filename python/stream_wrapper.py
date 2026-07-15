"""StreamWrapper — sendet Agenten-Koordinaten pro Step an ws_map_server.

Analog zu PokéRL's StreamWrapper (github.com/PWhiddy/PokemonRedExperiments).
Statt eines externen Servers (wss://transdimensional.xyz) nutzen wir einen
lokalen ws_map_server im selben Prozess — kein Netzwerk-Overhead.

Verwendung (direkt):
    from stream_wrapper import StreamWrapper
    env = StreamWrapper(StoneforgeWorldEnv(...), agent_id=0)

Verwendung (in train_curriculum.py via make_vec_env):
    env_fns = [make_env_factory(phase, pool, i) for i in range(n_envs)]
    env = DummyVecEnv(env_fns)
"""
from __future__ import annotations

import os
import sys

import numpy as np
import gymnasium as gym

# ws_map_server aus scripts/ importieren
_SCRIPTS_DIR = os.path.join(os.path.dirname(__file__), "..", "scripts")
if _SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, _SCRIPTS_DIR)

try:
    import ws_map_server as _ws
except ImportError:
    _ws = None


class StreamWrapper(gym.Wrapper):
    """Wraps eine StoneforgeWorldEnv und streamt pro Step die Agentenposition.

    Erbt von gymnasium.Wrapper → kompatibel mit DummyVecEnv / make_vec_env.
    Analog zu PokéRL's StreamWrapper, aber in-process statt externer WS.
    """

    def __init__(self, env: gym.Env, agent_id: int = 0) -> None:
        super().__init__(env)
        self.agent_id  = agent_id
        self._step_count = 0
        self._episode    = 0

    def reset(self, *, seed: int | None = None, options: dict | None = None,
              **kwargs) -> tuple[np.ndarray, dict]:
        self._step_count = 0
        self._episode   += 1
        obs, info = self.env.reset(seed=seed, options=options, **kwargs)
        # Startposition für diese Episode merken
        pos = self.env.core.player_pos()
        self._start_x, self._start_y = int(pos[0]), int(pos[1])
        self._stream(obs=obs)
        return obs, info

    def step(self, action: int) -> tuple[np.ndarray, float, bool, bool, dict]:
        obs, reward, terminated, truncated, info = self.env.step(action)
        self._step_count += 1
        self._stream(obs=obs, reward=reward, info=info)
        return obs, reward, terminated, truncated, info

    def _stream(self, obs: np.ndarray | None = None,
                reward: float = 0.0, info: dict | None = None) -> None:
        if _ws is None:
            return
        try:
            core = self.env.core
            pos  = core.player_pos()
            bfs  = core.current_bfs_distance_to_exit()

            # Exit-Richtung aus Observation extrahieren (normiert /64 → rückrechnen)
            exit_dx = exit_dy = 0
            if obs is not None:
                n = len(obs)
                if n == 229:        # v11-Obs: [grid 225 | hp | exitDx | exitDy | step_frac]
                    exit_dx = round(float(obs[226]) * 64)
                    exit_dy = round(float(obs[227]) * 64)
                elif n == 231:      # Legacy-Obs (gs=225, extras ab 225)
                    exit_dx = round(float(obs[228]) * 64)
                    exit_dy = round(float(obs[229]) * 64)
                elif n == 456:      # CNN-Obs (extras ab 450)
                    exit_dx = round(float(obs[453]) * 64)
                    exit_dy = round(float(obs[454]) * 64)

            # Sichtfeld als Tile-IDs (obs[:gs] ist das Grid, normiert /30) —
            # die Live Map baut daraus pro Episode eine Weltkarte (Wände, Exit).
            # Grid-Größe dynamisch aus dem Env lesen (Radius ist konfigurierbar).
            grid = None
            gs = getattr(self.env, "_gs", 0)
            if obs is not None and gs and len(obs) > gs:
                grid = np.rint(np.asarray(obs[:gs]) * 30.0).astype(int).tolist()

            _ws.update_heatmap(int(pos[0]) - self._start_x,
                               int(pos[1]) - self._start_y)
            _ws.update_agent(self.agent_id, {
                "grid":    grid,
                "id":      self.agent_id,
                "x":       int(pos[0]),
                "y":       int(pos[1]),
                "start_x": self._start_x,
                "start_y": self._start_y,
                "exit_dx": exit_dx,
                "exit_dy": exit_dy,
                "seed":    int(self.env._current_seed),
                "step":    self._step_count,
                "episode": self._episode,
                "bfs":     int(bfs),
                "reward":  round(float(reward), 3),
                "success": bool((info or {}).get("reached_exit", False)),
            })
        except Exception:
            pass
