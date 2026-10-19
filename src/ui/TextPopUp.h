#ifndef TEXTPOPUP_H
#define TEXTPOPUP_H

#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "Button.h"
#include "PopUp.h"

struct Event;

// Shows a queue of Events one after the other in a single modal window.
class TextPopUp : public PopUp {
public:
    TextPopUp() = default;
    void Render() override;
    // Convenience for one-off messages (errors, hints).
    void Open(const std::string &text, const std::string &label, std::shared_ptr<ButtonWithAction> b = nullptr);
    void setEvents(const std::vector<std::shared_ptr<Event>> &e);
    size_t pending() const { return events.size(); }

private:
    std::deque<std::shared_ptr<Event>> events;
};

#endif
