#!/bin/bash

# tAI Uninstall Script
# Removes tAI binary and associated files from the system

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Paths
INSTALL_PATH="/usr/local/bin/tAI"
MAN_PATH="/usr/local/share/man/man1/tAI.1"
CONFIG_PATH="$HOME/.tAI"

echo -e "${YELLOW}tAI Uninstaller${NC}"
echo "================"

# Check if running as root for system-wide uninstall
if [ ! -w "/usr/local/bin" ] && [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: Root privileges required for system-wide uninstall.${NC}"
    echo "Please run with sudo: sudo bash scripts/uninstall.sh"
    exit 1
fi

# Remove binary
if [ -f "$INSTALL_PATH" ]; then
    echo -n "Removing binary from $INSTALL_PATH... "
    rm -f "$INSTALL_PATH"
    echo -e "${GREEN}done${NC}"
else
    echo -e "${YELLOW}Binary not found at $INSTALL_PATH${NC}"
fi

# Remove man page
if [ -f "$MAN_PATH" ]; then
    echo -n "Removing man page from $MAN_PATH... "
    rm -f "$MAN_PATH"
    echo -e "${GREEN}done${NC}"
fi

# Ask about removing config
if [ -d "$CONFIG_PATH" ]; then
    read -p "Remove configuration directory at $CONFIG_PATH? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo -n "Removing config directory... "
        rm -rf "$CONFIG_PATH"
        echo -e "${GREEN}done${NC}"
    else
        echo -e "${YELLOW}Keeping configuration directory at $CONFIG_PATH${NC}"
    fi
fi

echo ""
echo -e "${GREEN}tAI successfully uninstalled!${NC}"
echo "Thank you for using tAI."
exit 0
