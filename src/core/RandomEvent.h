#ifndef RANDOMEVENT_H
#define RANDOMEVENT_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "Language.h"
#include "Rng.h"

// One choice of a random life event and its outcome.
struct EventOption {
    std::string text;
    int dCash = 0;
    int dHealth = 0;
    int dReputation = 0;
};

// A small narrative event with several options.
struct RandomEvent {
    std::string title;
    std::string description;
    std::vector<EventOption> options;
};

// Snapshot of the game handed to an event generator. It is a plain value so it
// can safely be passed to a worker thread.
struct EventContext {
    int day = 0;
    int cash = 0;
    int health = 0;
    int reputation = 0;
    std::string lifeStory;
    Language::Lang lang = Language::Lang::ENGLISH;
};

// Limits applied to every generated outcome (offline or LLM) so that a model
// can never hand the player a fortune or an instant death.
struct EventLimits {
    static constexpr int MAX_CASH = 1000;
    static constexpr int MAX_HEALTH = 25;
    static constexpr int MAX_REPUTATION = 40;
    static constexpr std::size_t MAX_OPTIONS = 3;
};

class RandomEventSource {
public:
    virtual ~RandomEventSource() = default;
    // May throw std::exception when no event could be produced.
    virtual RandomEvent generate(const EventContext &ctx) = 0;
    virtual std::string describe() const = 0;
};

// Deterministic (for a given seed) generator backed by a built-in bilingual
// table of events. Needs no network.
class OfflineEventSource : public RandomEventSource {
public:
    explicit OfflineEventSource(std::uint32_t seed = Rng::randomSeed());
    RandomEvent generate(const EventContext &ctx) override;
    std::string describe() const override { return "offline"; }
    static std::size_t eventCount();

private:
    std::mutex mutex_;
    Rng rng_;
};

// Tries `primary` first and quietly falls back to `fallback` when it throws.
class FallbackEventSource : public RandomEventSource {
public:
    FallbackEventSource(std::shared_ptr<RandomEventSource> primary, std::shared_ptr<RandomEventSource> fallback);
    RandomEvent generate(const EventContext &ctx) override;
    std::string describe() const override { return primary_->describe(); }

private:
    std::shared_ptr<RandomEventSource> primary_;
    std::shared_ptr<RandomEventSource> fallback_;
};

// --- LLM prompt / response handling (pure functions, unit tested) ------------
std::string buildEventSystemPrompt(Language::Lang lang);
std::string buildEventUserPrompt(const EventContext &ctx);
// Parses a model reply into a RandomEvent. Tolerates markdown fences, text
// around the JSON object and numbers given as strings ("+300"). Throws
// std::runtime_error when the reply is unusable. Outcomes are clamped to EventLimits.
RandomEvent parseRandomEvent(const std::string &reply);
// Clamps all numbers of the event to EventLimits and drops surplus options.
void sanitizeRandomEvent(RandomEvent &event);

#endif
