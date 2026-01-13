# Install dependencies using uv (preferred) or pip (fallback)
#
# Copyright (c) 2026, Augusto Damasceno.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-2-Clause

Write-Host "Installing Python dependencies..." -ForegroundColor Cyan

# Check if uv is installed
$uvInstalled = Get-Command uv -ErrorAction SilentlyContinue

if ($uvInstalled) {
    Write-Host "Using uv (fast Python package manager)" -ForegroundColor Green
    uv pip install .
    Write-Host "Installing dev dependencies..." -ForegroundColor Cyan
    uv pip install ".[dev]"
} else {
    Write-Host "uv not found, falling back to pip" -ForegroundColor Yellow
    Write-Host "  Install uv for faster package management: https://github.com/astral-sh/uv" -ForegroundColor Yellow
    python -m pip install --upgrade pip
    pip install -e ".[dev]"
}

Write-Host "Dependencies installed successfully" -ForegroundColor Green
