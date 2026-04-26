#include "tAI/OpenRouterClient.h"
#include "tAI/Utils.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>

OpenRouterClient::OpenRouterClient(const std::string& api_key,
                                   const std::string& referer)
    : api_key_(api_key), referer_(referer) {}

std::string OpenRouterClient::chat(const std::string& user_query,
                                   const std::string& system_prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize CURL";
    
    std::string url = "https://openrouter.ai/api/v1/chat/completions";
    
    // Build JSON payload with proper escaping (OpenAI-compatible format)
    std::string payload = R"({"model":"auto","messages":[)";
    if (!system_prompt.empty()) {
        payload += R"({"role":"system","content":")" + jsonEscape(system_prompt) + R"("},)";
    }
    payload += R"({"role":"user","content":")" + jsonEscape(user_query) + R"("}]})";
    
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());
    
    // Add referer header if provided (for tracking/analytics at OpenRouter)
    if (!referer_.empty()) {
        headers = curl_slist_append(headers, ("HTTP-Referer: " + referer_).c_str());
    }
    
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
```

Perfect! I've successfully created all 4 new client implementations for tAI. Here's a summary of what was created:

## Summary

### Utility Updates
- **Enhanced `Utils.h` and `Utils.cpp`**: Added `jsonEscape()` function for proper JSON string escaping, handling special characters like quotes, backslashes, newlines, and control characters.

### 4 New API Client Implementations

#### 1. **OllamaCloudClient**
- **Files**: `include/tAI/OllamaCloudClient.h` and `src/OllamaCloudClient.cpp`
- **Constructor**: Takes `api_key` and optional `endpoint` (defaults to `https://api.ollama.cloud`)
- **API Endpoint**: Dynamically constructs `/api/chat`
- **Features**: Bearer token authentication, JSON payload escaping, 300-second timeout

#### 2. **HuggingfaceClient**
- **Files**: `include/tAI/HuggingfaceClient.h` and `src/HuggingfaceClient.cpp`
- **Constructor**: Takes `api_key` and optional `model_id` (defaults to `meta-llama/Llama-2-7b-chat-hf`)
- **API Endpoint**: `https://api-inference.huggingface.co/models/{model_id}/v1/chat/completions`
- **Features**: Configurable model selection, Bearer token auth, proper JSON escaping

#### 3. **GrokClient**
- **Files**: `include/tAI/GrokClient.h` and `src/GrokClient.cpp`
- **Constructor**: Takes `api_key`
- **API Endpoint**: `https://api.x.ai/chat/completions`
- **Features**: OpenAI-compatible format, uses model `grok-1`, Bearer token authentication

#### 4. **OpenRouterClient**
- **Files**: `include/tAI/OpenRouterClient.h` and `src/OpenRouterClient.cpp`
- **Constructor**: Takes `api_key` and optional `referer` (for tracking/analytics)
- **API Endpoint**: `https://openrouter.ai/api/v1/chat/completions`
- **Features**: OpenAI-compatible format, optional HTTP-Referer header support, uses model `auto`

### Common Features Across All Clients
✅ Inherit from `IApiClient` interface  
✅ Implement `chat(user_query, system_prompt)` method  
✅ Use CURL for HTTP POST requests  
✅ Bearer token authentication  
✅ Proper JSON escaping for payloads  
✅ 300-second timeout for API calls  
✅ Error handling with logging  
✅ Support for system prompts and user queries

All implementations follow the established pattern from `OpenAIClient.cpp` and are ready to be compiled into the project.