#pragma once
#include "IApiClient.h"
#include <string>

class HuggingfaceClient : public IApiClient {
public:
    HuggingfaceClient(const std::string& api_key,
                      const std::string& model_id = "meta-llama/Llama-2-7b-chat-hf");
    std::string chat(const std::string& user_query,
                     const std::string& system_prompt = "") override;
private:
    std::string api_key_;
    std::string model_id_;
};