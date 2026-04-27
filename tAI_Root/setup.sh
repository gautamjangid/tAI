#!/bin/bash
# Standalone installation script for tAI
# This script compiles the project and installs it in one go.
# It can be run locally or remotely via curl.

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

MODE="install"

while getopts "iu" opt; do
  case $opt in
    i) MODE="install" ;;
    u) MODE="uninstall" ;;
    \?) echo -e "${RED}Invalid option: -$OPTARG${NC}" >&2; exit 1 ;;
  esac
done

echo -e "${GREEN}=== tAI Setup ===${NC}"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: This script must be run as root.${NC}"
    echo "Please run with sudo:"
    if [ "$MODE" == "install" ]; then
        echo "  sudo bash $0 -i"
    else
        echo "  sudo bash $0 -u"
    fi
    exit 1
fi

if [ "$MODE" == "uninstall" ]; then
    echo -e "${YELLOW}Uninstalling tAI...${NC}"
    rm -f /usr/local/bin/tAI
    rm -rf /usr/share/man/man1/tAI.1*
    # Remove from uninstall.sh too if it was there
    if [ -f "scripts/uninstall.sh" ]; then
        bash scripts/uninstall.sh >/dev/null 2>&1 || true
    fi
    echo -e "${GREEN}tAI has been uninstalled successfully.${NC}"
    exit 0
fi

echo -e "Installing tAI..."

# Determine if we are running inside the cloned repo
if [ -f "CMakeLists.txt" ] && grep -q "project(tAI)" CMakeLists.txt; then
    # We are in tAI_Root
    REPO_DIR=$(pwd)
elif [ -d "tAI_Root" ] && [ -f "tAI_Root/CMakeLists.txt" ]; then
    # We are in the parent dir
    cd tAI_Root
    REPO_DIR=$(pwd)
else
    # We need to clone it
    echo -e "Cloning repository..."
    TEMP_DIR=$(mktemp -d)
    git clone https://github.com/gautamjangid/tAI.git "$TEMP_DIR/tAI"
    cd "$TEMP_DIR/tAI/tAI_Root"
    REPO_DIR=$(pwd)
fi

echo -e "Building tAI..."

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
cd ..

echo -e "${GREEN}Build successful.${NC}"
echo -e "Installing tAI binary..."

if [ -f "scripts/install.sh" ]; then
    bash scripts/install.sh
else
    cp build/tAI /usr/local/bin/tAI
    chmod +x /usr/local/bin/tAI
fi

echo -e "${GREEN}=== Setup Complete ===${NC}"
