#include "tAI/DuckDuckGoClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <iomanip>
#include <cmath>

// ── Helper: Simple unit conversion handler for common cases ──────────────────
static std::string trySimpleConversion(const std::string& query) {
    // Regex patterns for common conversions
    std::regex mile_to_km(R"(\b(?:how\s+)?(?:many\s+)?kilometer.*(?:in|from)\s+(?:a\s+)?mile|mile.*kilometer|mile.*km)", std::regex::icase);
    std::regex km_to_mile(R"(\b(?:how\s+)?(?:many\s+)?mile.*(?:in|from)\s+(?:a\s+)?kilometer|kilometer.*mile|km.*mile)", std::regex::icase);
    std::regex fahrenheit_to_celsius(R"(\b(?:fahrenheit|°?f).*(?:celsius|°?c)|convert.*fahrenheit.*celsius)", std::regex::icase);
    std::regex celsius_to_fahrenheit(R"(\b(?:celsius|°?c).*(?:fahrenheit|°?f)|convert.*celsius.*fahrenheit)", std::regex::icase);

    std::ostringstream result;
    
    if (std::regex_search(query, mile_to_km)) {
        result << "1 mile = 1.60934 kilometers\n"
               << "Or: 1 km = 0.621371 miles\n"
               << "[Conversion Formula]";
        return result.str();
    }
    if (std::regex_search(query, km_to_mile)) {
        result << "1 kilometer = 0.621371 miles\n"
               << "Or: 1 mile = 1.60934 kilometers\n"
               << "[Conversion Formula]";
        return result.str();
    }
    if (std::regex_search(query, fahrenheit_to_celsius)) {
        result << "°C = (°F - 32) × 5/9\n"
               << "Example: 32°F = 0°C, 212°F = 100°C\n"
               << "[Conversion Formula]";
        return result.str();
    }
    if (std::regex_search(query, celsius_to_fahrenheit)) {
        result << "°F = (°C × 9/5) + 32\n"
               << "Example: 0°C = 32°F, 100°C = 212°F\n"
               << "[Conversion Formula]";
        return result.str();
    }
    
    return "";  // No simple conversion matched
}

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
        std::string answer;

        // Try "Answer" field first (most common for direct answers like math)
        if (j.contains("Answer") && j["Answer"].is_string()) {
            answer = j["Answer"].get<std::string>();
            if (!answer.empty()) {
                out << answer << "\n";
            }
        }
        
        // Try "Definition" (word definitions)
        if (answer.empty() && j.contains("Definition") && j["Definition"].is_string()) {
            std::string definition = j["Definition"].get<std::string>();
            if (!definition.empty()) {
                answer = definition;
                std::string def_source;
                if (j.contains("DefinitionSource") && j["DefinitionSource"].is_string())
                    def_source = j["DefinitionSource"].get<std::string>();
                out << definition << "\n";
                if (!def_source.empty())
                    out << "[Source: " << def_source << "]\n";
            }
        }
        
        // Try "AbstractText" (Wikipedia summaries)
        if (answer.empty() && j.contains("AbstractText") && j["AbstractText"].is_string()) {
            std::string abstract_text = j["AbstractText"].get<std::string>();
            if (!abstract_text.empty()) {
                answer = abstract_text;
                std::string abstract_source;
                if (j.contains("AbstractSource") && j["AbstractSource"].is_string())
                    abstract_source = j["AbstractSource"].get<std::string>();
                out << abstract_text << "\n";
                if (!abstract_source.empty())
                    out << "[Source: " << abstract_source << "]\n";
            }
        }

        // Try "RelatedTopics" for suggestions if nothing found yet
        if (answer.empty() && j.contains("RelatedTopics") && j["RelatedTopics"].is_array()) {
            auto& topics = j["RelatedTopics"];
            if (!topics.empty()) {
                out << "No direct answer found. Related topics:\n";
                int shown = 0;
                for (auto& topic : topics) {
                    if (shown >= 3) break;
                    if (topic.is_object() && topic.contains("Text")) {
                        std::string text = topic["Text"].get<std::string>();
                        if (!text.empty()) {
                            out << "  • " << text << "\n";
                            ++shown;
                        }
                    }
                }
                answer = "partial";
            }
        }

        // Check AnswerType for hints about why we didn't get an answer
        std::string answer_type;
        if (j.contains("AnswerType") && j["AnswerType"].is_string()) {
            answer_type = j["AnswerType"].get<std::string>();
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
            // Try fallback: simple conversion handler
            std::string simple_conversion = trySimpleConversion(user_query);
            if (!simple_conversion.empty()) {
                return simple_conversion;
            }

            std::string suggestion = "No direct answer found for: \"" + user_query + "\"\n";
            if (!answer_type.empty()) {
                suggestion += "Query type detected: " + answer_type + "\n";
            }
            suggestion += "Try rephrasing your query, or use an LLM engine (e.g. -m ollama) for open-ended questions.";
            return suggestion;
        }

        return result;

    } catch (const nlohmann::json::parse_error& e) {
        return std::string("Error: Failed to parse DuckDuckGo response: ") + e.what();
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
