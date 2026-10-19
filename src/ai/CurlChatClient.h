#ifndef CURLCHATCLIENT_H
#define CURLCHATCLIENT_H

#include <atomic>
#include <memory>
#include <string>

#include "AiConfig.h"
#include "LlmEventSource.h"

// ChatBackend that talks to any OpenAI-compatible /chat/completions endpoint
// over libcurl. Every call uses its own easy handle, so it is safe to call from
// worker threads, and cancel() aborts requests that are still in flight.
class CurlChatClient : public ChatBackend {
public:
    explicit CurlChatClient(AiConfig config);
    std::string complete(const std::string &systemPrompt, const std::string &userPrompt) override;
    std::string name() const override { return config_.model; }
    void cancel() { cancelled_ = true; }

private:
    AiConfig config_;
    std::atomic<bool> cancelled_{false};
};

#endif
