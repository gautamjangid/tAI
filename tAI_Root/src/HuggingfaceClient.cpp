#include "tAI/HuggingfaceClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>

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
        url = "https://router.huggingface.co/v1/chat/completions"
    }

    // Build JSON payload (OpenAI‑compatible format)
    std::string payload = R"({"model":)" +  jsonEscape(model_id) + R"(",")"
    std::string payload += R"("messages":[)";
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}
