#include <gtest/gtest.h>

#include <set>

#include "RandomEvent.h"

namespace {

const char *kValid = R"({
  "title": "Shady Agent",
  "description": "An agent wants a deposit before giving you a job.",
  "options": [
    {"text": "Pay", "cash": -400, "health": 0, "reputation": -10},
    {"text": "Argue", "cash": -100, "health": -12, "reputation": 5},
    {"text": "Report", "cash": 200, "health": -3, "reputation": 20}
  ]
})";

bool hasCjk(const std::string &s) {
    for (unsigned char c : s)
        if (c >= 0xE4 && c <= 0xE9)
            return true;  // lead bytes of the common CJK block
    return false;
}

struct ThrowingSource : RandomEventSource {
    int calls = 0;
    RandomEvent generate(const EventContext &) override {
        ++calls;
        throw std::runtime_error("network down");
    }
    std::string describe() const override { return "broken"; }
};

}  // namespace

// --- parsing -----------------------------------------------------------------
TEST(ParseRandomEvent, AcceptsTheDocumentedShape) {
    RandomEvent ev = parseRandomEvent(kValid);
    EXPECT_EQ(ev.title, "Shady Agent");
    ASSERT_EQ(ev.options.size(), 3u);
    EXPECT_EQ(ev.options[1].text, "Argue");
    EXPECT_EQ(ev.options[1].dCash, -100);
    EXPECT_EQ(ev.options[1].dHealth, -12);
    EXPECT_EQ(ev.options[1].dReputation, 5);
}

TEST(ParseRandomEvent, ToleratesMarkdownFencesAndChatter) {
    const std::string reply = std::string("Sure! Here is your event:\n```json\n") + kValid + "\n```\nHave fun!";
    EXPECT_EQ(parseRandomEvent(reply).options.size(), 3u);
}

TEST(ParseRandomEvent, IgnoresAReasoningModelsThinkBlock) {
    const std::string reply = std::string("<think>maybe {\"title\": 1} is a good start</think>\n") + kValid;
    EXPECT_EQ(parseRandomEvent(reply).title, "Shady Agent");
}

TEST(ParseRandomEvent, AcceptsNumbersWrittenAsStrings) {
    RandomEvent ev = parseRandomEvent(R"({"title":"T","description":"D","options":[
        {"text":"a","cash":"+300","health":"-15","reputation":"10"},
        {"text":"b","cash":"-50元","health":0,"reputation":2.4}]})");
    EXPECT_EQ(ev.options[0].dCash, 300);
    EXPECT_EQ(ev.options[0].dHealth, -15);
    EXPECT_EQ(ev.options[0].dReputation, 10);
    EXPECT_EQ(ev.options[1].dCash, -50);
    EXPECT_EQ(ev.options[1].dReputation, 2);
}

TEST(ParseRandomEvent, MissingNumbersDefaultToZero) {
    RandomEvent ev = parseRandomEvent(R"({"title":"T","description":"D","options":[{"text":"a"},{"text":"b","cash":5}]})");
    EXPECT_EQ(ev.options[0].dCash, 0);
    EXPECT_EQ(ev.options[0].dHealth, 0);
    EXPECT_EQ(ev.options[1].dCash, 5);
}

TEST(ParseRandomEvent, ClampsOutrageousOutcomes) {
    // A model must not be able to hand out a fortune or an instant death.
    RandomEvent ev = parseRandomEvent(R"({"title":"T","description":"D","options":[
        {"text":"a","cash":99999999,"health":-500,"reputation":777},
        {"text":"b","cash":-99999999,"health":500,"reputation":-777}]})");
    EXPECT_EQ(ev.options[0].dCash, EventLimits::MAX_CASH);
    EXPECT_EQ(ev.options[0].dHealth, -EventLimits::MAX_HEALTH);
    EXPECT_EQ(ev.options[0].dReputation, EventLimits::MAX_REPUTATION);
    EXPECT_EQ(ev.options[1].dCash, -EventLimits::MAX_CASH);
    EXPECT_EQ(ev.options[1].dHealth, EventLimits::MAX_HEALTH);
    EXPECT_EQ(ev.options[1].dReputation, -EventLimits::MAX_REPUTATION);
}

TEST(ParseRandomEvent, KeepsAtMostThreeOptions) {
    RandomEvent ev = parseRandomEvent(R"({"title":"T","description":"D","options":[
        {"text":"1"},{"text":"2"},{"text":"3"},{"text":"4"},{"text":"5"}]})");
    EXPECT_EQ(ev.options.size(), 3u);
}

TEST(ParseRandomEvent, RejectsUnusableReplies) {
    EXPECT_THROW(parseRandomEvent(""), std::runtime_error);
    EXPECT_THROW(parseRandomEvent("I cannot do that."), std::runtime_error);
    EXPECT_THROW(parseRandomEvent("{not json}"), std::runtime_error);
    EXPECT_THROW(parseRandomEvent(R"({"title":"T","description":"D"})"), std::runtime_error);
    EXPECT_THROW(parseRandomEvent(R"({"title":"T","description":"D","options":[{"text":"only one"}]})"), std::runtime_error);
    EXPECT_THROW(parseRandomEvent(R"({"title":"","description":"D","options":[{"text":"a"},{"text":"b"}]})"),
                 std::runtime_error);
    EXPECT_THROW(parseRandomEvent(R"({"title":"T","description":"D","options":[{"cash":1},{"text":"b"}]})"),
                 std::runtime_error);
    EXPECT_THROW(parseRandomEvent(R"({"title":"T","description":"D","options":"nope"})"), std::runtime_error);
}

TEST(ParseRandomEvent, TruncatesOverlongTextAtACodePointBoundary) {
    std::string longText;
    for (int i = 0; i < 400; ++i) longText += "汉";
    const std::string reply = R"({"title":"T","description":")" + longText + R"(","options":[{"text":"a"},{"text":"b"}]})";
    RandomEvent ev = parseRandomEvent(reply);
    EXPECT_LE(ev.description.size(), 600u);
    EXPECT_EQ(ev.description.size() % 3, 0u) << "cut in the middle of a UTF-8 sequence";
}

// --- offline source ------------------------------------------------------------
TEST(OfflineEvents, EveryEventIsWellFormedInBothLanguages) {
    for (auto lang : {Language::Lang::ENGLISH, Language::Lang::CHINESE}) {
        OfflineEventSource source(1);
        EventContext ctx;
        ctx.lang = lang;
        std::set<std::string> titles;
        for (int i = 0; i < 500; ++i) {
            RandomEvent ev = source.generate(ctx);
            titles.insert(ev.title);
            EXPECT_FALSE(ev.title.empty());
            EXPECT_FALSE(ev.description.empty());
            ASSERT_EQ(ev.options.size(), 3u);
            EXPECT_EQ(hasCjk(ev.title), lang == Language::Lang::CHINESE) << ev.title;
            for (const auto &o : ev.options) {
                EXPECT_FALSE(o.text.empty());
                EXPECT_LE(std::abs(o.dCash), EventLimits::MAX_CASH);
                EXPECT_LE(std::abs(o.dHealth), EventLimits::MAX_HEALTH);
                EXPECT_LE(std::abs(o.dReputation), EventLimits::MAX_REPUTATION);
            }
            // No option is a free lunch: at least one dimension gets worse or costs something.
            bool allGood = true;
            for (const auto &o : ev.options) allGood = allGood && o.dCash >= 0 && o.dHealth >= 0 && o.dReputation >= 0;
            EXPECT_FALSE(allGood) << ev.title;
        }
        EXPECT_EQ(titles.size(), OfflineEventSource::eventCount()) << "every event should show up eventually";
    }
}

TEST(OfflineEvents, AreDeterministicForASeed) {
    EventContext ctx;
    auto run = [&](std::uint32_t seed) {
        OfflineEventSource s(seed);
        std::vector<std::string> titles;
        for (int i = 0; i < 30; ++i) titles.push_back(s.generate(ctx).title);
        return titles;
    };
    EXPECT_EQ(run(5), run(5));
    EXPECT_NE(run(5), run(6));
}

// --- fallback ------------------------------------------------------------------
TEST(FallbackEventSource, UsesTheOfflineEventWhenTheLlmFails) {
    auto broken = std::make_shared<ThrowingSource>();
    FallbackEventSource source(broken, std::make_shared<OfflineEventSource>(3));
    testing::internal::CaptureStderr();
    RandomEvent ev = source.generate(EventContext{});
    const std::string log = testing::internal::GetCapturedStderr();
    EXPECT_EQ(broken->calls, 1);
    EXPECT_EQ(ev.options.size(), 3u);
    EXPECT_NE(log.find("network down"), std::string::npos);
}

// --- prompts -------------------------------------------------------------------
TEST(Prompts, UserPromptCarriesTheStatusAndTheStory) {
    EventContext ctx;
    ctx.day = 12;
    ctx.cash = 1234;
    ctx.health = 77;
    ctx.reputation = 55;
    ctx.lifeStory = "A young trader arrives in Beijing.";
    const std::string p = buildEventUserPrompt(ctx);
    EXPECT_NE(p.find("Day 12"), std::string::npos);
    EXPECT_NE(p.find("Cash 1234"), std::string::npos);
    EXPECT_NE(p.find("Health 77"), std::string::npos);
    EXPECT_NE(p.find("Reputation 55"), std::string::npos);
    EXPECT_NE(p.find("A young trader"), std::string::npos);

    ctx.lang = Language::Lang::CHINESE;
    const std::string zh = buildEventUserPrompt(ctx);
    EXPECT_NE(zh.find("现金 1234"), std::string::npos);
}

TEST(Prompts, SystemPromptDemandsJsonInTheRightLanguage) {
    const std::string en = buildEventSystemPrompt(Language::Lang::ENGLISH);
    const std::string zh = buildEventSystemPrompt(Language::Lang::CHINESE);
    for (const std::string &p : {en, zh}) {
        EXPECT_NE(p.find("JSON"), std::string::npos);
        EXPECT_NE(p.find("\"options\""), std::string::npos);
        EXPECT_NE(p.find("-800"), std::string::npos);
    }
    EXPECT_FALSE(hasCjk(en));
    EXPECT_TRUE(hasCjk(zh));
}
