#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include "tAI/Config.h"
#include "tAI/DuckDuckGoClient.h"
#include "tAI/OllamaClient.h"
#include "tAI/OllamaCloudClient.h"
#include "tAI/HuggingfaceClient.h"
#include "tAI/GrokClient.h"
#include "tAI/OpenRouterClient.h"
#include "tAI/Utils.h"

// ─── openConfigInEditor ───────────────────────────────────────────────────────
static void openConfigInEditor(const std::string& path) {
    std::cout << "Opening config: " << path << std::endl;

#ifdef _WIN32
    // On Windows, use notepad (guaranteed to be available)
    std::string cmd = "notepad \"" + path + "\"";
#elif defined(__APPLE__)
    // On macOS, use 'open -t' to open with default text editor
    std::string cmd = "open -t \"" + path + "\"";
#else
    // On Linux/Unix: Try common text editors in order of preference
    // Prioritize text editors over xdg-open to avoid browser opening for .json files
    const char* editor = getenv("VISUAL");
    if (!editor || editor[0] == '\0') editor = getenv("EDITOR");
    std::string cmd;
    if (editor && editor[0] != '\0') {
        // Use user's preferred editor from environment variables
        cmd = std::string(editor) + " \"" + path + "\"";
    } else {
        // Try common text editors first, fall back to vi if none available
        cmd = "nano \"" + path + "\" 2>/dev/null || "
              "vim \"" + path + "\" 2>/dev/null || "
              "vi \"" + path + "\"";
    }
#endif

    int ret = system(cmd.c_str());
    (void)ret; // Return value intentionally unused for editor launch
}

// ─── stripShellMarkdown ───────────────────────────────────────────────────────
// Remove leading/trailing whitespace and backtick code fences from shell output
static std::string stripShellMarkdown(const std::string& s) {
    std::string result = s;

    // Trim leading whitespace
    size_t start = result.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    result = result.substr(start);

    // Trim trailing whitespace
    size_t end = result.find_last_not_of(" \t\n\r");
    if (end != std::string::npos)
        result = result.substr(0, end + 1);

    // Strip ```bash / ```sh / ``` fences
    if (result.rfind("```", 0) == 0) {
        size_t first_newline = result.find('\n');
        if (first_newline != std::string::npos)
            result = result.substr(first_newline + 1);
        // Strip trailing ```
        if (result.size() >= 3 && result.substr(result.size() - 3) == "```")
            result = result.substr(0, result.size() - 3);
        // Re-trim
        start = result.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        result = result.substr(start);
        end = result.find_last_not_of(" \t\n\r");
        if (end != std::string::npos)
            result = result.substr(0, end + 1);
    }

    return result;
}

// ─── printUsage ───────────────────────────────────────────────────────────────
void printUsage() {
    std::cout << "tAI v" << TAI_VERSION << " – Terminal AI Assistant\n\n"
              << "Usage: tAI [options] \"prompt\"\n\n"
              << "Options:\n"
              << "  -c                    Code mode (no explanations)\n"
              << "  -s                    Shell mode: AI generates a command, optionally executes it\n"
              << "  --system <prompt>     Custom system prompt\n"
              << "  -f <file>             File mode: include file content as context\n"
              << "  -d, --default         Set the default engine in config and exit\n"
              << "  -m <engine>           AI engine (see Supported Engines below)\n"
              << "  --api-key <key>       API key for the selected engine (overrides config)\n"
              << "  --config              Open config file in default text editor\n"
              << "  --config <path>       Use a custom config file path\n"
              << "  --version             Show version and exit\n"
              << "  -h, --help            Show this help message\n\n"
              << "Supported Engines:\n"
              << "  duckduckgo          DuckDuckGo Instant Answer (no API key, default)\n"
              << "  ollama              Local Ollama models on localhost:11434 (no API key)\n"
              << "  ollama_cloud        Ollama Cloud API (requires API key)\n"
              << "  huggingface         Hugging Face Inference API (requires API key)\n"
              << "  grok                xAI Grok API (requires API key)\n"
              << "  openrouter          OpenRouter aggregator (requires API key)\n\n"
              << "Examples:\n"
              << "  tAI \"What is machine learning?\"\n"
              << "  tAI -c \"Write a Python function to sort a list\"\n"
              << "  tAI -s \"list all .cpp files recursively\"\n"
              << "  tAI -f README.md \"Summarize this file\"\n"
              << "  tAI --system \"You are a Linux expert\" \"How do I install Docker?\"\n"
              << "  tAI --config\n"
              << "  tAI -m ollama \"What is 2+2?\"\n"
              << "  tAI -m ollama_cloud --api-key ollama-xxxx \"Explain quantum computing\"\n"
              << "  tAI -m huggingface --api-key hf_xxxx \"Tell me about AI\"\n"
              << "  tAI -m grok --api-key xai-xxxx \"What is the meaning of life?\"\n"
              << "  tAI -m openrouter --api-key sk-or-xxxx \"Summarize this article\"\n";
}

// ─── main ─────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    // Load configuration
    Config config;
    std::string configPath = config.getConfigPath();
    config.load(configPath);

    // Parse command line arguments
    std::string engine        = config.default_engine;
    std::string api_key;
    std::string system_prompt;
    std::string file_path;
    bool        code_mode     = false;
    bool        shell_mode    = false;
    bool        open_config   = false;
    std::string user_query;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;

        } else if (arg == "--version") {
            std::cout << "tAI v" << TAI_VERSION << "\n";
            return 0;

        } else if (arg == "-d" || arg == "--default") {
            if (i + 1 < argc) {
                std::string new_default = argv[++i];
                config.default_engine   = new_default;
                config.save(configPath);
                std::cout << "Default engine set to: " << new_default << std::endl;
                return 0;
            } else {
                std::string err = "Engine name required for flag: " + arg;
                std::cerr << "Error: " << err << "\n";
                logError("Argument parsing", err);
                return 1;
            }

        } else if (arg == "-c") {
            code_mode = true;

        } else if (arg == "-s") {
            // Shell mode — no argument consumed
            shell_mode = true;

        } else if (arg == "--system" && i + 1 < argc) {
            system_prompt = argv[++i];

        } else if (arg == "-f" && i + 1 < argc) {
            file_path = argv[++i];

        } else if (arg == "-m" && i + 1 < argc) {
            engine = argv[++i];

        } else if (arg == "--api-key" && i + 1 < argc) {
            api_key = argv[++i];

        } else if (arg == "--config") {
            // No next arg OR next arg starts with '-' → open config in editor
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                open_config = true;
            } else {
                // Next arg is a custom path
                configPath = argv[++i];
                config.load(configPath);
            }

        } else if (arg[0] != '-') {
            user_query = arg;

        } else {
            std::string err = "Unknown option: " + arg;
            std::cerr << err << "\n";
            logError("Argument parsing", err);
            printUsage();
            return 1;
        }
    }

    // ── Handle --config (open in editor) ──────────────────────────────────────
    if (open_config) {
        // Ensure config file exists before trying to open it
        std::ifstream check(configPath);
        if (!check.is_open()) {
            config.load(configPath); // creates default if missing
        }
        openConfigInEditor(configPath);
        return 0;
    }

    // ── Validate query ────────────────────────────────────────────────────────
    if (user_query.empty()) {
        std::string err = "No prompt provided.";
        std::cerr << "Error: " << err << "\n";
        logError("Argument parsing", err);
        printUsage();
        return 1;
    }

    // ── Validate engine ───────────────────────────────────────────────────────
    if (engine != "duckduckgo" && engine != "ollama"       &&
        engine != "ollama_cloud"                           &&
        engine != "huggingface"  && engine != "grok"       &&
        engine != "openrouter") {
        std::string err = "Unknown engine '" + engine + "'. "
                          "Valid: duckduckgo, ollama, ollama_cloud, huggingface, grok, openrouter";
        std::cerr << "Error: " << err << "\n";
        logError("Engine validation", err);
        return 1;
    }

    // ── Apply modes ───────────────────────────────────────────────────────────
    if (shell_mode) {
        system_prompt = "You are a shell command expert. "
                        "Respond with ONLY the raw shell command that accomplishes the user's "
                        "request. No explanations, no markdown, no backtick fences — "
                        "just the command itself on a single line.";
    } else if (code_mode && system_prompt.empty()) {
        system_prompt = "You are an expert programmer. "
                        "Provide only the code without any explanations or comments.";
    }

    // ── Handle -f (file mode) ─────────────────────────────────────────────────
    if (!file_path.empty()) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::string err = "Cannot open file: " + file_path;
            std::cerr << "Error: " << err << "\n";
            logError("File mode", err);
            return 1;
        }
        std::string file_content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
        file.close();

        // Build context-enhanced query
        std::string fname = std::filesystem::path(file_path).filename().string();
        user_query = "[File: " + fname + "]\n"
                   + file_content + "\n\n"
                   + "User query: " + user_query;
    }

    // ── Apply template ────────────────────────────────────────────────────────
    std::string template_format;
    if      (engine == "ollama")       template_format = config.ollama.template_format;
    else if (engine == "ollama_cloud") template_format = config.ollama_cloud.template_format;
    else if (engine == "huggingface")  template_format = config.huggingface.template_format;
    else if (engine == "grok")         template_format = config.grok.template_format;
    else if (engine == "openrouter")   template_format = config.openrouter.template_format;

    std::string original_query = user_query;
    if (!template_format.empty()) {
        size_t pos = template_format.find("{query}");
        if (pos != std::string::npos) {
            user_query = template_format;
            user_query.replace(pos, 7, original_query);
        } else {
            user_query = template_format + "\n" + original_query;
        }
    }

    // ── Resolve API key ───────────────────────────────────────────────────────
    if (api_key.empty()) {
        if      (engine == "ollama_cloud") api_key = config.ollama_cloud.api_key;
        else if (engine == "huggingface")  api_key = config.huggingface.api_key;
        else if (engine == "grok")         api_key = config.grok.api_key;
        else if (engine == "openrouter")   api_key = config.openrouter.api_key;
    }

    // ── Build client ──────────────────────────────────────────────────────────
    IApiClient* client = nullptr;

    if (engine == "duckduckgo") {
        client = new DuckDuckGoClient(config.duckduckgo.api_endpoint);

    } else if (engine == "ollama") {
        client = new OllamaClient();

    } else if (engine == "ollama_cloud") {
        if (api_key.empty()) {
            std::string err = "Ollama Cloud API key required. "
                              "Provide it via --api-key or set it in config.";
            std::cerr << "Error: " << err << "\n";
            logError("Client setup – ollama_cloud", err);
            return 1;
        }
        client = new OllamaCloudClient(api_key, config.ollama_cloud.model, config.ollama_cloud.api_endpoint);

    } else if (engine == "huggingface") {
        if (api_key.empty()) {
            std::string err = "Hugging Face API key required. "
                              "Provide it via --api-key or set it in config.";
            std::cerr << "Error: " << err << "\n";
            logError("Client setup – huggingface", err);
            return 1;
        }
        client = new HuggingfaceClient(api_key, config.huggingface.model,
                                       config.huggingface.api_endpoint);

    } else if (engine == "grok") {
        if (api_key.empty()) {
            std::string err = "Grok API key required. "
                              "Provide it via --api-key or set it in config.";
            std::cerr << "Error: " << err << "\n";
            logError("Client setup – grok", err);
            return 1;
        }
        client = new GrokClient(api_key, config.grok.model, config.grok.api_endpoint);

    } else if (engine == "openrouter") {
        if (api_key.empty()) {
            std::string err = "OpenRouter API key required. "
                              "Provide it via --api-key or set it in config.";
            std::cerr << "Error: " << err << "\n";
            logError("Client setup – openrouter", err);
            return 1;
        }
        client = new OpenRouterClient(api_key, config.openrouter.model, config.openrouter.api_endpoint,
                                      config.openrouter.referer);
    }

    if (!client) {
        std::string err = "Failed to create API client for engine: " + engine;
        std::cerr << "Error: " << err << "\n";
        logError("Client creation", err);
        return 1;
    }

    // ── Run query ─────────────────────────────────────────────────────────────
    try {
        std::string result = client->chat(user_query, system_prompt);

        if (shell_mode) {
            // ── Shell mode: show command, ask to execute ──────────────────────
            std::string command = stripShellMarkdown(result);
            std::cout << "\n  " << command << "\n\n";
            std::cout << "Execute this command? [y/N]: " << std::flush;

            std::string answer;
            std::getline(std::cin, answer);
            if (!answer.empty() && (answer[0] == 'y' || answer[0] == 'Y')) {
                std::cout << "\n";
                int ret = system(command.c_str());
                if (ret != 0) {
                    std::cout << "\n[Command exited with code " << ret << "]\n";
                }
            } else {
                std::cout << "Command not executed.\n";
            }
        } else {
            // Normal mode: typing effect
            typingPrint(result);
        }

        // ── Save history ──────────────────────────────────────────────────────
        try {
            auto now        = std::chrono::system_clock::now();
            auto in_time_t  = std::chrono::system_clock::to_time_t(now);

            std::stringstream date_ss;
            date_ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
            std::string date_str = date_ss.str();

            std::stringstream hour_ss;
            hour_ss << std::put_time(std::localtime(&in_time_t), "%H");
            std::string hour_str = hour_ss.str();

            std::stringstream full_time_ss;
            full_time_ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");

            std::string historyDir = getHomeDirectory() + "/.tAI/history/" + date_str;
            std::filesystem::create_directories(historyDir);

            std::string filename   = historyDir + "/" + hour_str + ".html";
            bool        file_exists = std::filesystem::exists(filename);

            std::ofstream out(filename, std::ios::app);
            if (out.is_open()) {
                if (!file_exists) {
                    out << "<!DOCTYPE html>\n<html>\n<head>\n<title>tAI History - "
                        << date_str << "</title>\n";
                    out << "<style>\n"
                           "body { font-family: sans-serif; margin: 2rem; }\n"
                           ".entry { border-bottom: 1px solid #ccc; padding-bottom: 1rem; margin-bottom: 1rem; }\n"
                           ".query { background: #f0f0f0; padding: 1rem; border-radius: 5px; }\n"
                           ".response { background: #e8f4f8; padding: 1rem; border-radius: 5px; margin-top: 1rem; }\n"
                           ".timestamp { color: #888; font-size: 0.9em; font-weight: bold; margin-bottom: 0.5rem; }\n"
                           "</style>\n</head>\n<body>\n";
                }

                out << "<div class=\"entry\">\n";
                out << "<div class=\"timestamp\">[" << full_time_ss.str()
                    << "] Engine: " << engine
                    << " | tAI v" << TAI_VERSION << "</div>\n";
                out << "<h2>Query:</h2>\n<div class=\"query\"><pre>"
                    << original_query << "</pre></div>\n";
                out << "<h2>Response:</h2>\n<div class=\"response\"><pre>"
                    << result << "</pre></div>\n";
                out << "</div>\n";
                out.close();
            } else {
                std::string warn = "Failed to open history file: " + filename;
                std::cerr << "Warning: " << warn << "\n";
                logError("History save", warn);
            }
        } catch (const std::exception& e) {
            std::string warn = std::string("History save failed: ") + e.what();
            std::cerr << "Warning: " << warn << "\n";
            logError("History save", warn);
        }

    } catch (const std::exception& e) {
        std::string err = std::string(e.what());
        std::cerr << "Error during API call: " << err << "\n";
        std::cerr << "Error details saved to: "
                  << getHomeDirectory() << "/.tAI/logs/\n";
        logError("API call – engine: " + engine, err);
        delete client;
        return 1;
    }

    delete client;
    return 0;
}