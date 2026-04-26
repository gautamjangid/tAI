#pragma once
#include "IApiClient.h"
#include <string>

class OpenAIClient : public IApiClient {
public:
    OpenAIClient(const std::string& api_key);
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;
private:
    std::string api_key_;
};