#pragma once
#include "IApiClient.h"
#include <string>

class OllamaClient : public IApiClient {
public:
OllamaClient();
std::string chat(const std::string& user_query,
const std::string& system_prompt = "") override;
//void setProxy(const std::string& proxy) override;
private:
std::string proxy_;
};
