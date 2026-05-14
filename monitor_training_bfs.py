#!/usr/bin/env python3
"""
Live-Monitor für DQN Training Run mit BFS
Wertet regelmäßig Checkpoints auf 50-Seed-Set aus
"""
import os
import time
import numpy as np
from pathlib import Path
import json
from stable_baselines3 import DQN
from stoneforge_env import StoneforgeWorldEnv, ExitPotentialFieldWrapper

def evaluate_model(model_path, num_seeds=50, max_steps=4000):
    """Evaluiere Modell auf 50 Seeds"""
    if not os.path.exists(model_path):
        return None
    
    try:
        seeds = list(range(7000, 7000 + num_seeds))
        env = ExitPotentialFieldWrapper(StoneforgeWorldEnv())
        model = DQN.load(model_path)
        
        succ, lens, rets = 0, [], []
        
        for seed in seeds:
            obs, _ = env.reset(seed=seed)
            done, ep_ret, steps, reached = False, 0.0, 0, False
            
            while not done and steps < max_steps:
                action, _ = model.predict(obs, deterministic=True)
                a = int(action)
                a = 7 if a == 4 else a
                obs, r, term, trunc, info = env.step(a)
                ep_ret += float(r)
                steps += 1
                if info.get("reached_exit", False):
                    reached = True
                done = term or trunc
            
            succ += int(reached)
            lens.append(steps)
            rets.append(ep_ret)
        
        return {
            "success_rate": succ / num_seeds * 100,
            "success_count": succ,
            "mean_length": np.mean(lens),
            "mean_return": np.mean(rets),
            "std_length": np.std(lens),
            "std_return": np.std(rets),
        }
    except Exception as e:
        print(f"  ❌ Eval fehler: {e}")
        return None

def monitor_loop():
    """Kontinuierliches Monitoring während Training läuft"""
    run_dir = "tensorboard_logs/dqn_run_18"
    model_base = "best_models_dqn/best_model"
    eval_log = "dqn_run_18_eval_log.json"
    
    print("=" * 80)
    print("DQN Training Monitor (v1.2.0 + BFS)")
    print("=" * 80)
    print(f"Run: {run_dir}")
    print(f"Model: {model_base}.zip")
    print(f"Eval-Log: {eval_log}")
    print("=" * 80)
    
    eval_history = []
    last_check = 0
    check_interval = 300  # Alle 5 Minuten checken
    
    while True:
        try:
            current_time = time.time()
            
            # Periodisch evaluieren
            if current_time - last_check >= check_interval:
                print(f"\n[{time.strftime('%H:%M:%S')}] Prüfe aktuelles Modell...")
                
                result = evaluate_model(f"{model_base}.zip", num_seeds=50)
                
                if result:
                    entry = {
                        "timestamp": time.time(),
                        "time_str": time.strftime('%H:%M:%S'),
                        **result
                    }
                    eval_history.append(entry)
                    
                    # Speichere Log
                    with open(eval_log, 'w') as f:
                        json.dump(eval_history, f, indent=2)
                    
                    # Ausgabe
                    sr = result['success_rate']
                    mean_ret = result['mean_return']
                    status = "✓ GUTER PROGRESS" if sr >= 30 else "⚠ SCHWACH" if sr < 10 else "→ FORTSCHRITT"
                    
                    print(f"  Success Rate: {sr:5.1f}% ({result['success_count']}/50) {status}")
                    print(f"  Mean Return:  {mean_ret:7.2f} (±{result['std_return']:.2f})")
                    print(f"  Mean Length:  {result['mean_length']:7.1f} (±{result['std_length']:.1f})")
                    
                    # Zielkriterium
                    if sr >= 70:
                        print("\n🎉 ZIELKRITERIUM ERREICHT: 70% Success Rate!")
                        break
                
                last_check = current_time
            
            # Kurzes Warten
            time.sleep(10)
            
        except KeyboardInterrupt:
            print("\n[Monitor] Beendet.")
            break
        except Exception as e:
            print(f"[Monitor] Fehler: {e}")
            time.sleep(30)

if __name__ == "__main__":
    monitor_loop()
