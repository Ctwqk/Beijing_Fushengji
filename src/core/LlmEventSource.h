#ifndef LLMEVENTSOURCE_H
#define LLMEVENTSOURCE_H

#include <memory>
#include <string>

#include "RandomEvent.h"

// Transport used by LlmEventSource: sends a system + user prompt to a chat
// model and returns the assistant's reply. The libcurl implementation lives in
// src/ai; tests plug in fakes.
class ChatBackend {
public:
    virtual ~ChatBackend() = default;
    virtual std::string complete(const std::string &systemPrompt, const std::string &userPrompt) = 0;
    virtual std::string name() const = 0;
};

// Random events written by a language model. Throws on transport errors and on
// replies that cannot be parsed (wrap it in FallbackEventSource to stay playable).
class LlmEventSource : public RandomEventSource {
public:
    explicit LlmEventSource(std::shared_ptr<ChatBackend> backend) : backend_(std::move(backend)) {}
    RandomEvent generate(const EventContext &ctx) override;
    std::string describe() const override { return "AI (" + backend_->name() + ")"; }

private:
    std::shared_ptr<ChatBackend> backend_;
};

// JSON body of an OpenAI-style chat-completions request.
std::string buildChatRequestBody(const std::string &model, const std::string &systemPrompt, const std::string &userPrompt);
// Extracts choices[0].message.content; throws std::runtime_error with a short
// message for API errors or unexpected shapes.
std::string parseChatResponse(const std::string &body);

#endif
