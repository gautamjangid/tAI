#include "tAI/OllamaClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>

OllamaClient::OllamaClient() {}

std::string OllamaClient::chat(const std::string& user_query, const std::string& system_prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize CURL";
    
    std::string url = "http://localhost:11434/api/chat";
    std::string payload = R"({"model":"llama2","messages":[)";
    
    if (!system_prompt.empty()) {
        payload += R"({"role":"system","content":")" + system_prompt + R"("},)";
    }
    
    payload += R"({"role":"user","content":")" + user_query + R"("}],"stream":false})";
    
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        response = "Error: " + std::string(curl_easy_strerror(res));
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return response;
}