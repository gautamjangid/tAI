#pragma once
#include "IApiClient.h"
#include <string>

/**
 * @class OpenRouterClient
 * @brief Client for OpenRouter aggregator with configurable endpoint.
 */
class OpenRouterClient : public IApiClient {
public:
    /**
     * @brief Construct an OpenRouter client.
     *
     * @param api_key   Your OpenRouter API key.
     * @param model     The model to use (defaults to "auto").
     * @param endpoint  API endpoint URL (defaults to official OpenRouter endpoint).
     * @param referer   Optional referer for usage tracking.
     */
    OpenRouterClient(const std::string& api_key,
                     const std::string& model = "auto",
                     const std::string& endpoint = "https://openrouter.ai/api/v1",
                     const std::string& referer = "");

    /**
     * @brief Send a chat request to OpenRouter.
     *
     * @param user_query     The user's message.
     * @param system_prompt  Optional system prompt.
     *
     * @return The raw response string from OpenRouter.
     */
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;

private:
    std::string api_key_;
    std::string model_;    ///< Model name (e.g., "auto").
    std::string endpoint_;
    std::string referer_;
};