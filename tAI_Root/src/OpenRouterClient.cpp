#include "tAI/OpenRouterClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>

OpenRouterClient::OpenRouterClient(const std::string& api_key,
                                 const std::string& endpoint,
                                 const std::string& referer)
    : api_key_(api_key), endpoint_(endpoint), referer_(referer) {}

std::string OpenRouterClient::chat(const std::string& user_query,
                                 const std::string& system_prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize CURL";

    // Ensure we have a proper chat completions URL
    std::string url = endpoint_;
    if (url.find("/chat/completions") == std::string::npos) {
        if (!url.empty() && url.back() != '/')
            url += '/';
        url += "chat/completions";
    }

    // Build JSON payload (OpenAI‑compatible format, using OpenRouter's "auto" model)
    std::string payload = R"({"model":"auto","messages":[)";

    if (!system_prompt.empty()) {
        payload += R"({"role":"system","content":")" + jsonEscape(system_prompt) + R"("},)";
    }

    payload += R"({"role":"user","content":")" + jsonEscape(user_query) + R"("}]})";

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!api_key_.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());
    }
    if (!referer_.empty()) {
        headers = curl_slist_append(headers, ("Referer: " + referer_).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);  // 5 minute timeout

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}