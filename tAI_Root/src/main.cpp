#include <iostream>
#include <string>
#include <cstring>
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

    // Validate that we have a prompt
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

    // Set code mode system prompt if needed
    if (code_mode && system_prompt.empty()) {
        system_prompt = "You are an expert programmer. Provide only the code without any explanations or comments.";
    }

    // Resolve API key from config if not supplied on CLI
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

    // Create appropriate client
    IApiClient* client = nullptr;

    if (engine == "ollama") {
        client = new OllamaClient();
    } else if (engine == "ollama_cloud") {
        if (api_key.empty()) {
            std::cerr << "Error: Ollama Cloud API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config under ollama_cloud.api_key.\n";
            return 1;
        }
        client = new OllamaCloudClient(api_key, config.ollama_cloud.api_endpoint);
    } else if (engine == "huggingface") {
        if (api_key.empty()) {
            std::cerr << "Error: Hugging Face API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config under huggingface.api_key.\n";
            return 1;
        }
        client = new HuggingfaceClient(api_key, config.huggingface.model);
    } else if (engine == "grok") {
        if (api_key.empty()) {
            std::cerr << "Error: xAI Grok API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config under grok.api_key.\n";
            return 1;
        }
        client = new GrokClient(api_key);
    } else if (engine == "openrouter") {
        if (api_key.empty()) {
            std::cerr << "Error: OpenRouter API key required.\n";
            std::cerr << "Provide it via --api-key or set it in config under openrouter.api_key.\n";
            return 1;
        }
        client = new OpenRouterClient(api_key, config.openrouter.referer);
    }

    if (!client) {
        std::cerr << "Error: Failed to create API client.\n";
        return 1;
    }

    // Perform the API call
    try {
        std::string result = client->chat(user_query, system_prompt);
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during API call: " << e.what() << "\n";
        delete client;
        return 1;
    }

    delete client;
    return 0;
}