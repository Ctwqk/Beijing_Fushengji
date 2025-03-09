#include "Player.h"
#include "Market.h"


Player::Player(int h, int c, int d, int s, int r, int sp):health(h),cash(c),debt(d),saving(s),reputation(r), space(sp){}

void Player::sell(Item &i, int count){
    if(inventory.find(i) == inventory.end() || inventory[i] < count){
        // std::cout<<i.name<<std::endl;
        throw std::runtime_error("not enough item to sell");
        return;
    }
    std::cout<<i.getName()<<std::endl;
    cash += market->askPrice(i) * count;
    space += count;
    inventory[i] -= count;
    try{
        market->addItemToAvailable(i);
    }
    catch(std::runtime_error &e){
        std::cout<<e.what()<<std::endl;
    }
    reputation += i.getReputationPrice();
}

void Player::buy(Item &i, int count){
    // std::cout<<"Count: "<<count<<std::endl;
    if(i.getPrice() * count > cash){
        throw std::runtime_error("not enough money");
        return;
    }
    if(count > space){
        throw std::runtime_error("not enough space");
        return;
    }
    cash -= i.getPrice() * count;
    space -= count;
    
    if(inventory.find(i)!=inventory.end()){
        inventory[i] += count;
    }
    else{
        inventory[i] = count;
    }
}


void Player::setMarket(const std::shared_ptr<Market> &marketPtr){
    market = marketPtr;
}


const std::vector<std::pair<Item,int>> Player::getInventory(){
    std::vector<std::pair<Item,int>> ans;
    for(const auto& p:inventory){
        if(p.second)
            ans.push_back(p);
    }
    return ans;
}

int Player::getItemNumber(Item &i){
    return inventory[i];
}

void Player::newDay(){
    debt = (round) (DEBT_RATING * debt);
    saving = (round) (SAVING_RATING * saving);
}