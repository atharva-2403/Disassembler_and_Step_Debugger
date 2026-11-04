#!/usr/bin/env bash
# Automated Regression Test Runner (Linux / WSL / Git Bash)
set -e

echo "=================================================="
echo " Starting Automated Disassembler Regression Suite "
echo "=================================================="

# 1. Clean previous build artifacts
make clean

echo ""
echo "[STEP 1/3] Running 97 MC/DC Unit Tests..."
make unit_tests

echo ""
echo "[STEP 2/3] Running State & Feature Verification..."
make test_state

echo ""
echo "[STEP 3/3] Running Adapter Bridge & Full Regression..."
make test_adapter_bridge

echo ""
echo "=================================================="
echo " ALL REGRESSION SUITES PASSED CLEANLY!            "
echo "=================================================="
