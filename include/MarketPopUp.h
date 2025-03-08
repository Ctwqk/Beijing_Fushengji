#ifndef MARKETPOPUP_H
#define MARKETPOPUP_H

#include "Item.h"
#include <memory>
#include "PopUp.h"
#include "TextPopUp.h"
#include <string>

class Player;
class Market;

class MarketPopUp : public PopUp{
    private:
        Player* player;
    protected:
        Item curItem;
        int totalCash;
        int totalNumber;
        int availableSpace;
        bool isBuy;
        int quantity;
        
        std::string noMoney, noSpace, label;
        bool tradeSuccess = true;
        
    public:
        MarketPopUp( Player * p);
        void Render() ;
        void Open(Item &i, bool iB);
};


#endif