#ifndef BANKPOPUP_H
#define BANKPOPUP_H

#include "PopUp.h"
class Player;
class SelectPopUp;

class BankPopUp: public PopUp{
    private:
        Player* player;
        SelectPopUp* selectWin;
    protected:
        int quantity;
        int maxMoney;
        bool isSaving;
        bool isSelected;
    public:
        BankPopUp(Player *p);
        void Render();
        void Open();
        void Close();
};

#endif