#pragma once

#include <string>
#include <map>

/**
 * @class Config
 * @brief Configuration manager for tAI with structured nested engine configs
 *
 * Handles loading and saving configuration from/to ~/.tAI/config.json
 * Supports multiple AI engines with per-engine configuration.
 *
 * Configuration file format (JSON):
 * {
 *   "default_engine": "ollama",
 *   "ollama": {
 *     "api_endpoint": "http://localhost:11434",
 *     "enabled": true
 *   },
 *   "huggingface": {
 *     "api_endpoint": "https://api-inference.huggingface.co",
 *     "api_key": "",
 *     "model": "meta-llama/Llama-2-7b-chat-hf",
 *     "enabled": false
 *   },
 *   "ollama_cloud": {
 *     "api_endpoint": "https://api.ollama.cloud",
 *     "api_key": "",
 *     "enabled": false
 *   },
 *   "grok": {
 *     "api_endpoint": "https://api.x.ai/v1",
 *     "api_key": "",
 *     "enabled": false
 *   },
 *   "openrouter": {
 *     "api_endpoint": "https://openrouter.ai/api/v1",
 *     "api_key": "",
 *     "referer": "",
 *     "enabled": false
 *   }
 * }
 */

// Individual engine configuration structures
struct OllamaEngineConfig {
    std::string api_endpoint;
    bool enabled;
    
    OllamaEngineConfig() 
        : api_endpoint("http://localhost:11434"), enabled(true) {}
};

struct HuggingfaceEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    std::string model;
    bool enabled;
    
    HuggingfaceEngineConfig()
        : api_endpoint("https://api-inference.huggingface.co"),
          api_key(""),
          model("meta-llama/Llama-2-7b-chat-hf"),
          enabled(false) {}
};

struct OllamaCloudEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    bool enabled;
    
    OllamaCloudEngineConfig()
        : api_endpoint("https://api.ollama.cloud"),
          api_key(""),
          enabled(false) {}
};

struct GrokEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    bool enabled;
    
    GrokEngineConfig()
        : api_endpoint("https://api.x.ai/v1"),
          api_key(""),
          enabled(false) {}
};

struct OpenRouterEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    std::string referer;
    bool enabled;
    
    OpenRouterEngineConfig()
        : api_endpoint("https://openrouter.ai/api/v1"),
          api_key(""),
          referer(""),
          enabled(false) {}
};

/**
 * @class Config
 * @brief Main configuration manager
 *
 * Loads and manages all engine configurations from a single JSON file.
 * Default engine is Ollama (local) which requires no API key.
 */
class Config {
public:
    /**
     * @brief Constructor
     * Initializes config with default values:
     * - default_engine: "ollama" (local, free)
     * - All engine configs initialized with defaults
     */
    Config();

    /**
     * @brief Load configuration from file
     * @param path Path to configuration file
     *
     * Reads and parses JSON configuration file. If file doesn't exist,
     * creates a default config file at that location.
     */
    void load(const std::string& path);

    /**
     * @brief Save configuration to file
     * @param path Path to configuration file
     *
     * Writes current configuration as formatted JSON to file.
     * Creates parent directories if they don't exist.
     */
    void save(const std::string& path) const;

    /**
     * @brief Get the default configuration file path
     * @return Full path to config file (~/.tAI/config.json)
     *
     * Returns platform-specific path:
     * - Linux/macOS: ~/.tAI/config.json
     * - Windows: %USERPROFILE%\.tAI\config.json
     */
    std::string getConfigPath() const;

    /**
     * @brief Create a default configuration file
     * @param path Path where to create the default config
     *
     * Creates a well-formatted default config file with all engines
     * configured to their default endpoints. Ollama (local) is enabled by default.
     */
    void createDefaultConfig(const std::string& path) const;

    // Main settings
    std::string default_engine;  ///< Default AI engine (ollama, huggingface, ollama_cloud, grok, openrouter)

    // Engine-specific configurations
    OllamaEngineConfig ollama;              ///< Local Ollama (free, self-hosted)
    HuggingfaceEngineConfig huggingface;    ///< Hugging Face (free tier available)
    OllamaCloudEngineConfig ollama_cloud;   ///< Ollama Cloud
    GrokEngineConfig grok;                  ///< xAI Grok
    OpenRouterEngineConfig openrouter;      ///< OpenRouter (free tier available)

private:
    /**
     * @brief Parse a string value from JSON
     * @param json The JSON string
     * @param key The key to search for
     * @param defaultValue Default value if key not found
     * @return Parsed value or default
     */
    std::string parseJsonString(const std::string& json, 
                               const std::string& key,
                               const std::string& defaultValue = "");

    /**
     * @brief Parse a boolean value from JSON
     * @param json The JSON string
     * @param key The key to search for
     * @param defaultValue Default value if key not found
     * @return Parsed boolean or default
     */
    bool parseJsonBool(const std::string& json,
                      const std::string& key,
                      bool defaultValue = false);

    /**
     * @brief Escape a string for JSON output
     * @param input The string to escape
     * @return JSON-safe escaped string
     */
    std::string jsonEscape(const std::string& input) const;

    /**
     * @brief Get the default config as a JSON string
     * @return Formatted JSON string with all defaults
     */
    std::string getDefaultConfigJson() const;

    /**
     * @brief Ensure directory exists
     * @param path Directory path
     */
    void ensureDirectoryExists(const std::string& path) const;
};