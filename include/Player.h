#ifndef PLAYER_H
#define PLAYER_H
#include "Item.h"
#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>

class Market;

class Player{
    private:
        Market * market;
    protected:
        int health;
        int cash;
        int debt;
        int saving;
        int reputation;
        std::unordered_map<Item, int> inventory;
        int space;

    public:
        static constexpr float DEBT_RATING = 1.05, SAVING_RATING = 1.04, UPPER_LIMIT = 100;
        Player(int h, int c, int d, int s, int r, int sp);
        void setMarket(Market * marketPtr);
        void setHealth(int h) { health = h; }
        void setCash(int c) { cash = c; }
        void setDebt(int d) { debt = d; }
        void setSaving(int s) { saving = s; }
        void setReputation(int r) { reputation = r; } 
        void setSpace(int s) {space = s;}
        int getHealth(){return health;}
        int getCash(){return cash;}
        int getDebt(){return debt;}
        int getSaving(){return saving;}
        int getReputation(){return reputation;}
        int getSpace(){return space;}
        std::vector<std::pair<Item,int>> getInventory();
        void buy(Item &i, int count);
        void sell(Item &i, int count);
        void newDay();
};


#endif