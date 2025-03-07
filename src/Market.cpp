#include "Market.h"
#include "Player.h"


void Market::setPlayer(Player* playerPtr){
    p = playerPtr;
}

bool Market::whetherToInclude(){
    return dist(gen) >= 0.40000;
}

Market::Market(std::vector<Item> &v): gen(std::random_device{}()), dist(0.0, 1.0){
    
    for(int i=0;i<v.size();i++){
        inventory.push_back({v[i], false});
        inventory_[v[i]] = i;
    }
}

std::vector<Item> Market::getAvailableItems(){
    std::vector<Item> ans;
    for (auto& pair: inventory){
        if(whetherToInclude()){
            pair.first.renewPrice();
            // pair.second = pair.first.price;
            pair.first.setEvent();
            ans.push_back(pair.first);
        }
    }
    availableItems = ans;
    return ans;
}

std::vector<Item> Market::newDay(){
    availableItems = getAvailableItems();
    return availableItems;
}

void Market::addItemToAvailable(Item &i){
    if(inventory_.find(i) != inventory_.end()){
        throw std::runtime_error("item already exist");
        return;
    }
    inventory[inventory_[i]].second = true;
    availableItems.push_back(i);
}

int Market::askPrice(Item &i){
    if(inventory_.find(i)==inventory_.end()){
        throw std::runtime_error("no such item");
    }
    return inventory[inventory_[i]].first.price;
}