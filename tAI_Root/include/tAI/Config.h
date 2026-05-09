#pragma once

// ─── tAI Version ─────────────────────────────────────────────────────────────
#define TAI_VERSION "1.1.1"
// ─────────────────────────────────────────────────────────────────────────────

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
 *   "default_engine": "duckduckgo",
 *   "duckduckgo": {
 *     "api_endpoint": "https://api.duckduckgo.com",
 *     "enabled": true
 *   },
 *   "ollama": {
 *     "api_endpoint": "http://localhost:11434",
 *     "enabled": true
 *   },
 *   "huggingface": {
 *     "api_endpoint": "https://router.huggingface.co/v1/chat/completions",
 *     "api_key": "",
 *     "model": "openai/gpt-oss-120b:groq",
 *     "enabled": false
 *   },
 *   "ollama_cloud": {
 *     "api_endpoint": "http://localhost:11434/api/chat",
 *     "api_key": "",
 *     "enabled": false
 *   },
 *   "grok": {
 *     "api_endpoint": "https://api.groq.com/openai/v1/chat/completions",
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

struct DuckDuckGoEngineConfig {
    std::string api_endpoint;
    bool enabled;

    DuckDuckGoEngineConfig()
        : api_endpoint("https://api.duckduckgo.com"), enabled(true) {}
};

struct OllamaEngineConfig {
    std::string api_endpoint;
    std::string template_format;
    bool enabled;

    OllamaEngineConfig()
        : api_endpoint("http://localhost:11434"), template_format(""), enabled(false) {}
};

struct HuggingfaceEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    std::string model;
    std::string template_format;
    bool enabled;

    HuggingfaceEngineConfig()
        : api_endpoint("https://router.huggingface.co/v1/chat/completions"),
          api_key(""),
          model("openai/gpt-oss-120b:groq"),
          template_format("answer in one line sentence if question is not related to code"),
          enabled(false) {}
};

struct OllamaCloudEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    std::string model;
    std::string template_format;
    bool enabled;

    OllamaCloudEngineConfig()
        : api_endpoint("http://localhost:11434/api/chat"),
          api_key(""),
          model("kimi-k2.6:cloud"),
          template_format(""),
          enabled(false) {}
};

struct GrokEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    std::string model;
    std::string template_format;
    bool enabled;

    GrokEngineConfig()
        : api_endpoint("https://api.groq.com/openai/v1/chat/completions"),
          api_key(""),
          model("groq/compound-mini"),
          template_format(""),
          enabled(false) {}
};

struct OpenRouterEngineConfig {
    std::string api_endpoint;
    std::string api_key;
    std::string referer;
    std::string model;
    std::string template_format;
    bool enabled;

    OpenRouterEngineConfig()
        : api_endpoint("https://openrouter.ai/api/v1"),
          api_key(""),
          referer(""),
          model("auto"),
          template_format(""),
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
     * - default_engine: "duckduckgo" (no API key, no local server needed)
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
    std::string default_engine;  ///< Default AI engine (duckduckgo, ollama, huggingface, ollama_cloud, grok, openrouter)
    std::string version;         ///< Application version string (read-only, set from TAI_VERSION)

    // Engine-specific configurations
    DuckDuckGoEngineConfig duckduckgo;      ///< DuckDuckGo Instant Answer (free, no API key)
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
