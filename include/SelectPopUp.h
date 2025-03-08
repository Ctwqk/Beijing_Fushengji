#ifndef SELECTPOPUP_H
#define SELECTPOPUP_H
#include "PopUp.h"
#include <string>
#include <vector>


struct ButtonWithAction;

class SelectPopUp: public PopUp{
    private:
        std::vector<ButtonWithAction> selections;
        int buttonPerRow = 3;
    public:
        void Render();
        // void Open();
        SelectPopUp(std::vector<ButtonWithAction> selections, int bpr);
};

#endif