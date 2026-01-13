# CI/CD in gabriel-teleop-platform

## Overview

This project uses GitHub Actions for continuous integration and continuous deployment, with automated workflows that ensure code quality and documentation accuracy.

## Workflows

### 1. AI Documentation Check ([.github/workflows/llm-verification.yml](../.github/workflows/llm-verification.yml))

**Purpose:** Automatically verifies if code changes require documentation updates using AI analysis.

**Trigger:** Runs on every push and pull request to any branch

**How it works:**
1. Checks out the repository
2. Installs Python and dependencies (uv with pip fallback)
3. Analyzes git diff to detect code changes
4. Reads existing documentation files for context
5. Uses AI (OpenAI/Anthropic/Gemini/Grok) to determine if docs need updates
6. Posts comment on PR if documentation updates are needed
7. Fails the check if docs are out of sync with code

**Supported AI Providers:**
- OpenAI (GPT-4) - Requires `OPENAI_API_KEY`
- Anthropic (Claude) - Requires `ANTHROPIC_API_KEY`
- Google (Gemini) - Requires `GOOGLE_API_KEY`
- xAI (Grok) - Requires `XAI_API_KEY`

The workflow uses the first available API key in priority order.

**Configuration:**
API keys must be added as GitHub repository secrets under Settings → Secrets and variables → Actions.

### 2. Automated Testing ([.github/workflows/tests.yml](../.github/workflows/tests.yml))

**Purpose:** Runs the test suite to ensure code quality and prevent regressions.

**Trigger:** Runs on every push and pull request

**How it works:**
1. Sets up Python environment
2. Installs test dependencies using uv (or pip fallback)
3. Runs pytest with coverage reporting
4. Uploads coverage results to Codecov (if configured)
5. Fails if any tests fail or coverage drops

**Test Coverage:**
- Unit tests for AI documentation checker
- Mock-based testing for external dependencies (git, AI APIs)
- Comprehensive error handling validation
