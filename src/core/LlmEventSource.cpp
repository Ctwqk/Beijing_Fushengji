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
    body["messages"] = nlohmann::json::array({{{"role", "system"}, {"content", systemPrompt}},
                                              {{"role", "user"}, {"content", userPrompt}}});
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
        std::string msg = "API error";
        const auto &err = j["error"];
        if (err.is_object() && err.contains("message") && err["message"].is_string()) msg += ": " + err["message"].get<std::string>();
        else if (err.is_string()) msg += ": " + err.get<std::string>();
        if (msg.size() > 300) msg.resize(300);
        throw std::runtime_error(msg);
    }
    try {
        const auto &content = j.at("choices").at(0).at("message").at("content");
        if (!content.is_string()) throw std::runtime_error("message content is not a string");
        return content.get<std::string>();
    } catch (const nlohmann::json::exception &) {
        throw std::runtime_error("response has no choices[0].message.content");
    }
}
