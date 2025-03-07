#ifndef MARKET_H
#define MARKET_H

#include "Item.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <utility>
#include <random>
#include <memory>

class Player;

class Market{
    private:
        bool whetherToInclude();
        Player* p;
        std::mt19937 gen;
        std::uniform_real_distribution<double> dist;
    protected:
        std::unordered_map<Item, int> inventory;
        std::vector<Item> availableItems;
    public:
        void setPlayer(Player* playerPtr);
        Market(std::vector<Item> &v);
        std::vector<Item> getAvailableItems();
        std::vector<Item> newDay();
        void addItem(Item &i);
        int askPrice(Item & i);
        std::vector<Item> getItems(){return availableItems;}
        
        
};



#endif