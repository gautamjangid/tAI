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

// Constructor - Initialize with defaults
Config::Config() 
    : default_engine("ollama")
{
    // All nested structs initialize with their defaults in header
}

// Parse string value from JSON section
std::string Config::parseJsonString(const std::string& json, 
                                   const std::string& key,
                                   const std::string& defaultValue) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return defaultValue;
    
    // Find opening quote
    pos = json.find("\"", pos + search.length());
    if (pos == std::string::npos) return defaultValue;
    pos++; // skip opening quote
    
    // Find closing quote (handle escaped quotes)
    size_t end = pos;
    while (end < json.length()) {
        if (json[end] == '"' && (end == 0 || json[end-1] != '\\')) {
            break;
        }
        end++;
    }
    
    if (end == json.length()) return defaultValue;
    
    return json.substr(pos, end - pos);
}

// Parse boolean value from JSON section
bool Config::parseJsonBool(const std::string& json,
                          const std::string& key,
                          bool defaultValue) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return defaultValue;
    
    pos += search.length();
    
    // Skip whitespace
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) {
        pos++;
    }
    
    if (pos >= json.length()) return defaultValue;
    
    if (json.substr(pos, 4) == "true") return true;
    if (json.substr(pos, 5) == "false") return false;
    
    return defaultValue;
}

// Escape string for JSON
std::string Config::jsonEscape(const std::string& input) const {
    std::string result;
    for (char c : input) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            default:
                if (static_cast<unsigned char>(c) < 32) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
        }
    }
    return result;
}

// Ensure directory path exists
void Config::ensureDirectoryExists(const std::string& path) const {
    size_t pos = 0;
#ifdef _WIN32
    pos = path.find("\\");
    while (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        if (!dir.empty() && dir != "\\") {
            _mkdir(dir.c_str());
        }
        pos = path.find("\\", pos + 1);
    }
#else
    pos = path.find("/");
    while (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        if (!dir.empty() && dir != "/") {
            mkdir(dir.c_str(), 0755);
        }
        pos = path.find("/", pos + 1);
    }
#endif
}

// Get default config as JSON string
std::string Config::getDefaultConfigJson() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"default_engine\": \"ollama\",\n";
    ss << "  \"ollama\": {\n";
    ss << "    \"api_endpoint\": \"http://localhost:11434\",\n";
    ss << "    \"template\": \"\",\n";
    ss << "    \"enabled\": true\n";
    ss << "  },\n";
    ss << "  \"huggingface\": {\n";
    ss << "    \"api_endpoint\": \"https://api-inference.huggingface.co\",\n";
    ss << "    \"api_key\": \"\",\n";
    ss << "    \"model\": \"meta-llama/Llama-2-7b-chat-hf\",\n";
    ss << "    \"template\": \"\",\n";
    ss << "    \"enabled\": false\n";
    ss << "  },\n";
    ss << "  \"ollama_cloud\": {\n";
    ss << "    \"api_endpoint\": \"https://api.ollama.cloud\",\n";
    ss << "    \"api_key\": \"\",\n";
    ss << "    \"template\": \"\",\n";
    ss << "    \"enabled\": false\n";
    ss << "  },\n";
    ss << "  \"grok\": {\n";
    ss << "    \"api_endpoint\": \"https://api.x.ai/v1\",\n";
    ss << "    \"api_key\": \"\",\n";
    ss << "    \"template\": \"\",\n";
    ss << "    \"enabled\": false\n";
    ss << "  },\n";
    ss << "  \"openrouter\": {\n";
    ss << "    \"api_endpoint\": \"https://openrouter.ai/api/v1\",\n";
    ss << "    \"api_key\": \"\",\n";
    ss << "    \"referer\": \"\",\n";
    ss << "    \"template\": \"\",\n";
    ss << "    \"enabled\": false\n";
    ss << "  }\n";
    ss << "}\n";
    return ss.str();
}

// Load configuration from file
void Config::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        // File doesn't exist, create default
        std::cout << "Config file not found. Creating default at: " << path << std::endl;
        createDefaultConfig(path);
        return;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();

    // Parse default engine
    std::string engine = parseJsonString(content, "default_engine", "ollama");
    if (!engine.empty()) {
        default_engine = engine;
    }

    // === Parse Ollama (Local) ===
    size_t ollama_pos = content.find("\"ollama\":");
    if (ollama_pos != std::string::npos) {
        size_t ollama_end = content.find("}", ollama_pos);
        if (ollama_end != std::string::npos) {
            std::string section = content.substr(ollama_pos, ollama_end - ollama_pos + 1);
            
            std::string endpoint = parseJsonString(section, "api_endpoint", "http://localhost:11434");
            if (!endpoint.empty()) {
                ollama.api_endpoint = endpoint;
            }
            
            std::string tpl = parseJsonString(section, "template", "");
            if (!tpl.empty()) ollama.template_format = tpl;
            
            ollama.enabled = parseJsonBool(section, "enabled", true);
        }
    }

    // === Parse Hugging Face ===
    size_t hf_pos = content.find("\"huggingface\":");
    if (hf_pos != std::string::npos) {
        size_t hf_end = content.find("}", hf_pos);
        if (hf_end != std::string::npos) {
            std::string section = content.substr(hf_pos, hf_end - hf_pos + 1);
            
            std::string endpoint = parseJsonString(section, "api_endpoint", "https://api-inference.huggingface.co");
            if (!endpoint.empty()) {
                huggingface.api_endpoint = endpoint;
            }
            
            std::string key = parseJsonString(section, "api_key", "");
            if (!key.empty()) {
                huggingface.api_key = key;
            }
            
            std::string model = parseJsonString(section, "model", "meta-llama/Llama-2-7b-chat-hf");
            if (!model.empty()) {
                huggingface.model = model;
            }
            
            std::string tpl = parseJsonString(section, "template", "");
            if (!tpl.empty()) huggingface.template_format = tpl;
            
            huggingface.enabled = parseJsonBool(section, "enabled", false);
        }
    }

    // === Parse Ollama Cloud ===
    size_t oc_pos = content.find("\"ollama_cloud\":");
    if (oc_pos != std::string::npos) {
        size_t oc_end = content.find("}", oc_pos);
        if (oc_end != std::string::npos) {
            std::string section = content.substr(oc_pos, oc_end - oc_pos + 1);
            
            std::string endpoint = parseJsonString(section, "api_endpoint", "https://api.ollama.cloud");
            if (!endpoint.empty()) {
                ollama_cloud.api_endpoint = endpoint;
            }
            
            std::string key = parseJsonString(section, "api_key", "");
            if (!key.empty()) {
                ollama_cloud.api_key = key;
            }
            
            std::string tpl = parseJsonString(section, "template", "");
            if (!tpl.empty()) ollama_cloud.template_format = tpl;
            
            ollama_cloud.enabled = parseJsonBool(section, "enabled", false);
        }
    }

    // === Parse Grok ===
    size_t grok_pos = content.find("\"grok\":");
    if (grok_pos != std::string::npos) {
        size_t grok_end = content.find("}", grok_pos);
        if (grok_end != std::string::npos) {
            std::string section = content.substr(grok_pos, grok_end - grok_pos + 1);
            
            std::string endpoint = parseJsonString(section, "api_endpoint", "https://api.x.ai/v1");
            if (!endpoint.empty()) {
                grok.api_endpoint = endpoint;
            }
            
            std::string key = parseJsonString(section, "api_key", "");
            if (!key.empty()) {
                grok.api_key = key;
            }
            
            std::string tpl = parseJsonString(section, "template", "");
            if (!tpl.empty()) grok.template_format = tpl;
            
            grok.enabled = parseJsonBool(section, "enabled", false);
        }
    }

    // === Parse OpenRouter ===
    size_t or_pos = content.find("\"openrouter\":");
    if (or_pos != std::string::npos) {
        size_t or_end = content.find("}", or_pos);
        if (or_end != std::string::npos) {
            std::string section = content.substr(or_pos, or_end - or_pos + 1);
            
            std::string endpoint = parseJsonString(section, "api_endpoint", "https://openrouter.ai/api/v1");
            if (!endpoint.empty()) {
                openrouter.api_endpoint = endpoint;
            }
            
            std::string key = parseJsonString(section, "api_key", "");
            if (!key.empty()) {
                openrouter.api_key = key;
            }
            
            std::string referer = parseJsonString(section, "referer", "");
            if (!referer.empty()) {
                openrouter.referer = referer;
            }
            
            std::string tpl = parseJsonString(section, "template", "");
            if (!tpl.empty()) openrouter.template_format = tpl;
            
            openrouter.enabled = parseJsonBool(section, "enabled", false);
        }
    }
}

// Save configuration to file
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
    file << "  \"default_engine\": \"" << jsonEscape(default_engine) << "\",\n";
    
    // === Save Ollama ===
    file << "  \"ollama\": {\n";
    file << "    \"api_endpoint\": \"" << jsonEscape(ollama.api_endpoint) << "\",\n";
    file << "    \"template\": \"" << jsonEscape(ollama.template_format) << "\",\n";
    file << "    \"enabled\": " << (ollama.enabled ? "true" : "false") << "\n";
    file << "  },\n";
    
    // === Save Hugging Face ===
    file << "  \"huggingface\": {\n";
    file << "    \"api_endpoint\": \"" << jsonEscape(huggingface.api_endpoint) << "\",\n";
    file << "    \"api_key\": \"" << jsonEscape(huggingface.api_key) << "\",\n";
    file << "    \"model\": \"" << jsonEscape(huggingface.model) << "\",\n";
    file << "    \"template\": \"" << jsonEscape(huggingface.template_format) << "\",\n";
    file << "    \"enabled\": " << (huggingface.enabled ? "true" : "false") << "\n";
    file << "  },\n";
    
    // === Save Ollama Cloud ===
    file << "  \"ollama_cloud\": {\n";
    file << "    \"api_endpoint\": \"" << jsonEscape(ollama_cloud.api_endpoint) << "\",\n";
    file << "    \"api_key\": \"" << jsonEscape(ollama_cloud.api_key) << "\",\n";
    file << "    \"template\": \"" << jsonEscape(ollama_cloud.template_format) << "\",\n";
    file << "    \"enabled\": " << (ollama_cloud.enabled ? "true" : "false") << "\n";
    file << "  },\n";
    
    // === Save Grok ===
    file << "  \"grok\": {\n";
    file << "    \"api_endpoint\": \"" << jsonEscape(grok.api_endpoint) << "\",\n";
    file << "    \"api_key\": \"" << jsonEscape(grok.api_key) << "\",\n";
    file << "    \"template\": \"" << jsonEscape(grok.template_format) << "\",\n";
    file << "    \"enabled\": " << (grok.enabled ? "true" : "false") << "\n";
    file << "  },\n";
    
    // === Save OpenRouter ===
    file << "  \"openrouter\": {\n";
    file << "    \"api_endpoint\": \"" << jsonEscape(openrouter.api_endpoint) << "\",\n";
    file << "    \"api_key\": \"" << jsonEscape(openrouter.api_key) << "\",\n";
    file << "    \"referer\": \"" << jsonEscape(openrouter.referer) << "\",\n";
    file << "    \"template\": \"" << jsonEscape(openrouter.template_format) << "\",\n";
    file << "    \"enabled\": " << (openrouter.enabled ? "true" : "false") << "\n";
    file << "  }\n";
    file << "}\n";
    
    file.close();
}

// Create default config file
void Config::createDefaultConfig(const std::string& path) const {
    // Ensure directory exists
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        std::string dir = path.substr(0, lastSlash);
        ensureDirectoryExists(dir);
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create default config file at: " << path << std::endl;
        return;
    }
    
    file << getDefaultConfigJson();
    file.close();
    
    std::cout << "Created default config at: " << path << std::endl;
}

// Get default config file path
std::string Config::getConfigPath() const {
#ifdef _WIN32
    return getHomeDirectory() + "\\.tAI\\config.json";
#else
    return getHomeDirectory() + "/.tAI/config.json";
#endif
}