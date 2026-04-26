#pragma once

#include <string>
#include <map>

/**
 * @class Config
 * @brief Configuration manager for tAI
 *
 * Handles loading and saving configuration from/to ~/.tAI/config.json
 * Stores user preferences such as API keys, default engine, and other settings.
 *
 * Configuration file format (JSON):
 * {
 *   "default_engine": "openai",
 *   "openai_key": "sk-xxxxxxxx",
 *   "ollama_cloud_key": "ollama-key-xxxxxxxx",
 *   "ollama_cloud_endpoint": "https://api.ollama.cloud",
 *   "huggingface_key": "hf_xxxxxxxx",
 *   "huggingface_model": "meta-llama/Llama-2-7b-chat-hf",
 *   "grok_key": "xai-xxxxxxxx",
 *   "openrouter_key": "sk-or-xxxxxxxx",
 *   "openrouter_referer": "https://myapp.com"
 * }
 */
class Config {
public:
    /**
     * @brief Constructor
     * Initializes config with default values:
     * - default_engine: "openai"
     * - all API keys: "" (empty)
     * - ollama_cloud_endpoint: "https://api.ollama.cloud"
     * - huggingface_model: "meta-llama/Llama-2-7b-chat-hf"
     * - openrouter_referer: "" (empty)
     */
    Config();

    /**
     * @brief Load configuration from file
     * @param path Path to configuration file
     *
     * Reads and parses JSON configuration file. If file doesn't exist,
     * silently returns with default values.
     */
    void load(const std::string& path);

    /**
     * @brief Save configuration to file
     * @param path Path to configuration file
     *
     * Writes current configuration as JSON to file.
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

    // Configuration fields - API Keys
    std::string openai_key;              ///< OpenAI API key (for GPT models)
    std::string ollama_cloud_key;        ///< Ollama Cloud API key
    std::string huggingface_key;         ///< Hugging Face Inference API key
    std::string grok_key;                ///< xAI Grok API key
    std::string openrouter_key;          ///< OpenRouter API key

    // Configuration fields - Engine settings
    std::string default_engine;          ///< Default AI engine (openai, ollama_cloud, huggingface, grok, openrouter, ollama_local)
    std::string ollama_cloud_endpoint;   ///< Ollama Cloud endpoint URL
    std::string huggingface_model;       ///< Hugging Face model ID
    std::string openrouter_referer;      ///< OpenRouter referer URL (for tracking)
};