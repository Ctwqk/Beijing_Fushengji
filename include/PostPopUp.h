#ifndef POSTPOPUP_H
#define POSTPOPUP_H
#include "PopUp.h"
#include "TextPopUp.h"
#include "Player.h"
#include <string>

class PostPopUp: public PopUp{
    private:
        Player* player;
    protected:
        int quantity;
        int maxMoney;

    public:
        PostPopUp(Player *p);
        void Render();
        void Open();
};


#endif