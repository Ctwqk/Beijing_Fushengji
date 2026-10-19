#ifndef EVENTSOURCEFACTORY_H
#define EVENTSOURCEFACTORY_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "AiConfig.h"
#include "RandomEvent.h"

struct EventSourceSetup {
    std::shared_ptr<RandomEventSource> source;
    std::string description;       // e.g. "AI (gpt-4o-mini)" or "offline"; safe to show in the UI
    bool ai = false;               // true when the LLM is in use
    std::function<void()> cancel;  // aborts in-flight LLM requests (no-op for offline)
};

// Builds the event source for the given configuration: the LLM with an offline
// fallback when AI is enabled and libcurl support was compiled in, otherwise
// the offline generator alone.
EventSourceSetup makeEventSource(const AiConfig &config, std::uint32_t seed);

#endif
