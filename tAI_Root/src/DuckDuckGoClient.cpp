#include "tAI/DuckDuckGoClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

DuckDuckGoClient::DuckDuckGoClient(const std::string& endpoint)
    : endpoint_(endpoint)
{
    // Strip trailing slash – we append '?' ourselves
    if (!endpoint_.empty() && endpoint_.back() == '/')
        endpoint_.pop_back();
}

std::string DuckDuckGoClient::chat(const std::string& user_query,
                                   const std::string& /*system_prompt*/)
{
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize CURL";

    // Build URL: GET https://api.duckduckgo.com/?q=<query>&format=json&no_html=1&skip_disambig=1
    std::string url = endpoint_
                    + "/?q=" + urlEncode(user_query)
                    + "&format=json&no_html=1&skip_disambig=1";

    std::string raw;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    // Identify ourselves politely to DDG
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "tAI-CLI/1.0 (https://github.com/gautamjangid/tAI)");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::string err = std::string("CURL error: ") + curl_easy_strerror(res);
        std::cerr << err << std::endl;
        return "Error: " + err;
    }

    // ── Parse JSON response ────────────────────────────────────────────────────
    try {
        auto j = nlohmann::json::parse(raw);
        std::ostringstream out;

        // 1. Direct answer (e.g. "42 degrees Fahrenheit = 5.56 Celsius")
        std::string answer;
        if (j.contains("Answer") && j["Answer"].is_string())
            answer = j["Answer"].get<std::string>();
        if (!answer.empty()) {
            out << answer << "\n";
        }

        // 2. Abstract text (Wikipedia-style summary)
        std::string abstract_text;
        if (j.contains("AbstractText") && j["AbstractText"].is_string())
            abstract_text = j["AbstractText"].get<std::string>();

        std::string abstract_source;
        if (j.contains("AbstractSource") && j["AbstractSource"].is_string())
            abstract_source = j["AbstractSource"].get<std::string>();

        if (!abstract_text.empty()) {
            out << abstract_text << "\n";
            if (!abstract_source.empty())
                out << "[Source: " << abstract_source << "]\n";
        }

        // 3. Definition (for word lookups)
        std::string definition;
        if (j.contains("Definition") && j["Definition"].is_string())
            definition = j["Definition"].get<std::string>();

        std::string def_source;
        if (j.contains("DefinitionSource") && j["DefinitionSource"].is_string())
            def_source = j["DefinitionSource"].get<std::string>();

        if (!definition.empty()) {
            out << "\nDefinition: " << definition << "\n";
            if (!def_source.empty())
                out << "[Source: " << def_source << "]\n";
        }

        // 4. Related topics (up to 5 bullets)
        if (j.contains("RelatedTopics") && j["RelatedTopics"].is_array()) {
            int shown = 0;
            bool header_written = false;
            for (auto& topic : j["RelatedTopics"]) {
                if (shown >= 5) break;
                if (!topic.is_object()) continue;
                if (!topic.contains("Text") || !topic["Text"].is_string()) continue;
                std::string text = topic["Text"].get<std::string>();
                if (text.empty()) continue;
                if (!header_written) {
                    out << "\nRelated:\n";
                    header_written = true;
                }
                out << "  • " << text << "\n";
                ++shown;
            }
        }

        std::string result = out.str();

        // Trim leading/trailing whitespace
        size_t start = result.find_first_not_of(" \t\n\r");
        size_t end   = result.find_last_not_of(" \t\n\r");
        if (start == std::string::npos)
            result.clear();
        else
            result = result.substr(start, end - start + 1);

        if (result.empty()) {
            return "No direct answer found for: \"" + user_query + "\"\n"
                   "Try rephrasing, or use an LLM engine (e.g. -m ollama) "
                   "for open-ended questions.";
        }

        return result;

    } catch (const nlohmann::json::parse_error& e) {
        return std::string("Error: Failed to parse DuckDuckGo response: ") + e.what();
    }
}
