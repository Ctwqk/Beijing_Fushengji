#ifndef TEXTPOPUP_H
#define TEXTPOPUP_H

#include "PopUp.h"
#include <string>
#include "Button.h"
#include <deque>

class Item;

class Event;
class TextPopUp : public PopUp{
    private:
        std::string text;
        std::string label;
        std::vector<std::shared_ptr<ButtonWithAction>> buttons;
        std::deque<std::shared_ptr<Event>> events;
        int buttonPerRow = 3;
    public:
        TextPopUp();
        void Render();
        void Open(std::string text, std::string label, std::shared_ptr<ButtonWithAction>b = nullptr);
        void setEvents(std::vector<std::shared_ptr<Event>> &e);
};


#endif