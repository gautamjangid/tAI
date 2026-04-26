#pragma once

#include "IApiClient.h"
#include <string>

/**
 * @class GrokClient
 * @brief xAI Grok API client with configurable endpoint.
 *
 * The endpoint can be changed via the configuration file. By default it points
 * to the public Grok chat endpoint.
 */
class GrokClient : public IApiClient {
public:
    /**
     * @brief Construct a Grok client.
     *
     * @param api_key   Your Grok API key.
     * @param endpoint  API endpoint URL (defaults to https://api.x.ai/v1).
     */
    GrokClient(const std::string& api_key,
               const std::string& endpoint = "https://api.x.ai/v1");

    /**
     * @brief Send a chat request.
     *
     * @param user_query     The user's message.
     * @param system_prompt  Optional system prompt (defaults to empty).
     *
     * @return The raw response string from the Grok API.
     */
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;

private:
    std::string api_key_;      ///< Grok API key.
    std::string endpoint_;    ///< Base endpoint for the Grok service.
};