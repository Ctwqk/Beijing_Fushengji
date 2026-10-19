#include "CurlChatClient.h"

#include <curl/curl.h>

#include <mutex>
#include <stdexcept>

namespace {

void ensureCurlInitialised() {
    static std::once_flag flag;
    std::call_once(flag, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    auto *buffer = static_cast<std::string *>(userp);
    const size_t total = size * nmemb;
    // Refuse absurdly large bodies instead of growing without bound.
    if (buffer->size() + total > (4u << 20))
        return 0;
    buffer->append(static_cast<char *>(contents), total);
    return total;
}

int progressCallback(void *userp, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    return static_cast<std::atomic<bool> *>(userp)->load() ? 1 : 0;
}

struct CurlHandle {
    CURL *easy = curl_easy_init();
    curl_slist *headers = nullptr;
    ~CurlHandle() {
        curl_slist_free_all(headers);
        if (easy)
            curl_easy_cleanup(easy);
    }
};

}  // namespace

CurlChatClient::CurlChatClient(AiConfig config) : config_(std::move(config)) {
    ensureCurlInitialised();
}

std::string CurlChatClient::complete(const std::string &systemPrompt, const std::string &userPrompt) {
    if (cancelled_)
        throw std::runtime_error("request cancelled");
    CurlHandle h;
    if (!h.easy)
        throw std::runtime_error("curl_easy_init failed");

    const std::string url = config_.chatCompletionsUrl();
    const std::string payload = buildChatRequestBody(config_.model, systemPrompt, userPrompt);
    std::string response;
    char errorBuffer[CURL_ERROR_SIZE] = {0};

    h.headers = curl_slist_append(h.headers, "Content-Type: application/json");
    if (!config_.apiKey.empty()) {
        const std::string auth = "Authorization: Bearer " + config_.apiKey;
        h.headers = curl_slist_append(h.headers, auth.c_str());
    }

    curl_easy_setopt(h.easy, CURLOPT_URL, url.c_str());
    curl_easy_setopt(h.easy, CURLOPT_HTTPHEADER, h.headers);
    curl_easy_setopt(h.easy, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(h.easy, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    curl_easy_setopt(h.easy, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(h.easy, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(h.easy, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(h.easy, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(h.easy, CURLOPT_TIMEOUT, static_cast<long>(config_.timeoutSeconds));
    curl_easy_setopt(h.easy, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(h.easy, CURLOPT_USERAGENT, "beijing-fushengji/1.0");
    curl_easy_setopt(h.easy, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(h.easy, CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(h.easy, CURLOPT_XFERINFODATA, &cancelled_);

    const CURLcode rc = curl_easy_perform(h.easy);
    if (rc != CURLE_OK) {
        const std::string detail = errorBuffer[0] ? errorBuffer : curl_easy_strerror(rc);
        throw std::runtime_error("HTTP request failed: " + detail);
    }
    long status = 0;
    curl_easy_getinfo(h.easy, CURLINFO_RESPONSE_CODE, &status);
    if (status < 200 || status >= 300) {
        // Prefer the API's own error message, fall back to the bare status code.
        try {
            parseChatResponse(response);
        } catch (const std::runtime_error &e) {
            throw std::runtime_error("HTTP " + std::to_string(status) + " (" + e.what() + ")");
        }
        throw std::runtime_error("HTTP status " + std::to_string(status));
    }
    return parseChatResponse(response);
}
