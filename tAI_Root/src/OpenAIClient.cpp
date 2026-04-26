#include "tAI/OpenAIClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>

OpenAIClient::OpenAIClient(const std::string& api_key) : api_key_(api_key) {}

std::string OpenAIClient::chat(const std::string& user_query, const std::string& system_prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize CURL";
    
    std::string url = "https://api.openai.com/v1/chat/completions";
    
    // Build JSON payload
    std::string payload = R"({"model":"gpt-3.5-turbo","messages":[)";
    if (!system_prompt.empty()) {
        payload += R"({"role":"system","content":")" + system_prompt + R"("},)";
    }
    payload += R"({"role":"user","content":")" + user_query + R"("}]})";
    
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return response;
}