// Exercises the libcurl client against tests/mock_openai_server.py (started by
// run_with_mock_server.py, which exports FSJ_MOCK_URL).
#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <future>
#include <thread>

#include <nlohmann/json.hpp>

#include "CurlChatClient.h"
#include "EventSourceFactory.h"

namespace {

std::string mockUrl() {
    const char *u = std::getenv("FSJ_MOCK_URL");
    return u ? u : "";
}

AiConfig configFor(const std::string &mode, const std::string &key = "test-key") {
    AiConfig cfg;
    cfg.baseUrl = mockUrl() + "/" + mode + "/v1";
    cfg.baseUrlFromEnv = true;
    cfg.apiKey = key;
    cfg.model = "mock-model";
    cfg.timeoutSeconds = 10;
    return cfg;
}

#define REQUIRE_MOCK()     \
    if (mockUrl().empty()) \
    GTEST_SKIP() << "FSJ_MOCK_URL is not set (run through run_with_mock_server.py)"

}  // namespace

TEST(CurlChatClient, SendsAnAuthenticatedOpenAiRequest) {
    REQUIRE_MOCK();
    CurlChatClient client(configFor("echo", "test-key"));
    const auto echoed = nlohmann::json::parse(client.complete("system text", "user text"));
    EXPECT_EQ(echoed.at("authorization"), "Bearer test-key");
    EXPECT_EQ(echoed.at("content_type"), "application/json");
    EXPECT_EQ(echoed.at("body").at("model"), "mock-model");
    EXPECT_EQ(echoed.at("body").at("messages")[0].at("content"), "system text");
    EXPECT_EQ(echoed.at("body").at("messages")[1].at("content"), "user text");
}

TEST(CurlChatClient, OmitsTheAuthorizationHeaderWithoutAKey) {
    REQUIRE_MOCK();
    CurlChatClient client(configFor("echo", ""));
    const auto echoed = nlohmann::json::parse(client.complete("s", "u"));
    EXPECT_TRUE(echoed.at("authorization").is_null());
}

TEST(CurlChatClient, ReportsApiErrorsWithoutLeakingTheKey) {
    REQUIRE_MOCK();
    CurlChatClient client(configFor("unauthorized", "super-secret-key"));
    try {
        client.complete("s", "u");
        FAIL();
    } catch (const std::runtime_error &e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("401"), std::string::npos) << msg;
        EXPECT_NE(msg.find("API error"), std::string::npos) << msg;
        EXPECT_EQ(msg.find("super-secret-key"), std::string::npos);
    }
}

TEST(EventSourceFactory, DoesNotLogCredentialsEchoedInProviderErrors) {
    REQUIRE_MOCK();
    for (const char *mode : {"unauthorized", "error200"}) {
        EventSourceSetup setup = makeEventSource(configFor(mode, "test-rejected-credential"), 1);
        testing::internal::CaptureStderr();
        const RandomEvent ev = setup.source->generate(EventContext{});
        const std::string log = testing::internal::GetCapturedStderr();
        EXPECT_EQ(ev.options.size(), 3u);
        EXPECT_NE(log.find("offline"), std::string::npos);
        EXPECT_EQ(log.find("test-rejected-credential"), std::string::npos);
    }
}

TEST(CurlChatClient, ReportsNonJsonGatewayErrors) {
    REQUIRE_MOCK();
    CurlChatClient client(configFor("badgateway"));
    try {
        client.complete("s", "u");
        FAIL();
    } catch (const std::runtime_error &e) {
        EXPECT_NE(std::string(e.what()).find("502"), std::string::npos) << e.what();
    }
}

TEST(CurlChatClient, TimesOut) {
    REQUIRE_MOCK();
    AiConfig cfg = configFor("slow");
    cfg.timeoutSeconds = 1;
    CurlChatClient client(cfg);
    const auto start = std::chrono::steady_clock::now();
    EXPECT_THROW(client.complete("s", "u"), std::runtime_error);
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(4));
}

TEST(CurlChatClient, CancelAbortsARunningRequest) {
    REQUIRE_MOCK();
    CurlChatClient client(configFor("slow"));
    auto result = std::async(std::launch::async, [&] {
        try {
            client.complete("s", "u");
            return false;
        } catch (const std::runtime_error &) {
            return true;
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    const auto start = std::chrono::steady_clock::now();
    client.cancel();
    EXPECT_TRUE(result.get());
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(3));
}

TEST(CurlChatClient, ConnectionRefusedIsAnError) {
    AiConfig cfg;
    cfg.baseUrl = "http://127.0.0.1:9/v1";  // discard port, nothing listens
    cfg.baseUrlFromEnv = true;
    cfg.timeoutSeconds = 5;
    CurlChatClient client(cfg);
    EXPECT_THROW(client.complete("s", "u"), std::runtime_error);
}

TEST(EventSourceFactory, UsesTheLlmWhenConfigured) {
    REQUIRE_MOCK();
    EventSourceSetup setup = makeEventSource(configFor("ok"), 1);
    EXPECT_TRUE(setup.ai);
    EXPECT_EQ(setup.description, "AI (mock-model)");
    RandomEvent ev = setup.source->generate(EventContext{});
    EXPECT_EQ(ev.title, "Mock Event");  // parsed out of a fenced JSON reply
    ASSERT_EQ(ev.options.size(), 3u);
    EXPECT_EQ(ev.options[0].dCash, 300);
}

TEST(EventSourceFactory, FallsBackToOfflineEventsWhenTheLlmMisbehaves) {
    REQUIRE_MOCK();
    for (const char *mode : {"garbage", "unauthorized", "badgateway"}) {
        EventSourceSetup setup = makeEventSource(configFor(mode), 1);
        testing::internal::CaptureStderr();
        RandomEvent ev = setup.source->generate(EventContext{});
        testing::internal::GetCapturedStderr();
        EXPECT_EQ(ev.options.size(), 3u) << mode;
        EXPECT_NE(ev.title, "Mock Event") << mode;
    }
}

TEST(EventSourceFactory, StaysOfflineWithoutConfiguration) {
    AiConfig cfg;  // no key, default URL
    EventSourceSetup setup = makeEventSource(cfg, 1);
    EXPECT_FALSE(setup.ai);
    EXPECT_EQ(setup.description, "offline");
    EXPECT_EQ(setup.source->generate(EventContext{}).options.size(), 3u);

    AiConfig forced = configFor("ok");
    forced.forceOffline = true;
    EXPECT_FALSE(makeEventSource(forced, 1).ai);
}
