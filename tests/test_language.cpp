#include <gtest/gtest.h>

#include "Item.h"
#include "Language.h"

TEST(Language, SubstituteReplacesPlaceholders) {
    EXPECT_EQ(Language::substitute("a {0} b {1} {0}", {"x", "y"}), "a x b y x");
    EXPECT_EQ(Language::substitute("no placeholders", {"x"}), "no placeholders");
    EXPECT_EQ(Language::substitute("{2} stays", {"x"}), "{2} stays");
    EXPECT_EQ(Language::substitute("{} {x} {", {"x"}), "{} {x} {");
}

TEST(Language, SubstitutedTextIsNeverRescanned) {
    EXPECT_EQ(Language::substitute("{0} {1}", {"{1}", "z"}), "{1} z");
}

TEST(Language, FormatUsesTheCurrentLanguage) {
    LANG.setLanguage(Language::Lang::ENGLISH);
    EXPECT_EQ(LANG.format("DAY_COUNTER", {"3", "40"}), "Day 3/40");
    LANG.setLanguage(Language::Lang::CHINESE);
    EXPECT_EQ(LANG.format("DAY_COUNTER", {"3", "40"}), "第3/40天");
    LANG.setLanguage(Language::Lang::ENGLISH);
}

TEST(Language, UnknownKeysFallBackToTheKey) {
    EXPECT_EQ(GET_TEXT("NO_SUCH_KEY"), "NO_SUCH_KEY");
}

TEST(Language, EveryKeyTheGameUsesIsTranslatedBothWays) {
    std::vector<std::string> keys = {"GAME_TITLE",
                                     "BLACK_MARKET",
                                     "GOODS",
                                     "PRICE",
                                     "YOUR_ROOM",
                                     "STATUS",
                                     "CASH",
                                     "SAVINGS",
                                     "DEBT",
                                     "HEALTH",
                                     "REPUTATION",
                                     "BANK",
                                     "HOSPITAL",
                                     "POST_OFFICE",
                                     "RENTAL_AGENCY",
                                     "INTERNET_CAFE",
                                     "CONFIRM",
                                     "CANCEL",
                                     "BUY",
                                     "SELL",
                                     "NEWS_EVENT",
                                     "ENDING",
                                     "FINAL_PROFIT",
                                     "FINAL_DEBT",
                                     "DEAD_ENDING",
                                     "SOLD_REMAINING",
                                     "DAY_COUNTER",
                                     "LIFE_BACKGROUND",
                                     "AI_SYSTEM_PROMPT",
                                     "NOT_ENOUGH_MONEY",
                                     "NOT_ENOUGH_SPACE",
                                     "NOT_ENOUGH_STOCK",
                                     "NOT_ON_SALE",
                                     "SELECT_FIRST",
                                     "HEALTHY",
                                     "NO_DEBT"};
    for (int i = 0; i < 10; ++i) {
        keys.push_back(std::string("LOC_") + std::vector<std::string>{"HAIDIAN", "YAYUN", "SANYUAN", "BAJIAO", "CUIWEI",
                                                                      "FUYOU", "YONGANLI", "YUQUAN", "YONGDINGMEN",
                                                                      "FANGZHUANG"}[static_cast<size_t>(i)]);
    }
    for (const Item &item : defaultItems()) {
        keys.push_back(item.getKey());
        keys.push_back(item.getNewsUpKey());
        keys.push_back(item.getNewsDownKey());
    }
    for (const std::string &key : keys) {
        const std::string zh = LANG.getText(key, Language::Lang::CHINESE);
        const std::string en = LANG.getText(key, Language::Lang::ENGLISH);
        EXPECT_NE(zh, key) << "missing Chinese text for " << key;
        EXPECT_NE(en, key) << "missing English text for " << key;
        EXPECT_NE(zh, en) << "untranslated: " << key;
    }
}
