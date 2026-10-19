#ifndef ITEM_H
#define ITEM_H

#include <functional>
#include <string>
#include <vector>

// A tradeable good. `key` is a stable identifier that doubles as the key of the
// localisation table (so the display name follows the selected language);
// two items are the same good when their keys match, whatever their price.
class Item {
public:
    Item() = default;
    Item(std::string key, int basePrice, int reputationPrice = 0, std::string newsUpKey = "", std::string newsDownKey = "");

    const std::string &getKey() const { return key_; }
    std::string getName() const;  // localised
    int getPrice() const { return price_; }
    void setPrice(int p) { price_ = p < 1 ? 1 : p; }
    int getBasePrice() const { return basePrice_; }
    int getReputationPrice() const { return reputationPrice_; }
    const std::string &getNewsUpKey() const { return newsUpKey_; }
    const std::string &getNewsDownKey() const { return newsDownKey_; }

    // How many units `cash` buys at the current price.
    int calPossibleNumber(int cash) const { return cash <= 0 ? 0 : cash / price_; }

    bool operator==(const Item &other) const { return key_ == other.key_; }
    bool operator!=(const Item &other) const { return !(*this == other); }

private:
    std::string key_;
    int basePrice_ = 1;
    int price_ = 1;
    int reputationPrice_ = 0;  // reputation change every time this good is sold
    std::string newsUpKey_;
    std::string newsDownKey_;
};

namespace std {
template <>
struct hash<Item> {
    size_t operator()(const Item &item) const { return hash<std::string>()(item.getKey()); }
};
}  // namespace std

// The seven goods of the original game, with their baseline prices.
std::vector<Item> defaultItems();

#endif
