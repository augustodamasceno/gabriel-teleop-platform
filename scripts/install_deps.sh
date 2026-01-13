#!/bin/bash
# Install dependencies using uv (preferred) or pip (fallback)
#
# Copyright (c) 2026, Augusto Damasceno.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-2-Clause

set -e

echo "Installing Python dependencies..."

# Check if uv is installed
if command -v uv &> /dev/null; then
    echo "Using uv (fast Python package manager)"
    uv pip install .
    echo "Installing dev dependencies..."
    uv pip install ".[dev]"
elif [ -d ".venv" ]; then
    echo "Using uv with virtual environment"
    uv pip install .
    echo "Installing dev dependencies..."
    uv pip install ".[dev]"
else
    echo "uv not found, falling back to pip"
    echo "  Install uv for faster package management: curl -LsSf https://astral.sh/uv/install.sh | sh"
    python -m pip install --upgrade pip
    pip install -e ".[dev]"
fi

echo "Dependencies installed successfully"
