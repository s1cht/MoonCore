#!/usr/bin/env bash
# Script to generate MoonCore Doxygen documentation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${ROOT_DIR}"

if ! command -v doxygen &> /dev/null; then
    echo "[ERROR] Doxygen is not installed or not in PATH."
    echo "Please install Doxygen (e.g., sudo apt-get install doxygen or brew install doxygen)."
    exit 1
fi

echo "==> Generating MoonCore documentation..."
doxygen Doxyfile

echo ""
echo "==> Documentation generation completed successfully!"
echo "==> Open file://${ROOT_DIR}/docs/html/index.html in your browser."
