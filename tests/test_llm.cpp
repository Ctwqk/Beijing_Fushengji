#include <gtest/gtest.h>

#include <map>

#include <nlohmann/json.hpp>

#include "AiConfig.h"
#include "LlmEventSource.h"

namespace {

struct FakeBackend : ChatBackend {
    std::string reply;
    std::string lastSystem, lastUser;
    bool fail = false;
    std::string complete(const std::string &s, const std::string &u) override {
        lastSystem = s;
        lastUser = u;
        if (fail) throw std::runtime_error("HTTP 401");
        return reply;
    }
    std::string name() const override { return "fake-model"; }
};

AiConfig fromMap(const std::map<std::string, std::string> &env) {
    return AiConfig::fromEnv([&](const char *name) -> const char * {
        auto it = env.find(name);
        return it == env.end() ? nullptr : it->second.c_str();
    });
}

}  // namespace

TEST(LlmEventSource, TurnsTheModelReplyIntoAnEvent) {
    auto backend = std::make_shared<FakeBackend>();
    backend->reply = R"({"title":"T","description":"D","options":[{"text":"a","cash":10},{"text":"b","cash":-10}]})";
    LlmEventSource source(backend);
    EventContext ctx;
    ctx.lifeStory = "story";
    RandomEvent ev = source.generate(ctx);
    EXPECT_EQ(ev.title, "T");
    EXPECT_EQ(ev.options.size(), 2u);
    EXPECT_NE(backend->lastSystem.find("JSON"), std::string::npos);
    EXPECT_NE(backend->lastUser.find("story"), std::string::npos);
    EXPECT_EQ(source.describe(), "AI (fake-model)");
}

TEST(LlmEventSource, GarbageRepliesThrowSoTheFallbackKicksIn) {
    auto backend = std::make_shared<FakeBackend>();
    backend->reply = "Sorry, I can't help with that.";
    LlmEventSource llm(backend);
    EXPECT_THROW(llm.generate(EventContext{}), std::runtime_error);

    FallbackEventSource safe(std::make_shared<LlmEventSource>(backend), std::make_shared<OfflineEventSource>(1));
    testing::internal::CaptureStderr();
    EXPECT_EQ(safe.generate(EventContext{}).options.size(), 3u);
    testing::internal::GetCapturedStderr();

    backend->fail = true;
    testing::internal::CaptureStderr();
    EXPECT_EQ(safe.generate(EventContext{}).options.size(), 3u);
    testing::internal::GetCapturedStderr();
}

TEST(ChatProtocol, RequestBodyIsAnOpenAiChatCompletionRequest) {
    const auto j = nlohmann::json::parse(buildChatRequestBody("gpt-x", "be brief", "say \"hi\"\nplease"));
    EXPECT_EQ(j.at("model"), "gpt-x");
    ASSERT_EQ(j.at("messages").size(), 2u);
    EXPECT_EQ(j.at("messages")[0].at("role"), "system");
    EXPECT_EQ(j.at("messages")[0].at("content"), "be brief");
    EXPECT_EQ(j.at("messages")[1].at("role"), "user");
    EXPECT_EQ(j.at("messages")[1].at("content"), "say \"hi\"\nplease");  // survives JSON escaping
}

TEST(ChatProtocol, ResponseParsing) {
    EXPECT_EQ(parseChatResponse(R"({"choices":[{"message":{"role":"assistant","content":"hello"}}]})"), "hello");
    EXPECT_THROW(parseChatResponse("<html>502 Bad Gateway</html>"), std::runtime_error);
    EXPECT_THROW(parseChatResponse(R"({"choices":[]})"), std::runtime_error);
    EXPECT_THROW(parseChatResponse(R"({"choices":[{"message":{"content":null}}]})"), std::runtime_error);
    try {
        parseChatResponse(R"({"error":{"message":"Incorrect API key provided","type":"invalid_request_error"}})");
        FAIL();
    } catch (const std::runtime_error &e) {
        EXPECT_NE(std::string(e.what()).find("Incorrect API key"), std::string::npos);
    }
}

// --- configuration -------------------------------------------------------------
TEST(AiConfig, WithoutAnyVariablesTheGameStaysOffline) {
    AiConfig cfg = fromMap({});
    EXPECT_FALSE(cfg.enabled());
    EXPECT_EQ(cfg.model, "gpt-4o-mini");
    EXPECT_EQ(cfg.chatCompletionsUrl(), "https://api.openai.com/v1/chat/completions");
    EXPECT_TRUE(cfg.apiKey.empty());
}

TEST(AiConfig, ApiKeyEnablesTheLlm) {
    AiConfig cfg = fromMap({{"FUSHENGJI_AI_API_KEY", "  sk-test  "}});
    EXPECT_TRUE(cfg.enabled());
    EXPECT_EQ(cfg.apiKey, "sk-test");
}

TEST(AiConfig, CustomBaseUrlWorksWithoutAKeyForLocalServers) {
    AiConfig cfg = fromMap({{"FUSHENGJI_AI_BASE_URL", "http://localhost:11434/v1/"}, {"FUSHENGJI_AI_MODEL", "llama3"}});
    EXPECT_TRUE(cfg.enabled());
    EXPECT_EQ(cfg.model, "llama3");
    EXPECT_EQ(cfg.chatCompletionsUrl(), "http://localhost:11434/v1/chat/completions");
}

TEST(AiConfig, FullEndpointUrlsAreAcceptedAsIs) {
    AiConfig cfg = fromMap({{"FUSHENGJI_AI_BASE_URL", "https://api.groq.com/openai/v1/chat/completions"}});
    EXPECT_EQ(cfg.chatCompletionsUrl(), "https://api.groq.com/openai/v1/chat/completions");
}

TEST(AiConfig, OfflineSwitchWinsOverEverything) {
    for (const char *v : {"1", "true", "TRUE", "yes", "on"}) {
        AiConfig cfg = fromMap({{"FUSHENGJI_AI_API_KEY", "k"}, {"FUSHENGJI_OFFLINE", v}});
        EXPECT_FALSE(cfg.enabled()) << v;
    }
    EXPECT_TRUE(fromMap({{"FUSHENGJI_AI_API_KEY", "k"}, {"FUSHENGJI_OFFLINE", "0"}}).enabled());
}

TEST(AiConfig, TimeoutIsValidated) {
    EXPECT_EQ(fromMap({{"FUSHENGJI_AI_TIMEOUT", "45"}}).timeoutSeconds, 45);
    EXPECT_EQ(fromMap({{"FUSHENGJI_AI_TIMEOUT", "abc"}}).timeoutSeconds, 20);
    EXPECT_EQ(fromMap({{"FUSHENGJI_AI_TIMEOUT", "-3"}}).timeoutSeconds, 20);
    EXPECT_EQ(fromMap({{"FUSHENGJI_AI_TIMEOUT", "99999"}}).timeoutSeconds, 20);
}
