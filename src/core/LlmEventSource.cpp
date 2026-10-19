#include "LlmEventSource.h"

#include <stdexcept>

#include <nlohmann/json.hpp>

RandomEvent LlmEventSource::generate(const EventContext &ctx) {
    const std::string reply = backend_->complete(buildEventSystemPrompt(ctx.lang), buildEventUserPrompt(ctx));
    return parseRandomEvent(reply);
}

std::string buildChatRequestBody(const std::string &model, const std::string &systemPrompt, const std::string &userPrompt) {
    nlohmann::json body;
    body["model"] = model;
    body["messages"] = nlohmann::json::array(
        {{{"role", "system"}, {"content", systemPrompt}}, {{"role", "user"}, {"content", userPrompt}}});
    return body.dump();
}

std::string parseChatResponse(const std::string &text) {
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(text);
    } catch (const std::exception &) {
        throw std::runtime_error("response is not valid JSON");
    }
    if (j.is_object() && j.contains("error")) {
        // A provider may echo rejected credentials in its error body. Keep it
        // out of exceptions and logs; the transport still supplies HTTP status.
        throw std::runtime_error("API error (provider details omitted)");
    }
    try {
        const auto &content = j.at("choices").at(0).at("message").at("content");
        if (!content.is_string())
            throw std::runtime_error("message content is not a string");
        return content.get<std::string>();
    } catch (const nlohmann::json::exception &) {
        throw std::runtime_error("response has no choices[0].message.content");
    }
}
