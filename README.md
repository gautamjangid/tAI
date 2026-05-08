# tAI – Terminal AI Assistant

[![Version](https://img.shields.io/badge/version-1.1.1-blue.svg)](https://github.com/gautamjangid/tAI)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](https://github.com/gautamjangid/tAI)
[![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)](https://github.com/gautamjangid/tAI)

**tAI** is a lightweight, cross-platform CLI AI assistant written in C++.
It works entirely in the terminal, supports multiple free or free‑tier backends, and runs well even on legacy hardware.

---

## ✨ Features

- **Terminal‑only** – no GUI dependencies, minimal resource usage
- **Multi‑backend support** (all free or free‑tier):
  - **DuckDuckGo** – Instant Answer API, no API key required (**default engine**)
  - **Ollama (local)** – run models on `localhost:11434` (no API key required)
  - **Ollama Cloud** – cloud‑hosted Ollama models (requires API key)
  - **Hugging Face** – access 200K+ models via the Inference API (free tier available)
  - **xAI Grok** – advanced reasoning (free tier available)
  - **OpenRouter** – aggregate access to many models (free tier available)
- **tgpt‑like flags**:
  - `-c` – code generation mode (no explanations)
  - `-s` – **shell mode**: AI generates a shell command, tAI asks if you want to execute it
  - `--system "prompt"` – set a custom system prompt
  - `-f <file>` – **file mode**: include a file's content as context for the query
  - `--config` – open the config file in your default text editor
  - `-d "engine"` / `--default "engine"` – set the default engine in your config
  - `--version` – print the current tAI version and exit
- **Typing effect** – responses print character-by-character at a natural reading pace
- **Error logging** – every runtime error is written to `~/.tAI/logs/error-<YYYYMMDDHHmmss>.log`
- **History saving** – saves queries and responses as `.html` files in `~/.tAI/history/YYYY-MM-DD/HH.html`
- **Config file** – save API keys, templates, and defaults in `~/.tAI/config.json`
- **Versioned** – current release is `v1.1.1` (defined in `Config.h` as `TAI_VERSION`)
- **Platform independent** – compiles and runs on Linux, macOS, and Windows (via CMake + libcurl)
- **Privacy‑respecting** – direct API calls, no anonymous proxies

---

## 🚀 Quick Start

After building (see below), simply run:

```bash
# DuckDuckGo (default, no key needed)
tAI "What is the capital of France?"

# Shell mode – generates a command and asks to execute
tAI -s "list all .cpp files recursively"

# File mode – include file as context
tAI -f README.md "Summarize this file"
tAI -f error.log "What is causing this error?"

# Custom system prompt
tAI --system "You are a Linux expert" "How do I install Docker?"

# Code generation mode
tAI -c "Write a Python function to sort a list"

# Open config in your default editor
tAI --config

# Use a specific engine
tAI -m ollama "What is 2+2?"
tAI -m ollama_cloud --api-key ollama-xxxx "Explain quantum computing"
tAI -m huggingface --api-key hf_xxxx "Tell me about AI"
tAI -m grok --api-key xai-xxxx "What is consciousness?"
tAI -m openrouter --api-key sk-or-xxxx "Summarize the history of AI"

# Set a default engine and exit
tAI -d ollama

# Print version
tAI --version
```

---

## ⚙️ Configuration

On first run, tAI creates `~/.tAI/config.json` (or `%USERPROFILE%\.tAI\config.json` on Windows).
Edit it manually or run `tAI --config` to open it in your default editor:

```json
{
  "default_engine": "duckduckgo",
  "duckduckgo": {
    "api_endpoint": "https://api.duckduckgo.com",
    "enabled": true
  },
  "ollama": {
    "api_endpoint": "http://localhost:11434",
    "template": "",
    "enabled": true
  },
  "ollama_cloud": {
    "api_endpoint": "https://api.ollama.cloud",
    "api_key": "",
    "template": "",
    "enabled": false
  },
  "huggingface": {
    "api_endpoint": "https://router.huggingface.co/v1/chat/completions",
    "api_key": "",
    "model": "meta-llama/Llama-2-7b-chat-hf",
    "template": "",
    "enabled": false
  },
  "grok": {
    "api_endpoint": "https://api.groq.com/openai/v1/chat/completions",
    "api_key": "",
    "template": "",
    "enabled": false
  },
  "openrouter": {
    "api_endpoint": "https://openrouter.ai/api/v1",
    "api_key": "",
    "referer": "",
    "template": "",
    "enabled": false
  }
}
```

**Key settings:**
- `default_engine`: Engine used when `-m` is not supplied (default: `duckduckgo`). Change via `tAI -d <engine>` or `tAI --config`.
- `template`: Optional string to wrap user queries. `{query}` is replaced by the user's prompt. E.g. `"template": "Summarize this: {query}"`.
- Each engine section has its endpoint, optional API key, and an `enabled` flag.
- Override the config path with `--config /path/to/config.json`.

---

## 🔧 CLI Reference

| Flag | Argument | Description |
|------|----------|-------------|
| `-c` | — | Code mode (no explanations, code only) |
| `-s` | — | Shell mode: generate a command + ask to execute |
| `--system` | `<prompt>` | Custom system prompt |
| `-f` | `<file>` | Include file content as context |
| `-m` | `<engine>` | Select AI engine |
| `-d`, `--default` | `<engine>` | Set default engine in config and exit |
| `--api-key` | `<key>` | API key for selected engine |
| `--config` | *(none)* | Open config file in default editor |
| `--config` | `<path>` | Use a custom config file path |
| `--version` | — | Show version and exit |
| `-h`, `--help` | — | Show help message |

---

## 🐚 Shell Mode (`-s`)

Shell mode asks the AI to generate a shell command for your request, displays it, and interactively asks if you want to execute it — just like `tgpt -s`.

```bash
$ tAI -s "find all files larger than 100MB"

  find / -type f -size +100M 2>/dev/null

Execute this command? [y/N]: y

/home/user/large_backup.tar.gz
/var/log/huge.log
```

- The AI is instructed to return **only the raw command** (no markdown, no explanation).
- If you enter `y` or `Y`, tAI runs the command with `system()`.
- Any other input (or Enter) cancels execution safely.

---

## 📂 File Mode (`-f`)

File mode reads a local file and prepends its content as context to the prompt:

```bash
# Summarize a README
tAI -f README.md "Summarize this in 3 bullet points"

# Debug an error log
tAI -f /var/log/syslog "What is causing errors here?"

# Code review
tAI -f src/main.cpp "Review this code for bugs"
```

The file content is sent as:
```
[File: <filename>]
<file content>

User query: <your prompt>
```

---

## 🔍 DuckDuckGo Engine

The DuckDuckGo engine uses the **Instant Answer API** (`https://api.duckduckgo.com/`) — completely **free, no API key required**, and the **default engine**.

It parses:
1. **Direct answers** – e.g. conversions, facts
2. **Abstract summaries** – Wikipedia-style text with source attribution
3. **Definitions** – word/term definitions
4. **Related topics** – up to 5 bullet points of related info

> **Note:** DDG Instant Answer is best for factual, specific queries (definitions, conversions, geography, science facts). For open-ended, creative, or coding questions, switch to an LLM engine: `tAI -m ollama "…"` or `tAI -m openrouter "…"`.

---

## 📝 Open Config (`--config`)

```bash
# Open config in your default editor (no-argument form)
tAI --config
```

This ensures the config file exists (creates default if missing), then opens it in:
- **Windows**: Notepad
- **Linux/macOS**: `$VISUAL` → `$EDITOR` → `xdg-open` → `nano` → `vi`

After editing, save and close — tAI will pick up changes on the next run.

---

## 🖨️ Typing Effect

tAI prints AI responses character-by-character at approximately **18 ms per character** (shorter pauses on spaces/newlines for natural reading pace). This:
- Makes long responses easier to follow line-by-line
- Reduces perceived wait time (output starts immediately)
- Helps manage token consumption in time-sensitive sessions

*(Shell mode bypasses the typing effect — the command is shown instantly for fast interaction.)*

---

## 📋 Error Logging

Every error tAI encounters is automatically saved to a dedicated log file:

```
~/.tAI/logs/error-<YYYYMMDDHHmmss>.log
```

Example path: `~/.tAI/logs/error-20260428224000.log`

Each log file contains:
```
=== tAI Error Log ===
Timestamp : 2026-04-28 22:40:00
Context   : API call – engine: grok
Message   : curl error: Could not resolve host: api.x.ai
=====================
```

> **Tip:** If tAI exits with an error, check `~/.tAI/logs/` for the full details.

---

## 🛠️ Build from Source

Requirements: CMake ≥ 3.14, C++17 compiler, libcurl (with headers). `nlohmann/json` is fetched automatically via CMake.

### Linux / macOS

```bash
# Install dependencies (Debian/Ubuntu):
sudo apt install cmake g++ libcurl4-openssl-dev

# macOS (Homebrew):
brew install cmake curl

# Build
cd tAI_Root
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

The binary `tAI` will be in `build/`.

### Windows (using vcpkg or MSYS2)

```powershell
# Install libcurl via vcpkg
vcpkg install curl

# Configure with CMake:
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

---

## 📦 Install

### Linux/macOS (One-Command Install)

```bash
curl -sSL https://raw.githubusercontent.com/gautamjangid/tAI/main/tAI_Root/setup.sh | sudo bash -s -- -i
```

This script will automatically clone the latest version into a temporary directory, build it, and install it system-wide.

*(Alternatively: clone the repo, `cd tAI_Root`, and run `sudo bash setup.sh -i`)*

### Uninstall

```bash
curl -sSL https://raw.githubusercontent.com/gautamjangid/tAI/main/tAI_Root/setup.sh | sudo bash -s -- -u
```

*(Or locally: `sudo bash setup.sh -u`)*

### Windows (vcpkg)

Manually copy `build/Release/tAI.exe` to a folder in your PATH, or run the provided batch files (admin).

---

## 🧪 Supported Engines

| Engine          | Flag              | Requires API Key? | Notes                              |
|-----------------|-------------------|-------------------|------------------------------------|
| DuckDuckGo      | `-m duckduckgo`   | **No** (default)  | Instant answers, factual queries   |
| Ollama (local)  | `-m ollama`       | No                | Self-hosted LLM on localhost       |
| Ollama Cloud    | `-m ollama_cloud` | Yes               | Cloud-hosted Ollama                |
| Hugging Face    | `-m huggingface`  | Yes (free tier)   | 200K+ models                       |
| xAI Grok        | `-m grok`         | Yes (free tier)   | Advanced reasoning                 |
| OpenRouter      | `-m openrouter`   | Yes (free tier)   | Aggregated LLM access              |

---

## 🔑 Getting API Keys

### Ollama Cloud
- Visit https://ollama.cloud (when available) and create an API key.

### Hugging Face
- Sign in at https://huggingface.co and go to **Settings → Access Tokens**.
- Create a new token (read‑only is sufficient).

### xAI Grok
- Sign up at https://x.ai and generate an API key in the console.

### OpenRouter
- Sign in at https://openrouter.ai and create a key in the **Keys** section.

---

## 🗂️ Project Structure

```
tAI_Root/
├── CMakeLists.txt
├── setup.sh                 (install -i / uninstall -u)
├── scripts/
│   ├── install.sh
│   ├── uninstall.sh
│   ├── windows_install.bat
│   └── windows_uninstall.bat
├── include/tAI/             (public headers)
│   ├── Config.h             ← TAI_VERSION defined here
│   ├── IApiClient.h
│   ├── DuckDuckGoClient.h   ← NEW: no-key default engine
│   ├── OllamaClient.h
│   ├── OllamaCloudClient.h
│   ├── HuggingfaceClient.h
│   ├── GrokClient.h
│   ├── OpenRouterClient.h
│   └── Utils.h              ← logError(), typingPrint() declared here
├── src/                     (implementation files)
│   ├── main.cpp
│   ├── Config.cpp
│   ├── DuckDuckGoClient.cpp ← NEW
│   ├── OllamaClient.cpp
│   ├── OllamaCloudClient.cpp
│   ├── HuggingfaceClient.cpp
│   ├── GrokClient.cpp
│   ├── OpenRouterClient.cpp
│   └── Utils.cpp            ← logError(), typingPrint() implemented here
└── docs/                    (design notes & screenshots)
```

---

## 📁 Runtime Data Layout

```
~/.tAI/
├── config.json              ← User configuration (edit with: tAI --config)
├── history/
│   └── YYYY-MM-DD/
│       └── HH.html          ← Hourly HTML chat logs
└── logs/
    └── error-<timestamp>.log ← Per-error log files
```

---

## 🤝 Contributing

Pull requests are welcome. If you add a new backend, please:

1. Derive from `IApiClient` and implement `chat()`.
2. Add the new client to `main.cpp` dispatcher.
3. Update `Config.h`/`Config.cpp` with a nested config section.
4. Add the new files to `CMakeLists.txt`.
5. Document the engine in this README.

---

## 📄 License

This project is open-source under the MIT License.

---

## 📝 Changelog

### v1.1.1 (2026-05-08)
- ⚙️ **API endpoints corrections** - API endpoints updated correctly in config file

### v1.1.0 (2026-05-07)
- 🔍 **DuckDuckGo engine** – new default engine using the Instant Answer API (no API key required)
- 🐚 **Shell mode (`-s`)** – generate a shell command and interactively execute it (like `tgpt -s`)
- 📂 **File mode (`-f <file>`)** – include a file's content as context for any query
- ⚙️ **`tAI --config`** – open the config file in your default text editor
- 📝 **`--system <prompt>`** – dedicated flag for custom system prompts (replaces `-s <prompt>`)

### v1.0.0 (2026-04-28)
- 🎉 Initial versioned release
- ✨ **Typing effect** – responses now print character-by-character at a natural pace
- 🪵 **Error logging** – all errors saved to `~/.tAI/logs/error-<timestamp>.log`
- 🏷️ **Version tracking** – `TAI_VERSION` macro in `Config.h`, `--version` CLI flag
- 📜 History entries now include engine name and tAI version
