#ifndef AICONFIG_H
#define AICONFIG_H

#include <cstdlib>
#include <functional>
#include <string>

// Settings of the optional LLM-backed random events, read from the environment:
//   FUSHENGJI_AI_API_KEY   bearer token (never stored in the repository)
//   FUSHENGJI_AI_BASE_URL  OpenAI-compatible endpoint, e.g. https://api.openai.com/v1
//                          (a full ".../chat/completions" URL is accepted too)
//   FUSHENGJI_AI_MODEL     model name
//   FUSHENGJI_AI_TIMEOUT   request timeout in seconds
//   FUSHENGJI_OFFLINE      "1"/"true" forces the built-in offline events
struct AiConfig {
    using Getenv = std::function<const char *(const char *)>;

    std::string baseUrl = "https://api.openai.com/v1";
    std::string model = "gpt-4o-mini";
    std::string apiKey;
    int timeoutSeconds = 20;
    bool baseUrlFromEnv = false;
    bool forceOffline = false;

    // The LLM is used when an API key is present, or when a custom base URL was
    // given (local servers such as Ollama or llama.cpp do not need a key).
    bool enabled() const { return !forceOffline && (!apiKey.empty() || baseUrlFromEnv); }

    // ".../chat/completions" endpoint derived from baseUrl.
    std::string chatCompletionsUrl() const;

    static AiConfig fromEnv(const Getenv &getenv = [](const char *name) { return std::getenv(name); });
};

#endif
