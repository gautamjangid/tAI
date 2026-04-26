#include "tAI/HuggingfaceClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

HuggingfaceClient::HuggingfaceClient(const std::string& api_key,
                                     const std::string& model_id,
                                     const std::string& api_endpoint)
    : api_key_(api_key), model_id_(model_id), endpoint_(api_endpoint) {}

std::string HuggingfaceClient::chat(const std::string& user_query,
                                    const std::string& system_prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize CURL";

    // Build URL: base endpoint + /models/{model_id}/v1/chat/completions
    std::string url = endpoint_;
    if (url.empty()) {
        url = "https://router.huggingface.co/v1/chat/completions";
    }

    // Build JSON payload (OpenAI‑compatible format)
    std::string payload = "{\"model\":\"" + jsonEscape(model_id_) + "\",\"messages\":[";
    if (!system_prompt.empty()) {
        payload += "{\"role\":\"system\",\"content\":\"" + jsonEscape(system_prompt) + "\"},";
    }
    payload += "{\"role\":\"user\",\"content\":\"" + jsonEscape(user_query) + "\"}]}";

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
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
