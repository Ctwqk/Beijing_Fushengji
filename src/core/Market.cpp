#include "Market.h"

#include <cmath>
#include <stdexcept>

Market::Market(std::vector<Item> items, std::uint32_t seed, MarketConfig cfg)
    : items_(std::move(items)), available_(items_.size(), true), rng_(seed), cfg_(cfg) {
    for (std::size_t i = 0; i < items_.size(); ++i) index_[items_[i].getKey()] = static_cast<int>(i);
    newDay();
}

std::vector<Market::NewsEvent> Market::newDay() {
    std::vector<NewsEvent> news;
    bool any = false;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        Item &item = items_[i];
        double factor = cfg_.minFactor + (cfg_.maxFactor - cfg_.minFactor) * rng_.uniform();
        item.setPrice(static_cast<int>(std::lround(item.getBasePrice() * factor)));
        available_[i] = rng_.chance(cfg_.availability);
        any = any || available_[i];
    }
    // Never open an empty market.
    if (!any && !items_.empty()) {
        available_[static_cast<std::size_t>(rng_.range(0, static_cast<int>(items_.size()) - 1))] = true;
    }
    for (std::size_t i = 0; i < items_.size(); ++i) {
        Item &item = items_[i];
        if (!available_[i] || item.getNewsUpKey().empty() || !rng_.chance(cfg_.newsChance)) continue;
        bool up = rng_.chance(0.5);
        int oldPrice = item.getPrice();
        item.setPrice(up ? oldPrice * cfg_.newsMultiplier : oldPrice / cfg_.newsMultiplier);
        news.push_back({item, up, up ? item.getNewsUpKey() : item.getNewsDownKey(), oldPrice, item.getPrice()});
    }
    return news;
}

std::vector<Item> Market::getAvailableItems() const {
    std::vector<Item> out;
    for (std::size_t i = 0; i < items_.size(); ++i)
        if (available_[i]) out.push_back(items_[i]);
    return out;
}

int Market::indexOf(const Item &i) const {
    auto it = index_.find(i.getKey());
    if (it == index_.end()) throw std::runtime_error("no such item: " + i.getKey());
    return it->second;
}

bool Market::isAvailable(const Item &i) const {
    auto it = index_.find(i.getKey());
    return it != index_.end() && available_[static_cast<std::size_t>(it->second)];
}

int Market::askPrice(const Item &i) const { return items_[static_cast<std::size_t>(indexOf(i))].getPrice(); }

const Item &Market::getItem(const Item &i) const { return items_[static_cast<std::size_t>(indexOf(i))]; }
