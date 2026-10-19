#ifndef EVENTS_H
#define EVENTS_H

#include <memory>
#include <string>
#include <vector>

#include "Button.h"
#include "Market.h"
#include "RandomEvent.h"

class Player;

// A message the UI shows in a modal popup: a label (title), a text and the
// buttons that dismiss it. An event without buttons gets a default "Confirm".
struct Event {
    std::string label;
    std::string text;
    std::vector<std::shared_ptr<ButtonWithAction>> buttons;

    Event(std::string l, std::string t, std::vector<std::shared_ptr<ButtonWithAction>> bs)
        : label(std::move(l)), text(std::move(t)), buttons(std::move(bs)) {}
    Event(std::string l, std::string t, std::shared_ptr<ButtonWithAction> b = nullptr)
        : label(std::move(l)), text(std::move(t)) {
        if (b)
            buttons.push_back(std::move(b));
    }
};

// Informational popup for a news event. The price change has already been
// applied by Market::newDay().
std::shared_ptr<Event> makeNewsEvent(const Market::NewsEvent &news);

// Popup presenting a random event; every option applies its outcome to the player.
std::shared_ptr<Event> makeRandomEventPopup(const RandomEvent &event, const std::shared_ptr<Player> &player);

#endif  // EVENTS_H
