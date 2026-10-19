#include "Events.h"

#include "Language.h"
#include "Player.h"

std::shared_ptr<Event> makeNewsEvent(const Market::NewsEvent &news) {
    std::string text = GET_TEXT(news.textKey) + "\n\n" + news.item.getName() + ": " + std::to_string(news.oldPrice) +
                       " -> " + std::to_string(news.newPrice);
    return std::make_shared<Event>(GET_TEXT("NEWS_EVENT"), text);
}

std::shared_ptr<Event> makeRandomEventPopup(const RandomEvent &event, const std::shared_ptr<Player> &player) {
    std::vector<std::shared_ptr<ButtonWithAction>> buttons;
    for (const EventOption &option : event.options) {
        const std::string story = event.description + GET_TEXT("COMMA_YOUR_DECISION") + option.text;
        buttons.push_back(std::make_shared<ButtonWithAction>(option.text, [player, option, story]() {
            player->applyDelta(option.dCash, option.dHealth, option.dReputation);
            player->recordStory(story);
        }));
    }
    return std::make_shared<Event>(event.title, event.description, std::move(buttons));
}
