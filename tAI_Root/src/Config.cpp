#include "tAI/Config.h"
#include "tAI/Utils.h"
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#endif

// Minimal JSON helper - fixed quote escaping
static std::string getJsonValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    
    // Find opening quote
    pos = json.find("\"", pos + search.length());
    if (pos == std::string::npos) return "";
    pos++; // skip opening quote
    
    // Find closing quote (handle escaped quotes)
    size_t end = pos;
    while (end < json.length()) {
        if (json[end] == '"' && (end == 0 || json[end-1] != '\\')) {
            break;
        }
        end++;
    }
    
    if (end == json.length()) return "";
    
    return json.substr(pos, end - pos);
}

// Ensure config directory exists
static void ensureDirectoryExists(const std::string& path) {
    size_t pos = 0;
#ifdef _WIN32
    pos = path.find("\\");
    while (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        if (!dir.empty()) {
            _mkdir(dir.c_str());
        }
        pos = path.find("\\", pos + 1);
    }
#else
    pos = path.find("/");
    while (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        if (!dir.empty()) {
            mkdir(dir.c_str(), 0755);
        }
        pos = path.find("/", pos + 1);
    }
#endif
}

Config::Config() 
    : openai_key(""),
      ollama_cloud_key(""),
      ollama_cloud_endpoint("https://api.ollama.cloud"),
      huggingface_key(""),
      grok_key(""),
      openrouter_key(""),
      default_engine("openai"),
      huggingface_model("meta-llama/Llama-2-7b-chat-hf"),
      openrouter_referer("")
{}

void Config::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    // Load all API keys
    openai_key = getJsonValue(content, "openai_key");
    ollama_cloud_key = getJsonValue(content, "ollama_cloud_key");
    ollama_cloud_endpoint = getJsonValue(content, "ollama_cloud_endpoint");
    huggingface_key = getJsonValue(content, "huggingface_key");
    grok_key = getJsonValue(content, "grok_key");
    openrouter_key = getJsonValue(content, "openrouter_key");
    
    // Load engine settings
    default_engine = getJsonValue(content, "default_engine");
    if (default_engine.empty()) default_engine = "openai";
    
    std::string hf_model = getJsonValue(content, "huggingface_model");
    if (!hf_model.empty()) {
        huggingface_model = hf_model;
    }
    
    std::string endpoint = getJsonValue(content, "ollama_cloud_endpoint");
    if (!endpoint.empty()) {
        ollama_cloud_endpoint = endpoint;
    }
    
    openrouter_referer = getJsonValue(content, "openrouter_referer");
}

void Config::save(const std::string& path) const {
    // Ensure directory exists
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        std::string dir = path.substr(0, lastSlash);
        ensureDirectoryExists(dir);
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file for writing: " << path << std::endl;
        return;
    }
    
    file << "{\n";
    file << "  \"default_engine\": \"" << default_engine << "\",\n";
    file << "  \"openai_key\": \"" << openai_key << "\",\n";
    file << "  \"ollama_cloud_key\": \"" << ollama_cloud_key << "\",\n";
    file << "  \"ollama_cloud_endpoint\": \"" << ollama_cloud_endpoint << "\",\n";
    file << "  \"huggingface_key\": \"" << huggingface_key << "\",\n";
    file << "  \"huggingface_model\": \"" << huggingface_model << "\",\n";
    file << "  \"grok_key\": \"" << grok_key << "\",\n";
    file << "  \"openrouter_key\": \"" << openrouter_key << "\",\n";
    file << "  \"openrouter_referer\": \"" << openrouter_referer << "\"\n";
    file << "}\n";
    
    file.close();
}

std::string Config::getConfigPath() const {
#ifdef _WIN32
    return getHomeDirectory() + "\\.tAI\\config.json";
#else
    return getHomeDirectory() + "/.tAI/config.json";
#endif
}