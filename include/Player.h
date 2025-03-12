#ifndef PLAYER_H
#define PLAYER_H
#include "Item.h"
#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>

class Market;
class AiApi;

class Player{
    private:
        std::shared_ptr<Market> market;
        static std::shared_ptr<AiApi> aiApi;
    protected:
        int health;
        int cash;
        int debt;
        int saving;
        int reputation;
        std::unordered_map<Item, int> inventory;
        int space;
        std::string lifeStory;

    public:
        static void setAiApi(const std::shared_ptr<AiApi> &aiApi);
        static constexpr float DEBT_RATING = 1.05, SAVING_RATING = 1.04, UPPER_LIMIT = 100;
        Player(int h, int c, int d, int s, int r, int sp);
        void setMarket(const std::shared_ptr<Market> &marketPtr);
        void setHealth(int h) { health = h; }
        void setCash(int c) { cash = c; }
        void setDebt(int d) { debt = d; }
        void setSaving(int s) { saving = s; }
        void setReputation(int r) { reputation = r; } 
        void setSpace(int s) {space = s;}
        void setLifeStory(std::string ls){lifeStory = ls;}
        std::string getLifeStory(){return lifeStory;}
        int getHealth(){return health;}
        int getCash(){return cash;}
        int getDebt(){return debt;}
        int getSaving(){return saving;}
        int getReputation(){return reputation;}
        int getSpace(){return space;}
        const std::vector<std::pair<Item,int>> getInventory();
        void buy(Item &i, int count);
        void sell(Item &i, int count);
        int getItemNumber(Item &i);
        std::shared_ptr<Event> randomEvent();
        void newDay();
};


#endif