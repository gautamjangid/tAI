#include "tAI/Utils.h"
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

// ─── getHomeDirectory ──────────────────────────────────────────────────────────
std::string getHomeDirectory() {
    const char* home = getenv("HOME");
    if (home) return home;
#ifdef _WIN32
    home = getenv("USERPROFILE");
    if (home) return home;
#endif
    return ".";
}

// ─── urlEncode ────────────────────────────────────────────────────────────────
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

// ─── jsonEscape ───────────────────────────────────────────────────────────────
std::string jsonEscape(const std::string& value) {
    std::string escaped;
    for (char c : value) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b";  break;
            case '\f': escaped += "\\f";  break;
            case '\n': escaped += "\\n";  break;
            case '\r': escaped += "\\r";  break;
            case '\t': escaped += "\\t";  break;
            default:
                if ((unsigned char)c < 0x20) {
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

// ─── writeCallback ────────────────────────────────────────────────────────────
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

// ─── logError ─────────────────────────────────────────────────────────────────
void logError(const std::string& context, const std::string& message) {
    try {
        // Build log directory path: ~/.tAI/logs/
        std::string logDir = getHomeDirectory() + "/.tAI/logs";
        std::filesystem::create_directories(logDir);

        // Generate timestamp for filename: error-YYYYMMDDHHmmss.log
        auto now      = std::chrono::system_clock::now();
        auto time_t_  = std::chrono::system_clock::to_time_t(now);
        std::tm* tm_  = std::localtime(&time_t_);

        char tsBuf[20];
        std::strftime(tsBuf, sizeof(tsBuf), "%Y%m%d%H%M%S", tm_);

        std::string logFile = logDir + "/error-" + tsBuf + ".log";

        // Build human-readable timestamp for inside the file
        char hrBuf[24];
        std::strftime(hrBuf, sizeof(hrBuf), "%Y-%m-%d %H:%M:%S", tm_);

        std::ofstream out(logFile, std::ios::out | std::ios::trunc);
        if (out.is_open()) {
            out << "=== tAI Error Log ===\n";
            out << "Timestamp : " << hrBuf        << "\n";
            out << "Context   : " << context       << "\n";
            out << "Message   : " << message       << "\n";
            out << "=====================\n";
            out.close();
        } else {
            // Last-resort: print to stderr if we cannot write the log
            std::cerr << "[tAI] Could not write error log to: " << logFile << "\n";
        }
    } catch (...) {
        // Silently swallow – logging must never crash the process
    }
}

// ─── typingPrint ──────────────────────────────────────────────────────────────
void typingPrint(const std::string& text, int delay_ms) {
    for (char c : text) {
        std::cout << c << std::flush;
        // Newlines and spaces feel more natural with a shorter pause
        int actual_delay = (c == '\n' || c == ' ') ? delay_ms / 3 : delay_ms;
        if (actual_delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(actual_delay));
        }
    }
    std::cout << std::endl;
}