#pragma once
#include "tAI/IApiClient.h"
#include <string>

/**
 * @class DuckDuckGoClient
 * @brief Search-based AI client using the DuckDuckGo Instant Answer API.
 *
 * Queries https://api.duckduckgo.com/ with no API key required.
 * Returns structured knowledge-graph data: direct answers, abstract
 * summaries, and related topics.
 *
 * Best suited for factual queries (definitions, conversions, quick facts).
 * For open-ended or creative prompts, prefer an LLM-backed engine.
 */
class DuckDuckGoClient : public IApiClient {
public:
    /**
     * @brief Constructor
     * @param endpoint  Base URL for the DDG API (default: https://api.duckduckgo.com/)
     */
    explicit DuckDuckGoClient(
        const std::string& endpoint = "https://api.duckduckgo.com/");

    /**
     * @brief Query DuckDuckGo Instant Answer API
     * @param user_query   The user's search query
     * @param system_prompt Ignored (DDG has no system-prompt concept)
     * @return Human-readable answer assembled from DDG JSON fields
     */
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;

private:
    std::string endpoint_;
};
