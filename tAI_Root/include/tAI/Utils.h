#pragma once
#include <string>

// ─── Core Helpers ─────────────────────────────────────────────────────────────
std::string getHomeDirectory();
std::string urlEncode(const std::string& value);
std::string jsonEscape(const std::string& value);
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output);

// ─── Error Logger ─────────────────────────────────────────────────────────────
/**
 * @brief Log an error message to ~/.tAI/logs/error-<timestamp>.log
 *
 * Creates the log directory if it doesn't exist.
 * Each call writes a new timestamped log file containing the context and
 * the error message.
 *
 * @param context  Short description of where the error occurred (e.g. "API call")
 * @param message  The error message / exception what()
 */
void logError(const std::string& context, const std::string& message);

// ─── Typing Effect ────────────────────────────────────────────────────────────
/**
 * @brief Print a string to stdout with a simulated typing effect.
 *
 * Outputs each character with a small delay so the response appears to
 * "type itself out" – reducing the perceived wall-clock wait and making
 * long responses easier to read incrementally.
 *
 * @param text          The string to print
 * @param delay_ms      Milliseconds between characters (default 18 ms ≈ average typing)
 */
void typingPrint(const std::string& text, int delay_ms = 18);