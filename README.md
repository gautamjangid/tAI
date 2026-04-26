# tAI – Terminal AI Assistant

**tAI** is a lightweight, cross-platform CLI AI assistant written in C++.
It works entirely in the terminal, supports multiple AI backends, and runs well even on legacy hardware.

## ✨ Features

- **Terminal-only** – no GUI dependencies, minimal resource usage
- **Multi-backend support**:
  - **OpenAI** – GPT-3.5/4, bring your own API key
  - **Ollama** – local models at `localhost:11434`
  - **Ollama Cloud** – cloud-hosted Ollama models via API
  - **Hugging Face** – Inference API with 200K+ models
  - **xAI Grok** – Elon Musk's reasoning AI
  - **OpenRouter** – model aggregator with 100+ models
- **Flexible CLI flags**:
  - `-c` – code generation mode (no explanations)
  - `-s "system"` – set a custom system prompt
  - `-m <engine>` – choose which AI backend to use
- **Config file** – save API keys and defaults in `~/.tAI/config.json`
- **Platform independent** – compiles and runs on Linux, macOS, and Windows (via CMake + libcurl)

## 🚀 Quick Start

After building (see below), simply run:

```bash
# OpenAI (requires API key)
tAI -m openai --api-key sk-xxxx "Explain quantum computing"

# Local Ollama (no key needed, requires running Ollama server)
tAI -m ollama "What is the capital of France?"

# Ollama Cloud (requires API key)
tAI -m ollama_cloud --api-key ollama-xxxx "Write a poem about the sea"

# Hugging Face (requires API key)
tAI -m huggingface --api-key hf_xxxx "What is machine learning?"

# Grok (requires API key)
tAI -m grok --api-key xai-xxxx "What is consciousness?"

# OpenRouter (requires API key, can use any model on their platform)
tAI -m openrouter --api-key sk-or-xxxx "Summarize the history of AI"

# Default engine (configured in config.json)
tAI "Hello, how are you?"
```

## ⚙️ Configuration

On first run, tAI creates `~/.tAI/config.json` (or `%USERPROFILE%\.tAI\config.json` on Windows).
Edit it to set your preferences:

```json
{
  "default_engine": "openai",
  "openai_key": "sk-your-openai-key",
  "ollama_cloud_key": "ollama-your-cloud-key",
  "ollama_cloud_endpoint": "https://api.ollama.cloud",
  "huggingface_key": "hf_your-huggingface-key",
  "huggingface_model": "meta-llama/Llama-2-7b-chat-hf",
  "grok_key": "xai-your-grok-key",
  "openrouter_key": "sk-or-your-openrouter-key",
  "openrouter_referer": "https://myapp.com"
}
```

**Notes:**
- `default_engine`: Which engine to use if `-m` is not specified
- `huggingface_model`: Default HF model (can be overridden per-call; see [Hugging Face Models](https://huggingface.co/models?pipeline_tag=text-generation&sort=trending))
- `openrouter_referer`: Optional, for tracking your usage in OpenRouter dashboard
- You can also override the config path with `--config /path/to/config.json`

## 🛠️ Build from Source

Requirements: CMake ≥ 3.14, C++17 compiler, libcurl (with headers).

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

# Then configure with CMake:
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## 📦 Install

### Linux/macOS (system-wide)

```bash
sudo bash scripts/install.sh   # copies tAI to /usr/local/bin
```

### Uninstall

```bash
sudo bash scripts/uninstall.sh  # removes /usr/local/bin/tAI and the man page
```

### Windows (vcpkg)

Manually copy `build/Release/tAI.exe` to a folder in your PATH, or run the provided batch files (admin).

## 🧪 Supported Backends

| Engine | Flag | API Key Required | Setup |
|--------|------|------------------|-------|
| OpenAI | `-m openai` | Yes (GPT-3.5/4) | Get key from [openai.com](https://platform.openai.com/account/api-keys) |
| Ollama (Local) | `-m ollama` | No | Run `ollama serve` locally on port 11434 |
| Ollama Cloud | `-m ollama_cloud` | Yes | Get key from Ollama Cloud dashboard |
| Hugging Face | `-m huggingface` | Yes | Get key from [huggingface.co](https://huggingface.co/settings/tokens) |
| xAI Grok | `-m grok` | Yes | Get key from [x.ai](https://x.ai) |
| OpenRouter | `-m openrouter` | Yes | Get key from [openrouter.ai](https://openrouter.ai) |

### Getting API Keys

#### **OpenAI**
1. Sign up at https://platform.openai.com
2. Go to API keys: https://platform.openai.com/account/api-keys
3. Create a new API key and copy it
4. Set it in config or use `--api-key sk-...`

#### **Ollama Cloud**
1. Visit https://ollama.cloud (when available)
2. Create account and generate API key
3. Set in config: `ollama_cloud_key`

#### **Hugging Face**
1. Sign up at https://huggingface.co
2. Go to Settings → Access Tokens
3. Create a new token with "Read" access
4. Set in config: `huggingface_key`
5. Optionally customize `huggingface_model` (default: `meta-llama/Llama-2-7b-chat-hf`)

#### **xAI Grok**
1. Visit https://x.ai
2. Follow their setup process to get API credentials
3. Set in config: `grok_key`

#### **OpenRouter**
1. Sign up at https://openrouter.ai
2. Go to Keys section
3. Create API key
4. Set in config: `openrouter_key`
5. Optionally set `openrouter_referer` for tracking

## 📋 Usage Examples

### Code Mode (Generate code only)
```bash
tAI -c "Write a Python function to validate email addresses"
```

### Custom System Prompt
```bash
tAI -s "You are a financial advisor" "Should I invest in tech stocks?"
```

### Using Specific Engines
```bash
# OpenAI with GPT-4
tAI -m openai --api-key sk-xxxx "Explain relativity like I'm 5"

# Hugging Face with a specific model
tAI -m huggingface --api-key hf_xxxx "Tell me a joke"

# Local Ollama (no key needed)
tAI -m ollama -s "You are a philosopher" "What is happiness?"

# OpenRouter (can auto-select best model)
tAI -m openrouter --api-key sk-or-xxxx "Write a haiku about mountains"
```

## 🔒 Privacy & Security

- **No Data Collection**: tAI only sends your prompts to the backend you choose. No telemetry or logging.
- **API Key Safety**: Store API keys in `~/.tAI/config.json` and keep it private (`chmod 600 ~/.tAI/config.json` on Linux/macOS)
- **Backend Privacy**: Each service has their own privacy policy:
  - [OpenAI Privacy Policy](https://openai.com/privacy/)
  - [Hugging Face Privacy Policy](https://huggingface.co/privacy)
  - [OpenRouter Privacy Policy](https://openrouter.ai/privacy)
  - [xAI Grok](https://x.ai)

## 🗂️ Project Structure

```
tAI/
├── README.md
├── tAI_Root/
│   ├── CMakeLists.txt
│   ├── include/tAI/           (public headers)
│   ├── src/                   (implementation files)
│   ├── scripts/               (install/uninstall helpers)
│   └── docs/                  (design notes)
└── docs/                      (documentation)
```

## 🤝 Contributing

Pull requests are welcome! If you add a new backend:
1. Create a new class inheriting from `IApiClient` in `include/tAI/`
2. Implement the `chat()` method
3. Add source file in `src/`
4. Update `CMakeLists.txt` to include the new files
5. Update `main.cpp` to wire it into the CLI
6. Test thoroughly with valid API keys

## 📄 License

This project is open-source under the MIT License.