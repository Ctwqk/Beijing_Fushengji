#ifndef BANKPOPUP_H
#define BANKPOPUP_H

#include "PopUp.h"
#include "SelectPopUp.h"
class Player;

class BankPopUp: public PopUp{
    private:
        std::shared_ptr<Player> player;
        std::unique_ptr<SelectPopUp> selectWin;
    protected:
        int quantity;
        int maxMoney;
        bool isSaving;
        bool isSelected = false;
    public:
        BankPopUp(const std::shared_ptr<Player> &p);
        void Render();
        void Open();
        void Close();
};

#endif