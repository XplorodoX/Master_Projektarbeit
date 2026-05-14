#!/bin/bash
# Setup script for Stoneforge RL Training Environment
# Usage: source setup_env.sh

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Activate venv
source "$SCRIPT_DIR/.venv/bin/activate"

# Set PYTHONPATH
export PYTHONPATH="$SCRIPT_DIR/build:$SCRIPT_DIR/python:$PYTHONPATH"

# Verify setup
echo -e "${GREEN}✓ Virtual environment activated${NC}"
echo -e "${GREEN}✓ PYTHONPATH configured:${NC}"
echo "  - Build dir: $SCRIPT_DIR/build"
echo "  - Python dir: $SCRIPT_DIR/python"

# Test imports
python -c "import stoneforge_env" 2>/dev/null
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ stoneforge_env module found${NC}"
else
    echo -e "${YELLOW}⚠ Warning: stoneforge_env import failed${NC}"
fi

echo -e "\n${GREEN}Environment ready for training!${NC}"
echo "Run training with:"
echo "  python python/train.py --algo dqn --timesteps 1000000"
