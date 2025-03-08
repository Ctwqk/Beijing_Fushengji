#ifndef TEXTPOPUP_H
#define TEXTPOPUP_H

#include "Item.h"
#include "PopUp.h"
#include <string>
#include "Button.h"
#include "Events.h"
class TextPopUp : public PopUp{
    private:
        std::string text;
        std::string label;
        ButtonWithAction* button;
        std::vector<Event*> events;
    public:
        TextPopUp();
        void Render();
        void Open(std::string text, std::string label, ButtonWithAction *b = nullptr);
        void setEvents(std::vector<Event*> &e);
};


#endif