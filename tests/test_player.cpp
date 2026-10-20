#include <gtest/gtest.h>

#include <memory>

#include "Item.h"
#include "Market.h"
#include "Player.h"

namespace {

// A market where every good always costs exactly its base price and is always on sale.
std::shared_ptr<Market> fixedMarket(std::vector<Item> items) {
    MarketConfig cfg;
    cfg.availability = 1.0;
    cfg.minFactor = cfg.maxFactor = 1.0;
    cfg.newsChance = 0.0;
    return std::make_shared<Market>(std::move(items), 1, cfg);
}

struct Fixture : ::testing::Test {
    Item cd{"CD", 100};
    Item car{"CAR", 15000};
    Item liquor{"LIQUOR", 2000, -10};
    Item book{"BOOK", 1000, 5};
    std::shared_ptr<Market> market = fixedMarket({cd, car, liquor, book});
    Player player{100, 2000, 5000, 0, 100, 100};
    void SetUp() override { player.setMarket(market); }
};

}  // namespace

using PlayerTrade = Fixture;

TEST_F(PlayerTrade, BuyDeductsCashAndSpaceAndStoresTheGoods) {
    player.buy(cd, 5);
    EXPECT_EQ(player.getCash(), 1500);
    EXPECT_EQ(player.getSpace(), 95);
    EXPECT_EQ(player.getItemNumber(cd), 5);
    ASSERT_EQ(player.getHoldings().size(), 1u);
    EXPECT_EQ(player.getHoldings()[0].averageCost(), 100);
}

TEST_F(PlayerTrade, NonPositiveQuantitiesAreRejected) {
    // Regression: a negative quantity used to *add* cash on buy.
    for (int bad : {0, -1, -100}) {
        try {
            player.buy(cd, bad);
            FAIL() << "buy(" << bad << ") should throw";
        } catch (const TradeError &e) {
            EXPECT_EQ(e.reason(), TradeError::Reason::InvalidQuantity);
        }
    }
    EXPECT_EQ(player.getCash(), 2000);
    EXPECT_EQ(player.getSpace(), 100);
    player.buy(cd, 1);
    EXPECT_THROW(player.sell(cd, 0), TradeError);
    EXPECT_THROW(player.sell(cd, -1), TradeError);
}

TEST_F(PlayerTrade, NotEnoughCashAndNotEnoughSpaceReportTheRightReason) {
    try {
        player.buy(car, 1);
        FAIL();
    } catch (const TradeError &e) {
        EXPECT_EQ(e.reason(), TradeError::Reason::NotEnoughCash);
    }
    Player rich(100, 1000000, 0, 0, 100, 10);
    rich.setMarket(market);
    try {
        rich.buy(cd, 11);
        FAIL();
    } catch (const TradeError &e) {
        EXPECT_EQ(e.reason(), TradeError::Reason::NotEnoughSpace);
    }
    EXPECT_EQ(rich.getCash(), 1000000);  // failed trades change nothing
    EXPECT_EQ(rich.getSpace(), 10);
}

TEST_F(PlayerTrade, MaxAffordableIsLimitedByCashAndSpace) {
    EXPECT_EQ(player.maxAffordable(cd), 20);  // 2000 / 100
    EXPECT_EQ(player.maxAffordable(car), 0);
    player.setCash(100000);
    EXPECT_EQ(player.maxAffordable(cd), 100);  // space bound
    player.setSpace(3);
    EXPECT_EQ(player.maxAffordable(cd), 3);
}

TEST_F(PlayerTrade, TradesUseTheMarketPriceNotTheItemCopy) {
    Item stale = cd;
    stale.setPrice(1);  // a UI copy with an outdated price must not be exploitable
    player.buy(stale, 2);
    EXPECT_EQ(player.getCash(), 1800);
}

TEST_F(PlayerTrade, GoodsNotOnSaleCannotBeBought) {
    MarketConfig cfg;
    cfg.availability = 0.0;  // exactly one random good is on sale
    cfg.newsChance = 0.0;
    auto sparse = std::make_shared<Market>(std::vector<Item>{cd, car, liquor, book}, 4, cfg);
    player.setMarket(sparse);
    int rejected = 0;
    for (const Item &i : sparse->getItems()) {
        if (sparse->isAvailable(i))
            continue;
        try {
            player.buy(i, 1);
            FAIL() << i.getKey() << " is not on sale";
        } catch (const TradeError &e) {
            EXPECT_EQ(e.reason(), TradeError::Reason::NotOnSale);
            ++rejected;
        }
    }
    EXPECT_EQ(rejected, 3);
}

TEST_F(PlayerTrade, AverageCostFollowsEveryPurchase) {
    player.setCash(10000);
    player.buy(cd, 10);  // 10 @ 100
    auto pricier = fixedMarket({Item("CD", 200)});
    player.setMarket(pricier);
    player.buy(cd, 10);  // 10 @ 200
    ASSERT_EQ(player.getHoldings().size(), 1u);
    EXPECT_EQ(player.getHoldings()[0].quantity, 20);
    EXPECT_EQ(player.getHoldings()[0].averageCost(), 150);
}

TEST_F(PlayerTrade, SellingKeepsTheAverageCostOfWhatIsLeft) {
    player.setCash(10000);
    player.buy(cd, 10);
    auto pricier = fixedMarket({Item("CD", 200)});
    player.setMarket(pricier);
    player.buy(cd, 10);
    player.sell(cd, 5);
    EXPECT_EQ(player.getHoldings()[0].quantity, 15);
    EXPECT_EQ(player.getHoldings()[0].averageCost(), 150);
}

TEST_F(PlayerTrade, SellPaysTheMarketPriceAndFreesSpace) {
    player.buy(cd, 10);
    auto pricier = fixedMarket({Item("CD", 300)});
    player.setMarket(pricier);
    player.sell(cd, 4);
    EXPECT_EQ(player.getCash(), 1000 + 4 * 300);
    EXPECT_EQ(player.getSpace(), 100 - 10 + 4);
    EXPECT_EQ(player.getItemNumber(cd), 6);
}

TEST_F(PlayerTrade, SellingMoreThanOwnedFailsWithoutSideEffects) {
    player.buy(cd, 3);
    try {
        player.sell(cd, 4);
        FAIL();
    } catch (const TradeError &e) {
        EXPECT_EQ(e.reason(), TradeError::Reason::NotEnoughStock);
    }
    EXPECT_THROW(player.sell(liquor, 1), TradeError);
    EXPECT_EQ(player.getItemNumber(cd), 3);
    EXPECT_EQ(player.getCash(), 1700);
}

TEST_F(PlayerTrade, SoldOutGoodsDisappearFromTheInventoryAndStartFresh) {
    player.buy(cd, 5);
    player.sell(cd, 5);
    EXPECT_TRUE(player.getHoldings().empty());
    auto pricier = fixedMarket({Item("CD", 400)});
    player.setMarket(pricier);
    player.buy(cd, 1);
    // Regression: the old inventory kept the first purchase price forever.
    EXPECT_EQ(player.getHoldings()[0].averageCost(), 400);
}

TEST_F(PlayerTrade, ReputationChangesOncePerSaleAndNeverGoesNegative) {
    player.buy(liquor, 1);
    player.sell(liquor, 1);
    EXPECT_EQ(player.getReputation(), 90);
    player.setReputation(3);
    player.buy(liquor, 1);
    player.sell(liquor, 1);
    EXPECT_EQ(player.getReputation(), 0);
    player.buy(book, 2);
    player.sell(book, 2);
    EXPECT_EQ(player.getReputation(), 5);
}

TEST_F(PlayerTrade, LiquidateSellsEverythingAtMarketPrices) {
    player.setCash(10000);
    player.buy(cd, 10);
    player.buy(book, 3);
    const auto sales = player.liquidate();
    ASSERT_EQ(sales.size(), 2u);
    EXPECT_EQ(sales[0].proceeds, 1000);
    EXPECT_EQ(sales[1].proceeds, 3000);
    EXPECT_TRUE(player.getHoldings().empty());
    EXPECT_EQ(player.getCash(), 10000);
    EXPECT_EQ(player.getSpace(), 100);
}

TEST_F(PlayerTrade, NetWorthCountsStockAtMarketPrice) {
    EXPECT_EQ(player.netWorth(), 2000 - 5000);
    player.buy(cd, 10);
    EXPECT_EQ(player.netWorth(), 2000 - 5000);  // 1000 cash + 1000 stock
    player.setSaving(500);
    EXPECT_EQ(player.netWorth(), 2000 - 5000 + 500);
}

// ---------------------------------------------------------------------------
TEST(PlayerFinance, DebtAndSavingsCompoundDaily) {
    Player p(100, 0, 5000, 1000, 100, 100);
    p.newDay();
    EXPECT_EQ(p.getDebt(), 5250);
    EXPECT_EQ(p.getSaving(), 1040);
    p.newDay();
    EXPECT_EQ(p.getDebt(), 5513);  // 5512.5 rounds half away from zero
    EXPECT_EQ(p.getSaving(), 1082);
    for (int d = 0; d < 38; ++d) p.newDay();
    EXPECT_GT(p.getDebt(), 5000 * 6);  // 1.05^40 is about 7
    EXPECT_LT(p.getDebt(), 5000 * 8);
}

TEST(PlayerFinance, NoDebtMeansNoInterest) {
    Player p(100, 0, 0, 0, 100, 100);
    for (int d = 0; d < 10; ++d) p.newDay();
    EXPECT_EQ(p.getDebt(), 0);
}

TEST(PlayerFinance, DepositAndWithdraw) {
    Player p(100, 1000, 0, 0, 100, 100);
    p.deposit(400);
    EXPECT_EQ(p.getCash(), 600);
    EXPECT_EQ(p.getSaving(), 400);
    p.withdraw(150);
    EXPECT_EQ(p.getCash(), 750);
    EXPECT_EQ(p.getSaving(), 250);
    EXPECT_THROW(p.deposit(751), std::invalid_argument);
    EXPECT_THROW(p.deposit(-1), std::invalid_argument);
    EXPECT_THROW(p.withdraw(251), std::invalid_argument);
    EXPECT_THROW(p.withdraw(-1), std::invalid_argument);
    EXPECT_EQ(p.getCash() + p.getSaving(), 1000);
}

TEST(PlayerFinance, RepayDebt) {
    Player p(100, 1000, 700, 0, 100, 100);
    p.repayDebt(300);
    EXPECT_EQ(p.getCash(), 700);
    EXPECT_EQ(p.getDebt(), 400);
    EXPECT_THROW(p.repayDebt(401), std::invalid_argument);  // more than owed
    EXPECT_THROW(p.repayDebt(701), std::invalid_argument);  // more than cash
    EXPECT_THROW(p.repayDebt(-5), std::invalid_argument);
    p.repayDebt(400);
    EXPECT_EQ(p.getDebt(), 0);
}

TEST(PlayerHealth, HealingCostsMoneyAndStopsAtTheCap) {
    Player p(90, 17500, 0, 0, 100, 100);
    EXPECT_EQ(p.maxHealable(), 10);
    p.heal(4);
    EXPECT_EQ(p.getHealth(), 94);
    EXPECT_EQ(p.getCash(), 10500);  // four health points at the original 1750 yuan each
    EXPECT_EQ(p.maxHealable(), 6);
    EXPECT_THROW(p.heal(7), std::invalid_argument);  // beyond the health cap
    EXPECT_THROW(p.heal(-1), std::invalid_argument);
    EXPECT_EQ(p.getHealth(), 94);
}

TEST(PlayerHealth, HealingNeedsEnoughCash) {
    Player p(50, 1749, 0, 0, 100, 100);
    EXPECT_EQ(p.maxHealable(), 0);
    try {
        p.heal(1);
        FAIL();
    } catch (const TradeError &e) {
        EXPECT_EQ(e.reason(), TradeError::Reason::NotEnoughCash);
    }
    EXPECT_EQ(p.getHealth(), 50);
}

TEST(PlayerHealth, HealthIsClampedToItsRange) {
    Player p(150, 0, 0, 0, 100, 100);
    EXPECT_EQ(p.getHealth(), Player::UPPER_LIMIT);
    p.setHealth(-20);
    EXPECT_EQ(p.getHealth(), 0);
    EXPECT_TRUE(p.isDead());
    p.setHealth(1);
    EXPECT_FALSE(p.isDead());
}

TEST(PlayerEvents, OutcomesAreClamped) {
    // Regression: an event could push health above the cap, which made the
    // hospital pay out money for "negative treatment".
    Player p(95, 300, 0, 0, 20, 100);
    p.applyDelta(-1000, 30, -50);
    EXPECT_EQ(p.getCash(), 0);
    EXPECT_EQ(p.getHealth(), 100);
    EXPECT_EQ(p.getReputation(), 0);
    p.applyDelta(250, -100, 25);
    EXPECT_EQ(p.getCash(), 250);
    EXPECT_TRUE(p.isDead());
    EXPECT_EQ(p.getReputation(), 25);
}

TEST(PlayerStory, KeepsTheBackgroundAndOnlyTheLatestEntries) {
    Player p(100, 0, 0, 0, 100, 100);
    p.setLifeStory("background");
    for (int i = 0; i < 20; ++i) p.recordStory("entry " + std::to_string(i));
    const std::string story = p.getLifeStory();
    EXPECT_EQ(story.rfind("background", 0), 0u);
    EXPECT_EQ(story.find("entry 11"), std::string::npos);
    EXPECT_NE(story.find("entry 12"), std::string::npos);
    EXPECT_NE(story.find("entry 19"), std::string::npos);
}
