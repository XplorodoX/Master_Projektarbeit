#!/bin/bash
# Setup script for Stoneforge RL Training Environment
# Usage: source setup_env.sh

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Repo-Wurzel bestimmen (Skript liegt in scripts/, venv & build liegen eine Ebene höher)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( dirname "$SCRIPT_DIR" )"

# Activate venv
if [ ! -f "$ROOT_DIR/.venv/bin/activate" ]; then
    echo "✗ Kein venv gefunden. Einmalig anlegen mit:"
    echo "  python3 -m venv $ROOT_DIR/.venv && $ROOT_DIR/.venv/bin/pip install -r $ROOT_DIR/requirements.txt"
    return 1 2>/dev/null || exit 1
fi
source "$ROOT_DIR/.venv/bin/activate"

# Set PYTHONPATH
export PYTHONPATH="$ROOT_DIR/build:$ROOT_DIR/python:$PYTHONPATH"

# Verify setup
echo -e "${GREEN}✓ Virtual environment activated${NC}"
echo -e "${GREEN}✓ PYTHONPATH configured:${NC}"
echo "  - Build dir: $ROOT_DIR/build"
echo "  - Python dir: $ROOT_DIR/python"

# Test imports
python -c "import stoneforge_env" 2>/dev/null
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ stoneforge_env module found${NC}"
else
    echo -e "${YELLOW}⚠ Warning: stoneforge_env import failed${NC}"
fi

echo -e "\n${GREEN}Environment ready for training!${NC}"
echo "Run training with:"
echo "  python scripts/train.py --algo dqn --timesteps 1000000"
