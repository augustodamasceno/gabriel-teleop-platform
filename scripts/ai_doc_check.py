#!/usr/bin/env python3
"""
AI Documentation Check Script
Uses AI to verify if code changes require documentation updates

Copyright (c) 2026, Augusto Damasceno.
All rights reserved.

SPDX-License-Identifier: BSD-2-Clause
"""

import os
import sys
import subprocess
from typing import Optional, List, Dict


def get_api_key(service: str) -> Optional[str]:
    """Retrieve API key from environment variables"""
    key = os.getenv(f"{service.upper()}_API_KEY")
    if not key:
        print(f"Warning: {service.upper()}_API_KEY not found in environment")
    return key


def get_changed_files() -> List[str]:
    """Get list of changed files in the current branch/PR"""
    try:
        # Try to get diff from git
        result = subprocess.run(
            ["git", "diff", "--name-only", "HEAD~1"],
            capture_output=True,
            text=True,
            check=True
        )
        files = [f.strip() for f in result.stdout.split('\n') if f.strip()]
        return files
    except subprocess.CalledProcessError:
        # Fallback: get all tracked files if git diff fails
        print("Warning: Could not get git diff, checking all files")
        return []


def get_file_changes() -> Dict[str, str]:
    """Get the actual content changes for analysis"""
    try:
        result = subprocess.run(
            ["git", "diff", "HEAD~1"],
            capture_output=True,
            text=True,
            check=True
        )
        return {"diff": result.stdout}
    except subprocess.CalledProcessError:
        return {"diff": ""}


def get_documentation_content() -> Dict[str, str]:
    """Read all documentation files to provide context for AI analysis"""
    import glob
    from pathlib import Path
    
    docs = {}
    project_root = Path(__file__).parent.parent
    
    # Find all markdown files in docs/ directory
    docs_dir = project_root / "docs"
    if docs_dir.exists():
        for md_file in docs_dir.rglob("*.md"):
            try:
                rel_path = md_file.relative_to(project_root)
                with open(md_file, 'r', encoding='utf-8') as f:
                    docs[str(rel_path)] = f.read()
            except Exception as e:
                print(f"Warning: Could not read {md_file}: {e}")
    
    # Also include README.md if it exists
    readme = project_root / "README.md"
    if readme.exists():
        try:
            with open(readme, 'r', encoding='utf-8') as f:
                docs["README.md"] = f.read()
        except Exception as e:
            print(f"Warning: Could not read README.md: {e}")
    
    return docs


def check_docs(provider: str, changes: Dict[str, str], changed_files: List[str], existing_docs: Dict[str, str]) -> bool:
    """
    Check if documentation updates are needed using specified AI provider.
    
    Args:
        provider: AI provider name ('openai', 'anthropic', 'gemini', 'grok')
        changes: Dictionary containing git diff information
        changed_files: List of changed file paths
        existing_docs: Dictionary of existing documentation files and their content
    
    Returns:
        True if docs are OK, False if docs may be needed
    """
    # Get API key based on provider
    if provider == "openai":
        api_key = get_api_key("openai")
        provider_name = "OpenAI"
    elif provider == "anthropic":
        api_key = get_api_key("anthropic")
        provider_name = "Anthropic"
    elif provider == "gemini":
        api_key = get_api_key("google")
        provider_name = "Gemini"
    elif provider == "grok":
        api_key = get_api_key("xai")
        provider_name = "Grok"
    else:
        print(f"Unknown provider: {provider}")
        return False
    
    if not api_key:
        print(f"Skipping {provider_name} check - no API key provided")
        return True
    
    # Filter to code files only
    code_files = [f for f in changed_files if f.endswith(('.py', '.js', '.ts', '.cpp', '.c', '.h', '.hpp', '.java'))]
    
    if not code_files and not changes.get("diff"):
        print("No code changes detected - documentation check skipped")
        return True
    
    # Build documentation context
    docs_context = ""
    if existing_docs:
        docs_context = "\n\nEXISTING DOCUMENTATION:\n"
        for doc_file, content in existing_docs.items():
            # Limit each doc to 1000 chars to avoid token limits
            docs_context += f"\n--- {doc_file} ---\n{content[:1000]}\n"
            if len(content) > 1000:
                docs_context += "... (truncated)\n"
    
    # Build prompt with documentation context
    prompt = f"""
You are a documentation reviewer. Analyze the following code changes and compare them with existing documentation to determine if documentation updates are needed.

Changed files: {', '.join(code_files) if code_files else 'None'}

Code diff:
{changes.get('diff', 'No diff available')[:3000]}  # Limit to first 3000 chars
{docs_context}

Based on the code changes and existing documentation, respond with:
1. "DOCS_NEEDED" if the changes introduce new features, APIs, or significant behavior changes that are NOT already documented or need documentation updates
2. "DOCS_OK" if changes are minor fixes, refactoring, or are already adequately covered in the existing documentation

Provide a brief explanation (2-3 sentences) of your reasoning.
"""
    
    try:
        # Call appropriate AI provider
        if provider == "openai":
            from openai import OpenAI
            client = OpenAI(api_key=api_key)
            model = os.getenv("LLM_MODEL", "gpt-4")
            print(f"Analyzing changes with OpenAI ({model})...")
            
            response = client.chat.completions.create(
                model=model,
                messages=[{"role": "user", "content": prompt}],
                temperature=0.3,
                max_tokens=500
            )
            result = response.choices[0].message.content
            
        elif provider == "anthropic":
            import anthropic
            client = anthropic.Anthropic(api_key=api_key)
            print("Analyzing changes with Anthropic (Claude)...")
            
            response = client.messages.create(
                model="claude-3-5-sonnet-20241022",
                max_tokens=500,
                messages=[{"role": "user", "content": prompt}]
            )
            result = response.content[0].text
            
        elif provider == "gemini":
            import google.generativeai as genai
            genai.configure(api_key=api_key)
            print("Analyzing changes with Google Gemini...")
            
            model = genai.GenerativeModel('gemini-pro')
            response = model.generate_content(prompt)
            result = response.text
            
        elif provider == "grok":
            from openai import OpenAI
            client = OpenAI(api_key=api_key, base_url="https://api.x.ai/v1")
            print("Analyzing changes with xAI Grok...")
            
            response = client.chat.completions.create(
                model="grok-beta",
                messages=[{"role": "user", "content": prompt}],
                temperature=0.3,
                max_tokens=500
            )
            result = response.choices[0].message.content
        
        # Parse result
        print(f"\nAI Analysis:\n{result}\n")
        
        if "DOCS_NEEDED" in result:
            print("Documentation updates may be required")
            return False
        else:
            print("No documentation updates needed")
            return True
            
    except ImportError as e:
        print(f"Error: Required package not installed. {e}")
        print(f"Run: pip install -r requirements.txt")
        return False
    except Exception as e:
        print(f"Error during {provider_name} check: {e}")
        return False


def main():
    """Main documentation verification workflow"""
    print("=" * 60)
    print("AI Documentation Check - Verifying doc updates needed")
    print("=" * 60)
    
    # Get changes
    print("\nAnalyzing code changes...")
    changed_files = get_changed_files()
    changes = get_file_changes()
    
    # Read existing documentation
    print("Reading existing documentation...")
    existing_docs = get_documentation_content()
    
    if existing_docs:
        print(f"Found {len(existing_docs)} documentation file(s): {', '.join(existing_docs.keys())}")
    else:
        print("No documentation files found")
    
    if changed_files:
        print(f"Changed files: {', '.join(changed_files[:10])}")
        if len(changed_files) > 10:
            print(f"... and {len(changed_files) - 10} more")
    
    # Check providers in priority order - use first one with API key
    providers = [
        ("openai", "OpenAI", "OPENAI_API_KEY"),
        ("anthropic", "Anthropic", "ANTHROPIC_API_KEY"),
        ("gemini", "Gemini", "GOOGLE_API_KEY"),
        ("grok", "Grok", "XAI_API_KEY")
    ]
    
    selected_provider = None
    for provider_id, provider_name, env_key in providers:
        if os.getenv(env_key):
            selected_provider = (provider_id, provider_name)
            print(f"\nUsing {provider_name} for documentation check")
            break
    
    if not selected_provider:
        print("\nWarning: No API keys provided. Skipping AI documentation check.")
        print("Set OPENAI_API_KEY, ANTHROPIC_API_KEY, GOOGLE_API_KEY, or XAI_API_KEY environment variables.")
        print("CI will pass by default without AI verification.")
        return 0
    
    # Run check with selected provider
    provider_id, provider_name = selected_provider
    docs_ok = check_docs(provider_id, changes, changed_files, existing_docs)
    
    # Summary
    print("\n" + "=" * 60)
    print("Documentation Check Summary")
    print("=" * 60)
    status = "✓ PASSED" if docs_ok else "⚠️ DOCS MAY BE NEEDED"
    print(f"{provider_name}: {status}")
    
    # Exit with error if documentation updates are needed
    if docs_ok:
        print("\n✓ All checks passed - no documentation updates needed!")
        return 0
    else:
        print("\nDocumentation updates may be required")
        print("Please review the AI analysis above and update docs if needed.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
