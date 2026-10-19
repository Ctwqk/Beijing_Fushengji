#include "Player.h"

#include <algorithm>
#include <cmath>

Player::Player(int h, int c, int d, int s, int r, int sp)
    : health(h < 0 ? 0 : (h > UPPER_LIMIT ? UPPER_LIMIT : h)),
      cash(c),
      debt(d),
      saving(s),
      reputation(r < 0 ? 0 : r),
      space(sp) {}

Player::Holding *Player::findHolding(const Item &i) {
    for (auto &h : holdings)
        if (h.item == i)
            return &h;
    return nullptr;
}

const Player::Holding *Player::findHolding(const Item &i) const {
    for (const auto &h : holdings)
        if (h.item == i)
            return &h;
    return nullptr;
}

int Player::getItemNumber(const Item &i) const {
    const Holding *h = findHolding(i);
    return h ? h->quantity : 0;
}

int Player::maxAffordable(const Item &i) const {
    int price = market ? market->askPrice(i) : i.getPrice();
    return std::max(0, std::min(cash / std::max(1, price), space));
}

void Player::buy(const Item &i, int count) {
    if (count <= 0)
        throw TradeError(TradeError::Reason::InvalidQuantity, "quantity must be positive");
    if (market && !market->isAvailable(i))
        throw TradeError(TradeError::Reason::NotOnSale, i.getKey() + " is not on sale today");
    const int price = market ? market->askPrice(i) : i.getPrice();
    const long long cost = static_cast<long long>(price) * count;
    if (cost > cash)
        throw TradeError(TradeError::Reason::NotEnoughCash, "not enough money");
    if (count > space)
        throw TradeError(TradeError::Reason::NotEnoughSpace, "not enough space");

    cash -= static_cast<int>(cost);
    space -= count;
    if (Holding *h = findHolding(i)) {
        h->quantity += count;
        h->totalCost += cost;
    } else {
        Holding fresh;
        fresh.item = i;
        fresh.quantity = count;
        fresh.totalCost = cost;
        holdings.push_back(fresh);
    }
}

void Player::sell(const Item &i, int count) {
    if (count <= 0)
        throw TradeError(TradeError::Reason::InvalidQuantity, "quantity must be positive");
    Holding *h = findHolding(i);
    if (!h || h->quantity < count)
        throw TradeError(TradeError::Reason::NotEnoughStock, "not enough item to sell");
    const int price = market ? market->askPrice(i) : i.getPrice();

    // Remove the cost basis proportionally so the average cost of the rest is unchanged.
    h->totalCost -= h->totalCost * count / h->quantity;
    h->quantity -= count;
    cash += static_cast<int>(static_cast<long long>(price) * count);
    space += count;
    reputation = std::max(0, reputation + i.getReputationPrice());
    if (h->quantity == 0) {
        holdings.erase(std::remove_if(holdings.begin(), holdings.end(), [](const Holding &x) { return x.quantity == 0; }),
                       holdings.end());
    }
}

std::vector<Player::Sale> Player::liquidate() {
    std::vector<Sale> sales;
    const std::vector<Holding> snapshot = holdings;
    for (const auto &h : snapshot) {
        Sale s;
        s.item = h.item;
        s.quantity = h.quantity;
        s.unitPrice = market ? market->askPrice(h.item) : h.item.getPrice();
        s.proceeds = static_cast<long long>(s.unitPrice) * s.quantity;
        sell(h.item, h.quantity);
        sales.push_back(s);
    }
    return sales;
}

void Player::deposit(int amount) {
    if (amount < 0 || amount > cash)
        throw std::invalid_argument("invalid deposit amount");
    cash -= amount;
    saving += amount;
}

void Player::withdraw(int amount) {
    if (amount < 0 || amount > saving)
        throw std::invalid_argument("invalid withdrawal amount");
    saving -= amount;
    cash += amount;
}

void Player::repayDebt(int amount) {
    if (amount < 0 || amount > cash || amount > debt)
        throw std::invalid_argument("invalid repayment amount");
    cash -= amount;
    debt -= amount;
}

int Player::maxHealable() const {
    return std::max(0, std::min(UPPER_LIMIT - health, cash / PRICE_PER_HEALTH));
}

void Player::heal(int points) {
    if (points < 0 || points > UPPER_LIMIT - health)
        throw std::invalid_argument("invalid heal amount");
    const long long cost = static_cast<long long>(points) * PRICE_PER_HEALTH;
    if (cost > cash)
        throw TradeError(TradeError::Reason::NotEnoughCash, "not enough money for treatment");
    cash -= static_cast<int>(cost);
    setHealth(health + points);
}

void Player::applyDelta(int deltaCash, int deltaHealth, int deltaReputation) {
    cash = std::max(0, cash + deltaCash);
    setHealth(health + deltaHealth);
    setReputation(reputation + deltaReputation);
}

void Player::newDay() {
    debt = static_cast<int>(std::llround(debt * DEBT_RATING));
    saving = static_cast<int>(std::llround(saving * SAVING_RATING));
}

long long Player::netWorth() const {
    long long worth = static_cast<long long>(cash) + saving - debt;
    if (market) {
        for (const auto &h : holdings) worth += static_cast<long long>(market->askPrice(h.item)) * h.quantity;
    }
    return worth;
}

void Player::recordStory(const std::string &entry) {
    recentStory.push_back(entry);
    while (recentStory.size() > MAX_STORY_ENTRIES) recentStory.pop_front();
}

std::string Player::getLifeStory() const {
    std::string story = lifeStory;
    for (const auto &e : recentStory) story += "\n" + e;
    return story;
}
