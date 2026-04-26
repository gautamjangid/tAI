#include "tAI/Utils.h"
#include <cstdlib>
#include <cstdio>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

std::string getHomeDirectory() {
const char* home = getenv("HOME");
if (home) return home;
#ifdef _WIN32
home = getenv("USERPROFILE");
if (home) return home;
#endif
return ".";
}

std::string urlEncode(const std::string& value) {
std::ostringstream escaped;
escaped.fill('0');
escaped << std::hex;
for (char c : value) {
if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
escaped << c;
} else {
escaped << '%' << std::uppercase << int((unsigned char)c);
}
}
return escaped.str();
}

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    for (char c : value) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if ((unsigned char)c < 0x20) {
                    // Control characters
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                    escaped += buf;
                } else {
                    escaped += c;
                }
        }
    }
    return escaped;
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
size_t total = size * nmemb;
output->append((char*)contents, total);
return total;
}