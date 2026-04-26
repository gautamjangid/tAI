#pragma once
#include "IApiClient.h"
#include <string>

class OllamaCloudClient : public IApiClient {
public:
    OllamaCloudClient(const std::string& api_key, 
                      const std::string& endpoint = "https://api.ollama.cloud");
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;
private:
    std::string api_key_;
    std::string endpoint_;
};