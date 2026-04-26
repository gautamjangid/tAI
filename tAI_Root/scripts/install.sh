#!/bin/bash
# Install script for tAI - Terminal AI Assistant
# Usage: sudo bash install.sh [PREFIX=/usr/local]

set -e

# Get script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
BINARY_NAME="tAI"

# Installation prefix (default: /usr/local)
PREFIX="${PREFIX:-/usr/local}"
BIN_PATH="$PREFIX/bin"
MAN_PATH="$PREFIX/share/man/man1"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== tAI Installation ===${NC}"

# Check if running as root for system-wide installation
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: This script must be run as root for system-wide installation.${NC}"
    echo "Please run with sudo:"
    echo "  sudo bash $0"
    exit 1
fi

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory not found at $BUILD_DIR${NC}"
    echo "Please build the project first by running:"
    echo "  cd $PROJECT_ROOT"
    echo "  mkdir build && cd build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
    echo "  make"
    exit 1
fi

# Check if binary exists
if [ ! -f "$BUILD_DIR/$BINARY_NAME" ]; then
    echo -e "${RED}Error: Binary not found at $BUILD_DIR/$BINARY_NAME${NC}"
    echo "The build appears to be incomplete. Please run:"
    echo "  cd $BUILD_DIR"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
    echo "  make"
    exit 1
fi

echo "Installing to: $PREFIX"

# Create directories
mkdir -p "$BIN_PATH"
mkdir -p "$MAN_PATH"

# Copy and install binary
echo -n "Installing binary... "
cp "$BUILD_DIR/$BINARY_NAME" "$BIN_PATH/$BINARY_NAME"
chmod 755 "$BIN_PATH/$BINARY_NAME"
echo -e "${GREEN}Done${NC}"

# Install man page if available
if [ -f "$PROJECT_ROOT/docs/tAI.1" ]; then
    echo -n "Installing man page... "
    cp "$PROJECT_ROOT/docs/tAI.1" "$MAN_PATH/tAI.1"
    chmod 644 "$MAN_PATH/tAI.1"
    echo -e "${GREEN}Done${NC}"
fi

# Verify installation
echo ""
echo -e "${GREEN}=== Installation Complete ===${NC}"

if command -v tAI &> /dev/null; then
    INSTALLED_PATH=$(command -v tAI)
    echo -e "✓ tAI installed at: ${GREEN}$INSTALLED_PATH${NC}"
    echo ""
    echo "Quick start:"
    echo "  tAI \"Hello, how are you?\""
    echo "  tAI -f \"Latest AI news\""
    echo "  tAI --help"
    echo ""
    echo "For more help, run:"
    echo "  man tAI"
else
    echo -e "${YELLOW}Warning: tAI not found in PATH${NC}"
    echo "Make sure $BIN_PATH is in your PATH environment variable"
    echo "Current PATH: $PATH"
fi

exit 0
