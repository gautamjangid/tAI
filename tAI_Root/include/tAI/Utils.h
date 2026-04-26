#pragma once
#include <string>

std::string getHomeDirectory();
std::string urlEncode(const std::string& value);
std::string jsonEscape(const std::string& value);
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output);