#include <gtest/gtest.h>

#include "Events.h"
#include "Game.h"

namespace {

GameConfig config(std::uint32_t seed, int days = 40) {
    GameConfig cfg;
    cfg.seed = seed;
    cfg.totalDays = days;
    return cfg;
}

}  // namespace

TEST(Game, StartsWithTheClassicOpening) {
    Game g(config(1));
    EXPECT_EQ(g.day(), 0);
    EXPECT_EQ(g.totalDays(), 40);
    EXPECT_EQ(g.player()->getCash(), 2000);
    EXPECT_EQ(g.player()->getDebt(), 5000);
    EXPECT_EQ(g.player()->getSaving(), 0);
    EXPECT_EQ(g.player()->getHealth(), 100);
    EXPECT_EQ(g.player()->getSpace(), 100);
    EXPECT_FALSE(g.isOver());
    EXPECT_FALSE(g.market()->getAvailableItems().empty());
}

TEST(Game, TheBackgroundStoryMatchesTheRules) {
    // The story used to promise 5500 yuan at 10% while the code used 5000 at 5%.
    Game g(config(1));
    const std::string story = g.player()->getLifeStory();
    EXPECT_NE(story.find("2000"), std::string::npos);
    EXPECT_NE(story.find("5000"), std::string::npos);
    EXPECT_NE(story.find("5%"), std::string::npos);
    EXPECT_EQ(story.find("5500"), std::string::npos);
    EXPECT_EQ(story.find('{'), std::string::npos) << "unresolved placeholder";
}

TEST(Game, EachDayAccruesInterestAndRerollsTheMarket) {
    Game g(config(2));
    g.player()->setSaving(1000);
    auto before = g.market()->getItems();
    auto report = g.advanceDay();
    EXPECT_EQ(report.day, 1);
    EXPECT_EQ(g.day(), 1);
    EXPECT_EQ(g.player()->getDebt(), 5250);
    EXPECT_EQ(g.player()->getSaving(), 1040);
    bool changed = false;
    for (size_t i = 0; i < before.size(); ++i) changed = changed || before[i].getPrice() != g.market()->getItems()[i].getPrice();
    EXPECT_TRUE(changed);
}

TEST(Game, EndsAfterTheLastDayAndRefusesFurtherMoves) {
    Game g(config(3, 5));
    for (int d = 1; d <= 5; ++d) {
        EXPECT_FALSE(g.isOver());
        auto r = g.advanceDay();
        EXPECT_EQ(r.lastDay, d == 4);
        EXPECT_EQ(r.over, d == 5);
    }
    EXPECT_TRUE(g.isOver());
    EXPECT_THROW(g.advanceDay(), std::logic_error);
}

TEST(Game, RandomEventsFollowTheConfiguredChance) {
    auto count = [](double chance) {
        GameConfig cfg = config(4, 1000);
        cfg.randomEventChance = chance;
        Game g(cfg);
        int due = 0;
        for (int d = 0; d < 999; ++d) due += g.advanceDay().randomEventDue ? 1 : 0;
        return due;
    };
    EXPECT_EQ(count(0.0), 0);
    EXPECT_EQ(count(1.0), 999);
    EXPECT_NEAR(count(0.3) / 999.0, 0.3, 0.05);
}

TEST(Game, NoRandomEventIsDueOnTheFinalDay) {
    GameConfig cfg = config(4, 3);
    cfg.randomEventChance = 1.0;
    Game g(cfg);
    EXPECT_TRUE(g.advanceDay().randomEventDue);
    EXPECT_TRUE(g.advanceDay().randomEventDue);
    EXPECT_FALSE(g.advanceDay().randomEventDue);
}

TEST(Game, SameSeedPlaysTheSameGame) {
    auto trace = [](std::uint32_t seed) {
        Game g(config(seed));
        std::vector<int> t;
        while (!g.isOver()) {
            auto r = g.advanceDay();
            t.push_back(r.randomEventDue);
            t.push_back(static_cast<int>(r.news.size()));
            for (const Item &i : g.market()->getItems()) t.push_back(i.getPrice());
        }
        return t;
    };
    EXPECT_EQ(trace(77), trace(77));
    EXPECT_NE(trace(77), trace(78));
}

TEST(Game, DeathEndsTheGameEarly) {
    Game g(config(5));
    g.advanceDay();
    g.player()->applyDelta(0, -1000, 0);
    EXPECT_TRUE(g.isOver());
    EXPECT_THROW(g.advanceDay(), std::logic_error);
    auto ending = g.finish();
    EXPECT_TRUE(ending.died);
}

TEST(Game, FinishSellsTheStockAndSettlesTheAccounts) {
    Game g(config(6, 3));
    const Item cheap = g.market()->getAvailableItems().front();
    const int n = g.player()->maxAffordable(cheap);
    ASSERT_GT(n, 0);
    g.player()->buy(cheap, n);
    while (!g.isOver()) g.advanceDay();
    const long long expected = g.player()->netWorth();  // already counts the stock
    auto ending = g.finish();
    EXPECT_FALSE(ending.died);
    EXPECT_EQ(ending.netWorth, expected);
    EXPECT_EQ(ending.netWorth, static_cast<long long>(g.player()->getCash()) + g.player()->getSaving() - g.player()->getDebt());
    ASSERT_EQ(ending.sales.size(), 1u);
    EXPECT_EQ(ending.sales[0].quantity, n);
    EXPECT_TRUE(g.player()->getHoldings().empty());
    // Calling it again changes nothing.
    auto again = g.finish();
    EXPECT_TRUE(again.sales.empty());
    EXPECT_EQ(again.netWorth, ending.netWorth);
}

TEST(Game, DoingNothingLosesBecauseOfTheDebt) {
    Game g(config(7));
    while (!g.isOver()) g.advanceDay();
    EXPECT_LT(g.finish().netWorth, 0);
}

TEST(Game, EventContextIsASnapshot) {
    Game g(config(8));
    g.advanceDay();
    EventContext ctx = g.eventContext(Language::Lang::CHINESE);
    EXPECT_EQ(ctx.day, 1);
    EXPECT_EQ(ctx.cash, 2000);
    EXPECT_EQ(ctx.lang, Language::Lang::CHINESE);
    EXPECT_FALSE(ctx.lifeStory.empty());
    g.player()->setCash(5);
    EXPECT_EQ(ctx.cash, 2000);
}

TEST(GameEvents, NewsPopupExplainsThePriceChange) {
    Game g(config(9));
    Market::NewsEvent news{Item("PIRATED_CD", 100, 0, "FAKE_CD_UP", "FAKE_CD_DOWN"), true, "FAKE_CD_UP", 80, 400};
    auto ev = makeNewsEvent(news);
    EXPECT_NE(ev->text.find("80 -> 400"), std::string::npos);
    EXPECT_NE(ev->text.find("pirated CDs"), std::string::npos);
}

TEST(GameEvents, ChoosingAnOptionAppliesItsOutcomeAndWritesTheStory) {
    Game g(config(10));
    RandomEvent re;
    re.title = "Shady Agent";
    re.description = "An agent wants a deposit.";
    re.options = {{"Pay", -400, 0, -10}, {"Argue", -100, -12, 5}, {"Report", 200, -3, 20}};
    auto ev = makeRandomEventPopup(re, g.player());
    ASSERT_EQ(ev->buttons.size(), 3u);
    EXPECT_EQ(ev->buttons[1]->label, "Argue");
    ev->buttons[1]->action();
    EXPECT_EQ(g.player()->getCash(), 1900);
    EXPECT_EQ(g.player()->getHealth(), 88);
    EXPECT_EQ(g.player()->getReputation(), 105);
    EXPECT_NE(g.player()->getLifeStory().find("Argue"), std::string::npos);
}
