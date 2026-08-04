"""Misst die Bilanz der dichten Reward-Terme einer Episode.

Der PBRS-Anteil wird dabei Schritt fuer Schritt aus der tatsaechlichen
BFS-Distanz aufsummiert und NICHT aus der Teleskopierungsformel abgeleitet:
Fuer gamma < 1 ist die Summe wegabhaengig, weil der Restterm
beta*(1-gamma)*sum(d_t)/128 nicht verschwindet.

Nur Diagnose, kein Bestandteil der Trainings-Pipeline.
"""
import sys
import numpy as np
import torch
from sb3_contrib import RecurrentPPO
from stoneforge_env import StoneforgeWorldEnv

# Feste Politik-RNG: ohne das schwankt die Messung von Lauf zu Lauf um
# einige Prozentpunkte (stochastische Aktionswahl ueber 50 Bernoulli-Welten).
torch.manual_seed(0)
np.random.seed(0)

MODEL = sys.argv[1] if len(sys.argv) > 1 else "models/ppo_lstm_curriculum_v12_s1/best_model.zip"
SEEDS = list(range(7000, 7050))
CAP = 4000
BETA, GAMMA, PBRS_NORM = 2.5, 0.999, 128.0

model = RecurrentPPO.load(MODEL, device="cpu")
env = StoneforgeWorldEnv(exit_min=35, exit_max=45)

rows = []
for seed in SEEDS:
    obs, _ = env.reset(seed=seed)
    d_prev = env.core.current_bfs_distance_to_exit()
    d0 = d_prev
    state, starts = None, np.ones((1,), dtype=bool)
    visited = {(0, 0)}
    revisit_pen, pbrs_sum, pbrs_sum_disc, d_sum = 0.0, 0.0, 0.0, 0.0
    dense_disc = 0.0
    steps, reached, done = 0, False, False
    info = {}
    while not done and steps < CAP:
        action, state = model.predict(obs, state=state, episode_start=starts, deterministic=False)
        obs, r, term, trunc, info = env.step(int(action))
        starts = np.zeros((1,), dtype=bool)
        steps += 1
        g = GAMMA ** (steps - 1)

        px, py = env.core.player_pos()
        new_tile = (px, py) not in visited
        visited.add((px, py))
        vc = env._visit_counts[(px, py)]
        rev_step = 0.03 * min(vc / 25.0, 2.0) if vc > 25 else 0.0
        revisit_pen += rev_step

        # Phi(s) = -d/128  ->  F = beta * (gamma*Phi(s') - Phi(s)) = beta/128 * (d - gamma*d')
        d_now = 0 if info.get("reached_exit", False) else env.core.current_bfs_distance_to_exit()
        f = BETA / PBRS_NORM * (d_prev - GAMMA * d_now)
        pbrs_sum += f
        pbrs_sum_disc += f * g
        d_sum += d_prev
        d_prev = d_now

        dense_disc += g * (0.02 * new_tile - 0.01 - rev_step + f)

        if info.get("reached_exit", False):
            reached = True
        done = term or trunc
    rows.append(dict(seed=seed, ok=reached, steps=steps, d0=d0, d_mean=d_sum / max(steps, 1),
                     tiles=len(visited), revisit=revisit_pen,
                     pbrs=pbrs_sum, pbrs_disc=pbrs_sum_disc, dense_disc=dense_disc,
                     early=info.get("early_stop", False)))

ok = [r for r in rows if r["ok"]]
print(f"Modell: {MODEL}")
print(f"SR: {len(ok)}/{len(rows)}   Early-Stop-Truncations: {sum(r['early'] for r in rows)}/{len(rows)}")
if not ok:
    sys.exit()


def m(k):
    return float(np.mean([r[k] for r in ok]))


steps, tiles, d0, rev, pbrs = m("steps"), m("tiles"), m("d0"), m("revisit"), m("pbrs")
expl, stepmal = 0.02 * tiles, -0.01 * steps
disc = float(np.mean([GAMMA ** r["steps"] for r in ok]))

print(f"\n--- Mittel ueber {len(ok)} erfolgreiche Episoden (Testset A, stochastisch) ---")
print(f"Median / Mittel Schritte  : {np.median([r['steps'] for r in ok]):.0f} / {steps:.0f}")
print(f"BFS-Optimum d0 {d0:.1f}   mittlere Distanz waehrend Episode {m('d_mean'):.1f}")
print(f"eta Mittel-der-Quotienten {np.mean([r['d0']/r['steps'] for r in ok]):.3f}"
      f"   |   eta Quotient-der-Mittel {d0/steps:.3f}")
print(f"neu betretene Tiles       : {tiles:.0f}")
print(f"\nExplorations-Bonus                    = {expl:+.2f}")
print(f"Schritt-Malus                         = {stepmal:+.2f}")
print(f"Revisit-Penalty                       = {-rev:+.2f}")
print(f"PBRS GEMESSEN (Summe der F)           = {pbrs:+.2f}")
print(f"  teleskopierter Anteil beta*d0/128   = {BETA*d0/PBRS_NORM:+.2f}")
print(f"  Restterm beta*(1-g)*sum(d)/128      = {pbrs - BETA*d0/PBRS_NORM:+.2f}")
terms = [expl, stepmal, -rev, pbrs]
print(f"\nSumme der dichten Terme (undiskont.)  = {sum(terms):+.2f}")
mass = sum(abs(t) for t in terms)
for lbl, t in zip(["Exploration", "Schritt", "Revisit", "PBRS"], terms):
    print(f"  Anteil {lbl:12s} = {abs(t)/mass:5.1%}")
print(f"\n--- diskontiert (gamma={GAMMA}) ---")
gamma_at_meanT = GAMMA ** steps
print(f"E[gamma^T]  (Mittel ueber Episoden)   = {disc:.3f}   <- korrekt fuer E[disk. Terminal]")
print(f"gamma^E[T]  (an der mittleren Laenge) = {gamma_at_meanT:.3f}   <- Jensen-verzerrt, NICHT verwendet")
print(f"Terminal +100 diskontiert             = {100*disc:+.1f}")
print(f"dichte Terme diskontiert (gemessen)   = {m('dense_disc'):+.2f}")
print(f"  impliziter mittlerer Diskont dicht  = {m('dense_disc')/sum(terms):.3f}"
      f"   (unabhaengig gemessen, kein Skalar auf die undiskontierte Summe)")
