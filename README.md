# tAI – Terminal AI Assistant

**tAI** is a lightweight, cross-platform CLI AI assistant written in C++.
It works entirely in the terminal, supports multiple free or free‑tier backends, and runs well even on legacy hardware.

## ✨ Features

- **Terminal‑only** – no GUI dependencies, minimal resource usage
- **Multi‑backend support** (all free or free‑tier):
  - **Ollama (local)** – run models on `localhost:11434` (no API key required)
  - **Ollama Cloud** – cloud‑hosted Ollama models (requires API key)
  - **Hugging Face** – access 200K+ models via the Inference API (free tier available)
  - **xAI Grok** – advanced reasoning (free tier available)
  - **OpenRouter** – aggregate access to many models (free tier available)
- **tgpt‑like flags**:
  - `-c` – code generation mode (no explanations)
  - `-s "system"` – set a custom system prompt
  - `-d "engine"` / `--default "engine"` – easily set the default engine in your config file
- **History saving** – automatically saves queries and responses as stylish `.html` files in `~/.tAI/history/YYYY-MM-DD/HH.html` (day-wise folders with hourly appends).
- **Config file** – save API keys, templates, and defaults in `~/.tAI/config.json`
- **Platform independent** – compiles and runs on Linux, macOS, and Windows (via CMake + libcurl + nlohmann_json)
- **Privacy‑respecting** – direct API calls, no anonymous proxies

## 🚀 Quick Start

After building (see below), simply run:

```bash
# Use local Ollama (no key needed)
tAI "Explain quantum computing in simple terms"

# Ollama Cloud (requires API key)
tAI -m ollama_cloud --api-key ollama-xxxx "Write a poem about the sea"

# Hugging Face (requires API key, free tier)
tAI -m huggingface --api-key hf_xxxx "Tell me about AI"

# xAI Grok (requires API key, free tier)
tAI -m grok --api-key xai-xxxx "What is consciousness?"

# OpenRouter (requires API key, free tier)
tAI -m openrouter --api-key sk-or-xxxx "Summarize the history of AI"

# Code generation mode
tAI -c "Write a Python function to sort a list"

# Custom system prompt
tAI -s "You are a Linux expert" "How do I install Docker?"
```

## ⚙️ Configuration

On first run, tAI creates `~/.tAI/config.json` (or `%USERPROFILE%\.tAI\config.json` on Windows).
Edit it to set your preferences:

```json
{
  "default_engine": "ollama",
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
    "api_endpoint": "https://api-inference.huggingface.co",
    "api_key": "",
    "model": "meta-llama/Llama-2-7b-chat-hf",
    "template": "",
    "enabled": false
  },
  "grok": {
    "api_endpoint": "https://api.x.ai/v1",
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

- `default_engine`: Engine used when `-m` is not supplied (default: `ollama`). You can easily change this from the CLI via `tAI -d <engine>`.
- `template`: Optional string to wrap user queries for that specific engine. E.g., setting `"template": "Summarize this: {query}"` will automatically insert the user's prompt wherever `{query}` is placed!
- Each engine section contains its endpoint, optional API key, and an `enabled` flag.
- Fill in the appropriate `api_key` for any cloud engine you wish to use.

You can also override the config path with `--config /path/to/config.json`.

## 🛠️ Build from Source

Requirements: CMake ≥ 3.14, C++17 compiler, libcurl (with headers). `nlohmann/json` is fetched automatically via CMake.

### Linux / macOS

```bash
# Install dependencies (Debian/Ubuntu):
sudo apt install cmake g++ libcurl4-openssl-dev nlohmann-json3-dev

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

## 📦 Install

### Linux/macOS (One-Command Install)

You can install `tAI` without manually cloning the repository using this single command:

```bash
curl -sSL https://raw.githubusercontent.com/gautamjangid/tAI/main/tAI_Root/setup.sh | sudo bash -s -- -i
```

This script will automatically clone the latest version into a temporary directory, build it, and install it system-wide.

*(Alternatively, you can manually clone the repo, `cd tAI_Root`, and run `sudo bash setup.sh -i`)*

### Uninstall

You can easily uninstall `tAI` by passing the `-u` flag to the setup script:

```bash
curl -sSL https://raw.githubusercontent.com/gautamjangid/tAI/main/tAI_Root/setup.sh | sudo bash -s -- -u
```

*(Or locally: `sudo bash setup.sh -u`)*

### Windows (vcpkg)

Manually copy `build/Release/tAI.exe` to a folder in your PATH, or run the provided batch files (admin).

## 🧪 Supported Backends

| Engine          | Flag            | Requires API Key? |
|-----------------|-----------------|-------------------|
| Ollama (local)  | `-m ollama`     | No                |
| Ollama Cloud    | `-m ollama_cloud` | Yes               |
| Hugging Face    | `-m huggingface` | Yes (free tier)   |
| xAI Grok        | `-m grok`        | Yes (free tier)   |
| OpenRouter      | `-m openrouter`  | Yes (free tier)   |

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

## 🗂️ Project Structure

```
tAI_Root/
├── CMakeLists.txt
├── README.md
├── scripts/         (install/uninstall helpers)
├── include/tAI/     (public headers)
│   ├── Config.h
│   ├── IApiClient.h
│   ├── OllamaClient.h
│   ├── OllamaCloudClient.h
│   ├── HuggingfaceClient.h
│   ├── GrokClient.h
│   ├── OpenRouterClient.h
│   └── Utils.h
├── src/             (implementation files)
│   ├── main.cpp
│   ├── Config.cpp
│   ├── OllamaClient.cpp
│   ├── OllamaCloudClient.cpp
│   ├── HuggingfaceClient.cpp
│   ├── GrokClient.cpp
│   ├── OpenRouterClient.cpp
│   └── Utils.cpp
└── docs/            (design notes)
```

## 🤝 Contributing

Pull requests are welcome. If you add a new backend, please:

1. Derive from `IApiClient` and implement `chat()`.
2. Add the new client to `main.cpp` dispatcher.
3. Update `Config.h`/`Config.cpp` with a nested config section.
4. Add the new files to `CMakeLists.txt`.
5. Document the engine in this README.

## 📄 License

This project is open-source under the MIT License.