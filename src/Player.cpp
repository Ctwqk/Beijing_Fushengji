#include "Player.h"
#include "Market.h"


Player::Player(int h, int c, int d, int s, int r, int sp):health(h),cash(c),debt(d),saving(s),reputation(r), space(sp){}

void Player::sell(Item &i, int count){
    if(inventory.find(i) == inventory.end() || inventory[i] < count){
        // std::cout<<i.name<<std::endl;
        throw std::runtime_error("not enough item to sell");
        return;
    }
    
    cash += market->askPrice(i) * count;
    space += count;
    inventory[i] -= count;
    market->addItem(i);
    reputation += i.reputationPrice;
}

void Player::buy(Item &i, int count){
    // std::cout<<"Count: "<<count<<std::endl;
    if(i.price * count > cash){
        throw std::runtime_error("not enough money");
        return;
    }
    if(count > space){
        throw std::runtime_error("not enough space");
        return;
    }
    cash -= i.price * count;
    space -= count;
    
    if(inventory.find(i)!=inventory.end()){
        inventory[i] += count;
    }
    else{
        inventory[i] = count;
    }
}


void Player::setMarket(Market * marketPtr){
    market = marketPtr;
}


std::vector<std::pair<Item,int>> Player::getInventory(){
    std::vector<std::pair<Item,int>> ans;
    for(const auto& p:inventory){
        if(p.second)
            ans.push_back(p);
    }
    return ans;
}

void Player::newDay(){
    debt = (round) (DEBT_RATING * debt);
    saving = (round) (SAVING_RATING * saving);
}