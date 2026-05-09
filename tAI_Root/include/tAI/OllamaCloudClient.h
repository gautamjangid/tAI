#pragma once
#include "IApiClient.h"
#include <string>

class OllamaCloudClient : public IApiClient {
public:
    OllamaCloudClient(const std::string& api_key,
                      const std::string& model = "kimi-k2.6:cloud",
                      const std::string& endpoint = "http://localhost:11434/api/chat");
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;
private:
    std::string api_key_;
    std::string model_;
    std::string endpoint_;
};
