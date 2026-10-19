#include "AiConfig.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "util.h"

namespace {

std::string valueOf(const AiConfig::Getenv &getenv, const char *name) {
    const char *v = getenv(name);
    return v ? trim(v) : std::string();
}

bool isTruthy(std::string v) {
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return v == "1" || v == "true" || v == "yes" || v == "on";
}

}  // namespace

std::string AiConfig::chatCompletionsUrl() const {
    std::string url = baseUrl;
    while (!url.empty() && url.back() == '/') url.pop_back();
    const std::string suffix = "/chat/completions";
    if (url.size() >= suffix.size() && url.compare(url.size() - suffix.size(), suffix.size(), suffix) == 0)
        return url;
    return url + suffix;
}

AiConfig AiConfig::fromEnv(const Getenv &getenv) {
    AiConfig cfg;
    cfg.apiKey = valueOf(getenv, "FUSHENGJI_AI_API_KEY");
    if (std::string v = valueOf(getenv, "FUSHENGJI_AI_BASE_URL"); !v.empty()) {
        cfg.baseUrl = v;
        cfg.baseUrlFromEnv = true;
    }
    if (std::string v = valueOf(getenv, "FUSHENGJI_AI_MODEL"); !v.empty())
        cfg.model = v;
    if (std::string v = valueOf(getenv, "FUSHENGJI_AI_TIMEOUT"); !v.empty()) {
        char *end = nullptr;
        long t = std::strtol(v.c_str(), &end, 10);
        if (end != v.c_str() && *end == '\0' && t > 0 && t <= 600)
            cfg.timeoutSeconds = static_cast<int>(t);
    }
    cfg.forceOffline = isTruthy(valueOf(getenv, "FUSHENGJI_OFFLINE"));
    return cfg;
}
