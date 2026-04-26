#!/bin/bash
# Standalone installation script for tAI
# This script compiles the project and installs it in one go.

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== tAI Standalone Installation ===${NC}"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: This script must be run as root.${NC}"
    echo "Please run with sudo:"
    echo "  sudo bash $0"
    exit 1
fi

# Get the script directory (project root)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo -e "Building tAI..."

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
cd ..

echo -e "${GREEN}Build successful.${NC}"
echo -e "Installing tAI..."

bash scripts/install.sh

echo -e "${GREEN}=== Setup Complete ===${NC}"
