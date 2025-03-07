#include "Market.h"
#include "Player.h"


void Market::setPlayer(Player* playerPtr){
    p = playerPtr;
}

bool Market::whetherToInclude(){
    return dist(gen) >= 0.40000;
}

Market::Market(std::vector<Item> &v): gen(std::random_device{}()), dist(0.0, 1.0){
    for(Item &i:v){
        inventory[i] = i.price;
        // std::cout<<i.price<<std::endl;
    }
}

std::vector<Item> Market::getAvailableItems(){
    std::vector<Item> ans;
    for (auto& pair: inventory){
        if(whetherToInclude()){
            ans.push_back(pair.first);
            ans.back().renewPrice();
            // pair.second = pair.first.price;
            ans.back().setEvent();
        }
    }
    return ans;
}

std::vector<Item> Market::newDay(){
    availableItems = getAvailableItems();
    return availableItems;
}

void Market::addItem(Item &i){
    if(inventory.find(i) == inventory.end()){
        throw std::runtime_error("no such item");
        return;
    }
    inventory[i] = true;
    availableItems.push_back(i);
}

int Market::askPrice(Item &i){
    if(inventory.find(i)==inventory.end()){
        return -1;
    }
    return inventory[i];
}