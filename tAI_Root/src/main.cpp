#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "tAI/Config.h"
#include "tAI/OllamaClient.h"
#include "tAI/OllamaCloudClient.h"
#include "tAI/HuggingfaceClient.h"
#include "tAI/GrokClient.h"
#include "tAI/OpenRouterClient.h"
#include "tAI/Utils.h"

// ─── printUsage ───────────────────────────────────────────────────────────────
void printUsage() {
    std::cout << "tAI v" << TAI_VERSION << " – Terminal AI Assistant\n\n"
              << "Usage: tAI [options] \"prompt\"\n\n"
              << "Options:\n"
              << "  -c                  Code mode (no explanations)\n"
              << "  -d, --default       Set the default engine in config and exit\n"
              << "  -s <system>         Custom system prompt\n"
              << "  -m <engine>         AI engine (see Supported Engines below)\n"
              << "  --api-key <key>     API key for the selected engine (overrides config)\n"
              << "  --config <path>     Config file path\n"
              << "  --version           Show version and exit\n"
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
    std::string engine      = config.default_engine;
    std::string api_key;
    std::string system_prompt;
    bool        code_mode   = false;
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
            std::string err = "Unknown option: " + arg;
            std::cerr << err << "\n";
            logError("Argument parsing", err);
            printUsage();
            return 1;
        }
    }

    if (user_query.empty()) {
        std::string err = "No prompt provided.";
        std::cerr << "Error: " << err << "\n";
        logError("Argument parsing", err);
        printUsage();
        return 1;
    }

    // Validate engine choice
    if (engine != "ollama"       && engine != "ollama_cloud" &&
        engine != "huggingface"  && engine != "grok"         &&
        engine != "openrouter") {
        std::string err = "Unknown engine '" + engine + "'. "
                          "Valid engines: ollama, ollama_cloud, huggingface, grok, openrouter";
        std::cerr << "Error: " << err << "\n";
        logError("Engine validation", err);
        return 1;
    }

    if (code_mode && system_prompt.empty()) {
        system_prompt = "You are an expert programmer. "
                        "Provide only the code without any explanations or comments.";
    }

    // Apply template if configured
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

    // Resolve API key
    if (api_key.empty()) {
        if      (engine == "ollama_cloud") api_key = config.ollama_cloud.api_key;
        else if (engine == "huggingface")  api_key = config.huggingface.api_key;
        else if (engine == "grok")         api_key = config.grok.api_key;
        else if (engine == "openrouter")   api_key = config.openrouter.api_key;
    }

    // Build client
    IApiClient* client = nullptr;
    if (engine == "ollama") {
        client = new OllamaClient();

    } else if (engine == "ollama_cloud") {
        if (api_key.empty()) {
            std::string err = "Ollama Cloud API key required. "
                              "Provide it via --api-key or set it in config.";
            std::cerr << "Error: " << err << "\n";
            logError("Client setup – ollama_cloud", err);
            return 1;
        }
        client = new OllamaCloudClient(api_key, config.ollama_cloud.api_endpoint);

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
        client = new GrokClient(api_key, config.grok.api_endpoint);

    } else if (engine == "openrouter") {
        if (api_key.empty()) {
            std::string err = "OpenRouter API key required. "
                              "Provide it via --api-key or set it in config.";
            std::cerr << "Error: " << err << "\n";
            logError("Client setup – openrouter", err);
            return 1;
        }
        client = new OpenRouterClient(api_key, config.openrouter.api_endpoint,
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

        // Print with typing effect (avg pace: 18 ms/char)
        typingPrint(result);

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