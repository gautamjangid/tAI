#include "tAI/GrokClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

GrokClient::GrokClient(const std::string& api_key,
                       const std::string& endpoint)
    : api_key_(api_key), endpoint_(endpoint) {}

std::string GrokClient::chat(const std::string& user_query,
                             const std::string& system_prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize CURL";

    // Use the configured endpoint (default points to Grok chat endpoint)
    std::string url = endpoint_;

    // Build JSON payload with proper escaping (OpenAI-compatible format)
    std::string payload = R"({"model":"grok-1","messages":[)";
    if (!system_prompt.empty()) {
        payload += R"({"role":"system","content":")" + jsonEscape(system_prompt) + R"("},)";
    }
    payload += R"({"role":"user","content":")" + jsonEscape(user_query) + R"("}]})";

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    // Set a reasonable timeout (default 300 seconds)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    } else {
        try {
            auto j = nlohmann::json::parse(response);
            if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty()) {
                auto& choice = j["choices"][0];
                if (choice.contains("message") && choice["message"].contains("content")) {
                    response = choice["message"]["content"].get<std::string>();
                }
            } else if (j.contains("error")) {
                if (j["error"].is_object() && j["error"].contains("message")) {
                    response = "API Error: " + j["error"]["message"].get<std::string>();
                } else if (j["error"].is_string()) {
                    response = "API Error: " + j["error"].get<std::string>();
                }
            }
        } catch (const nlohmann::json::parse_error& e) {
            // Keep original response if parsing fails
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}