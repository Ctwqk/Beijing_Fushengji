#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>

#include "Item.h"
#include "Market.h"
#include "Player.h"
#include "RandomEvent.h"
#include "Rng.h"

struct GameConfig {
    std::uint32_t seed = Rng::randomSeed();
    int totalDays = 40;
    double randomEventChance = 0.3;  // chance per day that a random life event is due
    int startCash = 2000;
    int startDebt = 5000;
    int startHealth = 100;
    int startReputation = 100;
    int startSpace = 100;
};

// Window-free game state: the player, the market and the calendar.
// The UI drives it with advanceDay() and finish().
class Game {
public:
    struct DayReport {
        int day = 0;                             // days elapsed after this move
        std::vector<Market::NewsEvent> news;     // price shocks of the new day
        bool randomEventDue = false;             // the UI should fetch a random life event
        bool lastDay = false;                    // the next move ends the game
        bool over = false;                       // the game has ended (calendar ran out or player died)
    };
    struct Ending {
        bool died = false;
        long long netWorth = 0;                  // cash + savings - debt after selling the stock
        std::vector<Player::Sale> sales;         // what had to be sold off
    };

    explicit Game(GameConfig cfg = {}, std::vector<Item> items = defaultItems());

    const std::shared_ptr<Player> &player() const { return player_; }
    const std::shared_ptr<Market> &market() const { return market_; }
    const GameConfig &config() const { return cfg_; }
    int day() const { return day_; }
    int totalDays() const { return cfg_.totalDays; }
    bool isOver() const { return day_ >= cfg_.totalDays || player_->isDead(); }

    // Moves to the next day: interest accrues, prices are rerolled, news may strike.
    // Throws std::logic_error once the game is over.
    DayReport advanceDay();

    // Sells whatever is left and settles the accounts. Idempotent.
    Ending finish();

    EventContext eventContext(Language::Lang lang) const;

private:
    GameConfig cfg_;
    Rng rng_;
    int day_ = 0;
    std::shared_ptr<Market> market_;
    std::shared_ptr<Player> player_;
};

#endif
