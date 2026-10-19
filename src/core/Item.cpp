#include "Item.h"

#include "Language.h"

Item::Item(std::string key, int basePrice, int reputationPrice, std::string newsUpKey, std::string newsDownKey)
    : key_(std::move(key)),
      basePrice_(basePrice < 1 ? 1 : basePrice),
      price_(basePrice_),
      reputationPrice_(reputationPrice),
      newsUpKey_(std::move(newsUpKey)),
      newsDownKey_(std::move(newsDownKey)) {}

std::string Item::getName() const {
    return Language::getInstance().getText(key_);
}

std::vector<Item> defaultItems() {
    return {
        Item("IMPORTED_CIGARETTES", 206, 0, "FAKE_CIG_UP", "FAKE_CIG_DOWN"),
        Item("SMUGGLED_CARS", 15338, 0, "USED_CAR_UP", "USED_CAR_DOWN"),
        Item("FAKE_LIQUOR", 2634, -10, "FAKE_WINE_UP", "FAKE_WINE_DOWN"),
        Item("IMPORTED_TOYS", 610, 0, "IMPORT_TOY_UP", "IMPORT_TOY_DOWN"),
        Item("FAKE_COSMETICS", 235, 0, "FAKE_MAKEUP_UP", "FAKE_MAKEUP_DOWN"),
        Item("SHANGHAI_BABY", 1000, 5, "SHANGHAI_BABY_UP", "SHANGHAI_BABY_DOWN"),
        Item("PIRATED_CD", 100, 0, "FAKE_CD_UP", "FAKE_CD_DOWN"),
    };
}
