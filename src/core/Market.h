#ifndef MARKET_H
#define MARKET_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Item.h"
#include "Rng.h"

struct MarketConfig {
    double availability = 0.6;  // chance a given good is on sale on a given day
    double minFactor = 0.5;     // daily price = basePrice * uniform(minFactor, maxFactor)
    double maxFactor = 1.5;
    double newsChance = 0.08;  // chance a listed good is hit by a news event
    int newsMultiplier = 5;    // news multiplies (or divides) the day's price by this
};

// The black market: every day each good gets a fresh price around its baseline,
// only a random subset is offered for sale, and occasionally a news item sends
// one price soaring or crashing.
class Market {
public:
    struct NewsEvent {
        Item item;  // the good, carrying its post-event price
        bool priceUp;
        std::string textKey;
        int oldPrice;
        int newPrice;
    };

    explicit Market(std::vector<Item> items, std::uint32_t seed = Rng::randomSeed(), MarketConfig cfg = {});

    // Rolls a new day (prices, availability, news) and returns the news events.
    std::vector<NewsEvent> newDay();

    const std::vector<Item> &getItems() const { return items_; }
    std::vector<Item> getAvailableItems() const;
    bool isAvailable(const Item &i) const;
    // Today's price of a good (available or not). Throws std::runtime_error for unknown goods.
    int askPrice(const Item &i) const;
    const Item &getItem(const Item &i) const;

private:
    int indexOf(const Item &i) const;

    std::vector<Item> items_;
    std::vector<bool> available_;
    std::unordered_map<std::string, int> index_;
    Rng rng_;
    MarketConfig cfg_;
};

#endif
