import argparse
import json
import os
import re
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict

PROJECT_ROOT = Path(__file__).resolve().parent.parent
SIM_CPP = PROJECT_ROOT / 'src' / 'core' / 'simulation.cpp'


def patch_pbrs_beta(beta: float) -> None:
    txt = SIM_CPP.read_text(encoding='utf-8')
    new = re.sub(r"constexpr float PBRS_BETA = [0-9\.]+F;",
                 f"constexpr float PBRS_BETA = {beta}F;",
                 txt)
    if new != txt:
        SIM_CPP.write_text(new, encoding='utf-8')


def build_core() -> None:
    # build only the python extension target for speed
    cmd = ["cmake", "--build", "build", "-j", "--target", "stoneforge_sim"]
    subprocess.run(cmd, check=True)


def run_training(timesteps: int, n_envs: int, recipe: str = 'plr') -> Dict[str, Any]:
    cmd = [
        os.environ.get('PYTHON', 'python'),
        str(PROJECT_ROOT / 'python' / 'train_roadmap.py'),
        '--recipe', recipe,
        '--timesteps', str(timesteps),
        '--n-envs', str(n_envs),
    ]
    # run and capture stdout
    proc = subprocess.run(cmd, capture_output=True, text=True)
    return {'returncode': proc.returncode, 'stdout': proc.stdout, 'stderr': proc.stderr}


@dataclass
class AblationSpec:
    name: str
    pbrs_beta: float | None = None
    plr_alpha: float | None = None
    plr_pool_size: int | None = None


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('--spec-file', type=str, help='JSON file with list of ablation specs')
    parser.add_argument('--timesteps', type=int, default=100000)
    parser.add_argument('--n-envs', type=int, default=8)
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()

    if args.spec_file:
        specs = json.loads(Path(args.spec_file).read_text(encoding='utf-8'))
    else:
        specs = [
            {'name': 'pbeta_0.0', 'pbrs_beta': 0.0},
            {'name': 'pbeta_0.5', 'pbrs_beta': 0.5},
            {'name': 'pbeta_0.9', 'pbrs_beta': 0.9},
            {'name': 'plr_alpha_08', 'plr_alpha': 0.8},
            {'name': 'plr_alpha_2', 'plr_alpha': 2.0},
            {'name': 'plr_pool_256', 'plr_pool_size': 256},
            {'name': 'plr_pool_2048', 'plr_pool_size': 2048},
        ]

    out_dir = PROJECT_ROOT / 'logs' / 'ablation_runs'
    out_dir.mkdir(parents=True, exist_ok=True)
    results = []

    for s in specs:
        spec = AblationSpec(**s)
        print(f'--- Running spec: {spec.name} ---')

        # patch PBRS if requested
        if spec.pbrs_beta is not None:
            print(f'Patching PBRS_BETA -> {spec.pbrs_beta}')
            patch_pbrs_beta(spec.pbrs_beta)
            if not args.dry_run:
                print('Building core...')
                build_core()

        # for PLR params we rely on runtime override via a small config file that
        # the training script reads (roadmap_experiments currently reads RECIPES,
        # but to avoid editing code here we write an env file the training script
        # loads if present). For now, we rely on passing the defaults and
        # post-process results.

        if args.dry_run:
            print('(dry-run) would run training now')
            res = {'returncode': 0, 'stdout': '', 'stderr': ''}
        else:
            print(f'Starting training: timesteps={args.timesteps}, n_envs={args.n_envs}')
            t0 = time.time()
            res = run_training(args.timesteps, args.n_envs)
            dt = time.time() - t0
            print(f'Training finished in {dt:.1f}s (rc={res["returncode"]})')

        run_out = out_dir / f'{spec.name}.json'
        run_data = {
            'spec': s,
            'timesteps': args.timesteps,
            'n_envs': args.n_envs,
            'result': res,
        }
        run_out.write_text(json.dumps(run_data, indent=2), encoding='utf-8')
        results.append(run_data)

    summary_path = out_dir / 'summary.json'
    summary_path.write_text(json.dumps(results, indent=2), encoding='utf-8')
    print('All done. Results saved to', out_dir)


if __name__ == '__main__':
    main()
