#pragma once
#include "IApiClient.h"
#include <string>

class OpenRouterClient : public IApiClient {
public:
    OpenRouterClient(const std::string& api_key,
                     const std::string& referer = "");
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;
private:
    std::string api_key_;
    std::string referer_;
};
