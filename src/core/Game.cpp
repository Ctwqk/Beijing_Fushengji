#include "Game.h"

#include <cmath>
#include <stdexcept>

#include "Language.h"

Game::Game(GameConfig cfg, std::vector<Item> items) : cfg_(cfg), rng_(cfg.seed) {
    // The market gets its own stream so that trading never shifts the calendar rolls.
    market_ = std::make_shared<Market>(std::move(items), rng_.next());
    player_ = std::make_shared<Player>(cfg_.startHealth, cfg_.startCash, cfg_.startDebt, 0, cfg_.startReputation,
                                       cfg_.startSpace);
    player_->setMarket(market_);
    const int interestPercent = static_cast<int>(std::lround((Player::DEBT_RATING - 1.0) * 100.0));
    player_->setLifeStory(Language::getInstance().format(
        "LIFE_BACKGROUND", {std::to_string(cfg_.startCash), std::to_string(cfg_.startDebt), std::to_string(interestPercent)}));
}

Game::DayReport Game::advanceDay() {
    if (isOver()) throw std::logic_error("the game is already over");
    ++day_;
    player_->newDay();
    DayReport report;
    report.day = day_;
    report.news = market_->newDay();
    report.over = isOver();
    report.lastDay = day_ == cfg_.totalDays - 1;
    report.randomEventDue = !report.over && rng_.chance(cfg_.randomEventChance);
    return report;
}

Game::Ending Game::finish() {
    Ending ending;
    ending.died = player_->isDead();
    ending.sales = player_->liquidate();
    ending.netWorth = player_->netWorth();
    return ending;
}

EventContext Game::eventContext(Language::Lang lang) const {
    EventContext ctx;
    ctx.day = day_;
    ctx.cash = player_->getCash();
    ctx.health = player_->getHealth();
    ctx.reputation = player_->getReputation();
    ctx.lifeStory = player_->getLifeStory();
    ctx.lang = lang;
    return ctx;
}
