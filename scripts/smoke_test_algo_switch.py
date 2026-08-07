"""Smoke-Test des --algo ppo Pfads in train_curriculum.py.

Prueft in <1 min genau die zwei Stellen, an denen der Umbau brechen kann:
  1. Konstruktion  algo_cls(env=env, **algo_kwargs)
  2. Phasen-Reload algo_cls.load(path, env=env, **gefilterte kwargs)
Kein Curriculum, keine Stunden.
"""
import sys, tempfile, os
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "build"))
sys.path.insert(0, str(REPO / "python"))
sys.path.insert(0, str(REPO / "scripts"))
os.chdir(REPO)

from stable_baselines3.common.env_util import make_vec_env
from stoneforge_env import StoneforgeWorldEnv
import train_curriculum as tc

print("ALGOS:", {k: v[0].__name__ for k, v in tc.ALGOS.items()})

algo_cls, algo_kwargs = tc.ALGOS["ppo"]
env = make_vec_env(lambda: StoneforgeWorldEnv(exit_min=5, exit_max=12), n_envs=2, seed=1)

kw = dict(algo_kwargs)
kw["tensorboard_log"] = None          # kein TB-Muell im Smoke-Test
kw["verbose"] = 0

print("\n[1/3] Konstruktion ...")
model = algo_cls(env=env, **kw)
print("      policy:", type(model.policy).__name__)
print("      obs_space:", model.observation_space.shape)
n_params = sum(p.numel() for p in model.policy.parameters())
print(f"      Parameter: {n_params:,}")

print("\n[2/3] Kurzes learn() ...")
model.learn(total_timesteps=1024)
tmp = Path(tempfile.mkdtemp()) / "phase1_best_model.zip"
model.save(str(tmp))
print("      gespeichert:", tmp.name)

print("\n[3/3] Reload mit gefilterten Kwargs (Phasenwechsel-Pfad) ...")
reloaded = algo_cls.load(
    str(tmp), env=env,
    **{k: v for k, v in kw.items() if k not in ("verbose", "policy", "policy_kwargs")},
)
reloaded.learn(total_timesteps=512)
print("      Reload + Weitertraining OK")

# Gegenprobe: LSTM-Pfad darf nicht kaputtgegangen sein
print("\n[Gegenprobe] rppo konstruiert weiterhin ...")
r_cls, r_kw = tc.ALGOS["rppo"]
r_kw2 = dict(r_kw); r_kw2["tensorboard_log"] = None; r_kw2["verbose"] = 0
r_model = r_cls(env=env, **r_kw2)
print("      policy:", type(r_model.policy).__name__)

env.close()
print("\nSMOKE OK")
