#pragma once
#include "IApiClient.h"
#include <string>

class HuggingfaceClient : public IApiClient {
public:
    HuggingfaceClient(const std::string& api_key,
                      const std::string& model_id,
                      const std::string& api_endpoint);
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;
private:
    std::string api_key_;
    std::string model_id_;
    std::string endpoint_;
};
