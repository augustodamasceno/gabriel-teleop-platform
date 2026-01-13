#!/usr/bin/env python3
"""
Unit tests for AI Documentation Check Script

Copyright (c) 2026, Augusto Damasceno.
All rights reserved.

SPDX-License-Identifier: BSD-2-Clause
"""

import os
import sys
import subprocess
import pytest
from unittest.mock import Mock, patch, MagicMock
from pathlib import Path

# Add parent directory to path to import the script
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "scripts"))
import ai_doc_check


class TestGetApiKey:
    """Tests for get_api_key function"""
    
    def test_get_api_key_exists(self):
        """Test getting an API key that exists"""
        with patch.dict(os.environ, {"TEST_API_KEY": "test_key_123"}):
            result = ai_doc_check.get_api_key("test")
            assert result == "test_key_123"
    
    def test_get_api_key_missing(self, capsys):
        """Test getting an API key that doesn't exist"""
        with patch.dict(os.environ, {}, clear=True):
            result = ai_doc_check.get_api_key("missing")
            assert result is None
            captured = capsys.readouterr()
            assert "Warning: MISSING_API_KEY not found" in captured.out


class TestGetChangedFiles:
    """Tests for get_changed_files function"""
    
    @patch('subprocess.run')
    def test_get_changed_files_success(self, mock_run):
        """Test successfully getting changed files"""
        mock_run.return_value = Mock(
            stdout="file1.py\nfile2.js\nfile3.cpp\n",
            returncode=0
        )
        
        result = ai_doc_check.get_changed_files()
        
        assert result == ["file1.py", "file2.js", "file3.cpp"]
        mock_run.assert_called_once()
    
    @patch('subprocess.run')
    def test_get_changed_files_failure(self, mock_run, capsys):
        """Test handling git command failure"""
        mock_run.side_effect = subprocess.CalledProcessError(1, 'git')
        
        result = ai_doc_check.get_changed_files()
        
        assert result == []
        captured = capsys.readouterr()
        assert "Warning: Could not get git diff" in captured.out


class TestGetFileChanges:
    """Tests for get_file_changes function"""
    
    @patch('subprocess.run')
    def test_get_file_changes_success(self, mock_run):
        """Test successfully getting file changes"""
        mock_run.return_value = Mock(
            stdout="diff --git a/test.py b/test.py\n+new line\n-old line",
            returncode=0
        )
        
        result = ai_doc_check.get_file_changes()
        
        assert "diff" in result
        assert "new line" in result["diff"]
    
    @patch('subprocess.run')
    def test_get_file_changes_failure(self, mock_run):
        """Test handling git diff failure"""
        mock_run.side_effect = subprocess.CalledProcessError(1, 'git')
        
        result = ai_doc_check.get_file_changes()
        
        assert result == {"diff": ""}


class TestCheckDocs:
    """Tests for check_docs function"""
    
    def test_check_docs_unknown_provider(self, capsys):
        """Test with unknown provider"""
        result = ai_doc_check.check_docs("unknown", {"diff": ""}, [], {})
        
        assert result is False
        captured = capsys.readouterr()
        assert "Unknown provider: unknown" in captured.out
    
    @patch.dict(os.environ, {}, clear=True)
    def test_check_docs_no_api_key(self, capsys):
        """Test when API key is missing"""
        result = ai_doc_check.check_docs("openai", {"diff": ""}, [], {})
        
        assert result is True
        captured = capsys.readouterr()
        assert "Skipping OpenAI check" in captured.out
    
    def test_check_docs_no_code_changes(self, capsys):
        """Test when no code changes detected"""
        with patch.dict(os.environ, {"OPENAI_API_KEY": "test_key"}):
            result = ai_doc_check.check_docs("openai", {"diff": ""}, ["README.md"], {})
            
            assert result is True
            captured = capsys.readouterr()
            assert "No code changes detected" in captured.out
    
    @patch('ai_doc_check.get_api_key')
    def test_check_docs_openai_docs_needed(self, mock_get_key, capsys):
        """Test OpenAI detecting docs needed"""
        mock_get_key.return_value = "test_key"
        
        mock_client = Mock()
        mock_response = Mock()
        mock_response.choices = [Mock(message=Mock(content="DOCS_NEEDED: New API added"))]
        mock_client.chat.completions.create.return_value = mock_response
        
        with patch('builtins.__import__', side_effect=lambda name, *args, **kwargs: Mock(OpenAI=Mock(return_value=mock_client)) if name == 'openai' else __import__(name, *args, **kwargs)):
            result = ai_doc_check.check_docs(
                "openai", 
                {"diff": "+def new_function(): pass"}, 
                ["test.py"],
                {"README.md": "# Project\nNo mention of new_function"}
            )
            
            assert result is False
            captured = capsys.readouterr()
            assert "DOCS_NEEDED" in captured.out
    
    @patch('ai_doc_check.get_api_key')
    def test_check_docs_anthropic_docs_ok(self, mock_get_key, capsys):
        """Test Anthropic detecting docs OK"""
        mock_get_key.return_value = "test_key"
        
        mock_client = Mock()
        mock_response = Mock()
        mock_response.content = [Mock(text="DOCS_OK: Minor refactoring only")]
        mock_client.messages.create.return_value = mock_response
        
        with patch('anthropic.Anthropic', return_value=mock_client):
            result = ai_doc_check.check_docs(
                "anthropic", 
                {"diff": "+# Fixed typo"}, 
                ["test.py"],
                {}
            )
            
            assert result is True
            captured = capsys.readouterr()
            assert "DOCS_OK" in captured.out
    
    @patch('ai_doc_check.get_api_key')
    def test_check_docs_gemini(self, mock_get_key, capsys):
        """Test Gemini provider"""
        mock_get_key.return_value = "test_key"
        
        mock_model = Mock()
        mock_response = Mock(text="DOCS_OK: No significant changes")
        mock_model.generate_content.return_value = mock_response
        
        with patch('google.generativeai.configure'):
            with patch('google.generativeai.GenerativeModel', return_value=mock_model):
                result = ai_doc_check.check_docs(
                    "gemini", 
                    {"diff": "+# comment"}, 
                    ["test.py"],
                    {}
                )
                
                assert result is True
    
    @patch('ai_doc_check.get_api_key')
    def test_check_docs_grok(self, mock_get_key, capsys):
        """Test Grok provider"""
        mock_get_key.return_value = "test_key"
        
        mock_client = Mock()
        mock_response = Mock()
        mock_response.choices = [Mock(message=Mock(content="DOCS_NEEDED: Breaking change"))]
        mock_client.chat.completions.create.return_value = mock_response
        
        with patch('builtins.__import__', side_effect=lambda name, *args, **kwargs: Mock(OpenAI=Mock(return_value=mock_client)) if name == 'openai' else __import__(name, *args, **kwargs)):
            result = ai_doc_check.check_docs(
                "grok", 
                {"diff": "+def breaking_api(): pass"}, 
                ["test.py"],
                {}
            )
            
            assert result is False
    
    @patch('ai_doc_check.get_api_key')
    def test_check_docs_import_error(self, mock_get_key, capsys):
        """Test handling import errors"""
        mock_get_key.return_value = "test_key"
        
        with patch('builtins.__import__', side_effect=ImportError("No module")):
            result = ai_doc_check.check_docs(
                "openai", 
                {"diff": "+code"}, 
                ["test.py"],
                {}
            )
            
            assert result is False
            captured = capsys.readouterr()
            assert "Error: Required package not installed" in captured.out
    
    @patch('ai_doc_check.get_api_key')
    def test_check_docs_api_exception(self, mock_get_key, capsys):
        """Test handling API exceptions"""
        mock_get_key.return_value = "test_key"
        
        mock_client = Mock()
        mock_client.chat.completions.create.side_effect = Exception("API Error")
        
        with patch('builtins.__import__', side_effect=lambda name, *args, **kwargs: Mock(OpenAI=Mock(return_value=mock_client)) if name == 'openai' else __import__(name, *args, **kwargs)):
            result = ai_doc_check.check_docs(
                "openai", 
                {"diff": "+code"}, 
                ["test.py"],
                {}
            )
            
            assert result is False
            captured = capsys.readouterr()
            assert "Error during OpenAI check" in captured.out


class TestMain:
    """Tests for main function"""
    
    @patch('ai_doc_check.get_changed_files')
    @patch('ai_doc_check.get_file_changes')
    @patch.dict(os.environ, {}, clear=True)
    def test_main_no_api_keys(self, mock_changes, mock_files, capsys):
        """Test main with no API keys"""
        mock_files.return_value = []
        mock_changes.return_value = {"diff": ""}
        
        result = ai_doc_check.main()
        
        assert result == 0
        captured = capsys.readouterr()
        assert "No API keys provided" in captured.out
    
    @patch('ai_doc_check.get_changed_files')
    @patch('ai_doc_check.get_file_changes')
    @patch('ai_doc_check.check_docs')
    @patch.dict(os.environ, {"OPENAI_API_KEY": "test_key"})
    def test_main_docs_ok(self, mock_check, mock_changes, mock_files, capsys):
        """Test main with docs OK"""
        mock_files.return_value = ["test.py"]
        mock_changes.return_value = {"diff": "+code"}
        mock_check.return_value = True
        
        result = ai_doc_check.main()
        
        assert result == 0
        captured = capsys.readouterr()
        assert "All checks passed" in captured.out
    
    @patch('ai_doc_check.get_changed_files')
    @patch('ai_doc_check.get_file_changes')
    @patch('ai_doc_check.check_docs')
    @patch.dict(os.environ, {"ANTHROPIC_API_KEY": "test_key"})
    def test_main_docs_needed(self, mock_check, mock_changes, mock_files, capsys):
        """Test main with docs needed"""
        mock_files.return_value = ["test.py"]
        mock_changes.return_value = {"diff": "+new API"}
        mock_check.return_value = False
        
        result = ai_doc_check.main()
        
        assert result == 1
        captured = capsys.readouterr()
        assert "Documentation updates may be required" in captured.out
    
    @patch('ai_doc_check.get_changed_files')
    @patch('ai_doc_check.get_file_changes')
    @patch('ai_doc_check.check_docs')
    @patch.dict(os.environ, {"OPENAI_API_KEY": "key1", "GOOGLE_API_KEY": "key2"})
    def test_main_uses_first_provider(self, mock_check, mock_changes, mock_files, capsys):
        """Test that main uses first available provider"""
        mock_files.return_value = ["test.py"]
        mock_changes.return_value = {"diff": "+code"}
        mock_check.return_value = True
        
        result = ai_doc_check.main()
        
        # Should call with "openai" not "gemini"
        mock_check.assert_called_once()
        assert mock_check.call_args[0][0] == "openai"
        
        captured = capsys.readouterr()
        assert "Using OpenAI" in captured.out


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
