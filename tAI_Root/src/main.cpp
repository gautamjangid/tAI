#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include "tAI/Config.h"
#include "tAI/OllamaClient.h"
#include "tAI/OllamaCloudClient.h"
#include "tAI/HuggingfaceClient.h"
#include "tAI/GrokClient.h"
#include "tAI/OpenRouterClient.h"
#include "tAI/Utils.h"

void printUsage() {
    std::cout << "Usage: tAI [options] \"prompt\"\n\n"
              << "Options:\n"
              << "  -c                  Code mode (no explanations)\n"
              << "  -d, --default       Set the default engine in config and exit\n"
              << "  -s <system>         Custom system prompt\n"
              << "  -m <engine>         AI engine (see Supported Engines below)\n"
              << "  --api-key <key>     API key for the selected engine (overrides config)\n"
              << "  --config <path>     Config file path\n"
              << "  -h, --help          Show this help message\n\n"
              << "Supported Engines:\n"
              << "  ollama              Local Ollama models on localhost:11434 (no API key)\n"
              << "  ollama_cloud        Ollama Cloud API (requires API key)\n"
              << "  huggingface         Hugging Face Inference API (requires API key)\n"
              << "  grok                xAI Grok API (requires API key)\n"
              << "  openrouter          OpenRouter aggregator (requires API key)\n\n"
              << "Examples:\n"
              << "  tAI \"What is machine learning?\"\n"
              << "  tAI -c \"Write a Python function to sort a list\"\n"
              << "  tAI -m ollama \"What is 2+2?\"\n"
              << "  tAI -m ollama_cloud --api-key ollama-xxxx \"Explain quantum computing\"\n"
              << "  tAI -m huggingface --api-key hf_xxxx \"Tell me about AI\"\n"
              << "  tAI -m grok --api-key xai-xxxx \"What is the meaning of life?\"\n"
              << "  tAI -m openrouter --api-key sk-or-xxxx \"Summarize this article\"\n";
}

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
    std::string engine = config.default_engine; // default from config (should be "ollama")
    std::string api_key;                      // may be overridden by CLI
    std::string system_prompt;
    bool code_mode = false;
    std::string user_query;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "-d" || arg == "--default") {
            if (i + 1 < argc) {
                std::string new_default = argv[++i];
                config.default_engine = new_default;
                config.save(configPath);
                std::cout << "Default engine set to: " << new_default << std::endl;
                return 0;
            } else {
                std::cerr << "Error: Engine name required for " << arg << "\n";
                return 1;
            }
        } else if (arg == "-c") {
            code_mode = true;
        } else if (arg == "-s" && i + 1 < argc) {
            system_prompt = argv[++i];
        } else if (arg == "-m" && i + 1 < argc) {
            engine = argv[++i];
        } else if (arg == "--api-key" && i + 1 < argc) {
            api_key = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
            config.load(configPath);
        } else if (arg[0] != '-') {
            user_query = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage();
            return 1;
        }
    }

    if (user_query.empty()) {
        std::cerr << "Error: No prompt provided.\n";
        printUsage();
        return 1;
    }

    // Validate engine choice
    if (engine != "ollama" && engine != "ollama_cloud" &&
        engine != "huggingface" && engine != "grok" && engine != "openrouter") {
        std::cerr << "Error: Unknown engine '" << engine << "'\n";
        std::cerr << "Valid engines: ollama, ollama_cloud, huggingface, grok, openrouter\n";
        return 1;
    }

    if (code_mode && system_prompt.empty()) {
        system_prompt = "You are an expert programmer. Provide only the code without any explanations or comments.";
    }

    // Apply template if configured
    std::string template_format = "";
    if (engine == "ollama") template_format = config.ollama.template_format;
    else if (engine == "ollama_cloud") template_format = config.ollama_cloud.template_format;
    else if (engine == "huggingface") template_format = config.huggingface.template_format;
    else if (engine == "grok") template_format = config.grok.template_format;
    else if (engine == "openrouter") template_format = config.openrouter.template_format;

    std::string original_query = user_query; // Keep original for history
    if (!template_format.empty()) {
        size_t pos = template_format.find("{query}");
        if (pos != std::string::npos) {
            user_query = template_format;
            user_query.replace(pos, 7, original_query);
        } else {
            // Fallback if {query} is not in the template
            user_query = template_format + "\n" + original_query;
        }
    }

    // Determine which API key to use if not provided on command line
    if (api_key.empty()) {
        if (engine == "ollama_cloud") {
            api_key = config.ollama_cloud.api_key;
        } else if (engine == "huggingface") {
            api_key = config.huggingface.api_key;
        } else if (engine == "grok") {
            api_key = config.grok.api_key;
        } else if (engine == "openrouter") {
            api_key = config.openrouter.api_key;
        }
        // ollama (local) does not need an API key
    }

    IApiClient* client = nullptr;
    if (engine == "ollama") {
        client = new OllamaClient();
    } else if (engine == "ollama_cloud") {
        if (api_key.empty()) {
            std::cerr << "Error: Ollama Cloud API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config.\n";
            return 1;
        }
        client = new OllamaCloudClient(api_key, config.ollama_cloud.api_endpoint);
    } else if (engine == "huggingface") {
        if (api_key.empty()) {
            std::cerr << "Error: Hugging Face API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config.\n";
            return 1;
        }
        client = new HuggingfaceClient(api_key, config.huggingface.model, config.huggingface.api_endpoint);
    } else if (engine == "grok") {
        if (api_key.empty()) {
            std::cerr << "Error: Grok API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config.\n";
            return 1;
        }
        client = new GrokClient(api_key, config.grok.api_endpoint);
    } else if (engine == "openrouter") {
        if (api_key.empty()) {
            std::cerr << "Error: OpenRouter API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config.\n";
            return 1;
        }
        client = new OpenRouterClient(api_key, config.openrouter.api_endpoint, config.openrouter.referer);
    }

    if (!client) {
        std::cerr << "Error: Failed to create API client.\n";
        return 1;
    }

    try {
        std::string result = client->chat(user_query, system_prompt);
        std::cout << result << std::endl;

        // Save history
        try {
            std::string historyDir = getHomeDirectory() + "/.tAI/history";
            std::filesystem::create_directories(historyDir);

            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");

            std::string filename = historyDir + "/" + engine + "_" + ss.str() + ".html";
            std::ofstream out(filename);
            if (out.is_open()) {
                out << "<!DOCTYPE html>\n<html>\n<head>\n<title>tAI History</title>\n";
                out << "<style>\nbody { font-family: sans-serif; margin: 2rem; }\n";
                out << ".query { background: #f0f0f0; padding: 1rem; border-radius: 5px; }\n";
                out << ".response { background: #e8f4f8; padding: 1rem; border-radius: 5px; margin-top: 1rem; }\n";
                out << "</style>\n</head>\n<body>\n";
                out << "<h2>Query:</h2>\n<div class=\"query\"><pre>" << original_query << "</pre></div>\n";
                out << "<h2>Response:</h2>\n<div class=\"response\"><pre>" << result << "</pre></div>\n";
                out << "</body>\n</html>\n";
                out.close();
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to save history: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during API call: " << e.what() << "\n";
        delete client;
        return 1;
    }

    delete client;
    return 0;
}