#include <gtest/gtest.h>

#include <set>
#include <unordered_set>

#include "Item.h"
#include "Language.h"
#include "Market.h"

namespace {

MarketConfig quietConfig() {
    MarketConfig cfg;
    cfg.newsChance = 0.0;
    return cfg;
}

}  // namespace

TEST(Item, IdentityIsTheKeyNotThePrice) {
    Item a("CD", 100);
    Item b("CD", 100);
    b.setPrice(999);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, Item("CAR", 100));
    std::unordered_set<Item> set{a, b};
    EXPECT_EQ(set.size(), 1u);
}

TEST(Item, PriceNeverDropsBelowOne) {
    Item a("CD", 100);
    a.setPrice(0);
    EXPECT_EQ(a.getPrice(), 1);
    a.setPrice(-50);
    EXPECT_EQ(a.getPrice(), 1);
    EXPECT_EQ(Item("FREE", 0).getBasePrice(), 1);  // a zero baseline would divide by zero in calPossibleNumber
}

TEST(Item, CalPossibleNumber) {
    Item a("CD", 100);
    EXPECT_EQ(a.calPossibleNumber(1050), 10);
    EXPECT_EQ(a.calPossibleNumber(99), 0);
    EXPECT_EQ(a.calPossibleNumber(-5), 0);
}

TEST(Item, NameIsLocalised) {
    Item cigarettes("IMPORTED_CIGARETTES", 206);
    LANG.setLanguage(Language::Lang::CHINESE);
    EXPECT_EQ(cigarettes.getName(), "进口香烟");
    LANG.setLanguage(Language::Lang::ENGLISH);
    EXPECT_EQ(cigarettes.getName(), "Imported Cigarettes");
    EXPECT_EQ(Item("UNKNOWN_KEY", 5).getName(), "UNKNOWN_KEY");
}

TEST(Market, PricesStayInsideTheBandForever) {
    // Regression: prices used to be multiplied by 0.5..1.5 *cumulatively*, a random
    // walk with negative drift that eventually produced a price of 0 (division by zero).
    Market m({Item("A", 100), Item("B", 15338), Item("C", 3)}, 7, quietConfig());
    for (int day = 0; day < 5000; ++day) {
        m.newDay();
        for (const Item &i : m.getItems()) {
            EXPECT_GE(i.getPrice(), 1);
            EXPECT_GE(i.getPrice(), i.getBasePrice() / 2) << i.getKey();
            EXPECT_LE(i.getPrice(), i.getBasePrice() * 3 / 2 + 1) << i.getKey();
        }
    }
}

TEST(Market, PricesActuallyVary) {
    Market m({Item("A", 1000)}, 11, quietConfig());
    std::set<int> seen;
    for (int day = 0; day < 200; ++day) {
        m.newDay();
        seen.insert(m.askPrice(Item("A", 1)));
    }
    EXPECT_GT(seen.size(), 50u);
}

TEST(Market, SameSeedReplaysTheSameMarket) {
    auto run = [](std::uint32_t seed) {
        Market m(defaultItems(), seed);
        std::vector<int> trace;
        for (int day = 0; day < 60; ++day) {
            m.newDay();
            for (const Item &i : m.getItems()) trace.push_back(m.isAvailable(i) ? i.getPrice() : -i.getPrice());
        }
        return trace;
    };
    EXPECT_EQ(run(123), run(123));
    EXPECT_NE(run(123), run(124));
}

TEST(Market, AvailabilityRoughlyMatchesConfig) {
    MarketConfig cfg = quietConfig();
    cfg.availability = 0.6;
    Market m(defaultItems(), 99, cfg);
    int listed = 0;
    const int days = 2000;
    for (int d = 0; d < days; ++d) {
        m.newDay();
        listed += static_cast<int>(m.getAvailableItems().size());
    }
    const double rate = static_cast<double>(listed) / (days * static_cast<int>(m.getItems().size()));
    EXPECT_NEAR(rate, 0.6, 0.03);
}

TEST(Market, NeverOpensEmpty) {
    MarketConfig cfg = quietConfig();
    cfg.availability = 0.0;
    Market m(defaultItems(), 5, cfg);
    for (int d = 0; d < 100; ++d) {
        m.newDay();
        EXPECT_EQ(m.getAvailableItems().size(), 1u);
    }
}

TEST(Market, NewsMultipliesOrDividesThePrice) {
    MarketConfig cfg;
    cfg.availability = 1.0;
    cfg.newsChance = 1.0;
    Market m(defaultItems(), 3, cfg);
    int ups = 0, downs = 0;
    for (int d = 0; d < 50; ++d) {
        auto news = m.newDay();
        ASSERT_EQ(news.size(), m.getItems().size());
        for (const auto &n : news) {
            EXPECT_EQ(n.newPrice, m.askPrice(n.item));
            if (n.priceUp) {
                EXPECT_EQ(n.newPrice, n.oldPrice * 5);
                EXPECT_EQ(n.textKey, n.item.getNewsUpKey());
                ++ups;
            } else {
                EXPECT_EQ(n.newPrice, std::max(1, n.oldPrice / 5));
                EXPECT_EQ(n.textKey, n.item.getNewsDownKey());
                ++downs;
            }
        }
    }
    EXPECT_GT(ups, 0);
    EXPECT_GT(downs, 0);
}

TEST(Market, NewsOnlyHitsGoodsOnSale) {
    MarketConfig cfg;
    cfg.availability = 0.3;
    cfg.newsChance = 1.0;
    Market m(defaultItems(), 8, cfg);
    for (int d = 0; d < 100; ++d) {
        auto news = m.newDay();
        for (const auto &n : news) EXPECT_TRUE(m.isAvailable(n.item));
    }
}

TEST(Market, UnknownGoodsAreRejected) {
    Market m({Item("A", 100)}, 1, quietConfig());
    EXPECT_THROW(m.askPrice(Item("B", 5)), std::runtime_error);
    EXPECT_FALSE(m.isAvailable(Item("B", 5)));
}

TEST(Market, DefaultCatalogueIsConsistent) {
    const auto items = defaultItems();
    EXPECT_EQ(items.size(), 7u);
    std::set<std::string> keys;
    for (const Item &i : items) {
        EXPECT_TRUE(keys.insert(i.getKey()).second) << "duplicate key " << i.getKey();
        EXPECT_GT(i.getBasePrice(), 0);
        EXPECT_FALSE(i.getNewsUpKey().empty());
        EXPECT_FALSE(i.getNewsDownKey().empty());
    }
}
