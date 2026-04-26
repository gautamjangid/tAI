#pragma once
#include <string>

class IApiClient {
public:
    virtual ~IApiClient() = default;
    virtual std::string chat(const std::string& user_query,
                             const std::string& system_prompt = "") = 0;
};