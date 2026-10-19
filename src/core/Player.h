#ifndef PLAYER_H
#define PLAYER_H

#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Item.h"
#include "Market.h"

// Thrown when a trade cannot be carried out; reason() tells the UI why.
class TradeError : public std::runtime_error {
public:
    enum class Reason { InvalidQuantity, NotEnoughCash, NotEnoughSpace, NotEnoughStock, NotOnSale };
    TradeError(Reason r, const std::string &what) : std::runtime_error(what), reason_(r) {}
    Reason reason() const { return reason_; }

private:
    Reason reason_;
};

class Player {
public:
    // One line of the inventory. totalCost is what the units currently held cost,
    // so averageCost() is the "buy price" shown to the player.
    struct Holding {
        Item item;
        int quantity = 0;
        long long totalCost = 0;
        int averageCost() const { return quantity > 0 ? static_cast<int>(totalCost / quantity) : 0; }
    };
    struct Sale {
        Item item;
        int quantity = 0;
        int unitPrice = 0;
        long long proceeds = 0;
    };

    static constexpr double DEBT_RATING = 1.05;    // debt grows 5% per day
    static constexpr double SAVING_RATING = 1.04;  // savings grow 4% per day
    static constexpr int UPPER_LIMIT = 100;        // maximum health
    static constexpr int PRICE_PER_HEALTH = 1750;  // hospital price of one health point
    static constexpr std::size_t MAX_STORY_ENTRIES = 8;

    Player(int health, int cash, int debt, int saving, int reputation, int space);

    void setMarket(const std::shared_ptr<Market> &marketPtr) { market = marketPtr; }

    // --- state ---
    int getHealth() const { return health; }
    int getCash() const { return cash; }
    int getDebt() const { return debt; }
    int getSaving() const { return saving; }
    int getReputation() const { return reputation; }
    int getSpace() const { return space; }  // free inventory slots
    void setHealth(int h) { health = h < 0 ? 0 : (h > UPPER_LIMIT ? UPPER_LIMIT : h); }
    void setCash(int c) { cash = c; }
    void setDebt(int d) { debt = d; }
    void setSaving(int s) { saving = s; }
    void setReputation(int r) { reputation = r < 0 ? 0 : r; }
    void setSpace(int s) { space = s; }
    bool isDead() const { return health <= 0; }

    // --- trading ---
    // Both use the market's price of the day, not the price carried by `i`.
    void buy(const Item &i, int count);
    void sell(const Item &i, int count);
    int getItemNumber(const Item &i) const;
    const std::vector<Holding> &getHoldings() const { return holdings; }
    int maxAffordable(const Item &i) const;  // limited by cash and free space
    // Sells everything at the market's current prices and returns the receipts.
    std::vector<Sale> liquidate();

    // --- bank / post office / hospital ---
    void deposit(int amount);
    void withdraw(int amount);
    void repayDebt(int amount);
    int maxHealable() const;  // limited by missing health and cash
    void heal(int points);

    // Applies a random-event outcome; cash/health/reputation never go below zero
    // and health never exceeds UPPER_LIMIT.
    void applyDelta(int deltaCash, int deltaHealth, int deltaReputation);

    // Daily compound interest on debt and savings.
    void newDay();

    // cash + savings - debt (+ the stock valued at today's market prices).
    long long netWorth() const;

    // --- narrative used as LLM context ---
    void setLifeStory(const std::string &background) { lifeStory = background; }
    void recordStory(const std::string &entry);
    std::string getLifeStory() const;

private:
    Holding *findHolding(const Item &i);
    const Holding *findHolding(const Item &i) const;

    std::shared_ptr<Market> market;
    int health;
    int cash;
    int debt;
    int saving;
    int reputation;
    int space;
    std::vector<Holding> holdings;
    std::string lifeStory;
    std::deque<std::string> recentStory;
};

#endif
