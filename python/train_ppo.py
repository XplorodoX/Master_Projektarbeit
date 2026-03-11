from __future__ import annotations

from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env

from stoneforge_env import StoneforgeWorldEnv


def main() -> None:
    env = make_vec_env(lambda: StoneforgeWorldEnv(), n_envs=8)

    model = PPO(
        "MlpPolicy",
        env,
        verbose=1,
        n_steps=1024,
        batch_size=256,
        learning_rate=3e-4,
        gamma=0.99,
    )
    model.learn(total_timesteps=500_000)
    model.save("ppo_stoneforge_v1")


if __name__ == "__main__":
    main()
