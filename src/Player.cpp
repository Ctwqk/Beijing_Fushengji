#include "Player.h"
#include "Market.h"
#include "AiApi.h"
#include "Language.h"


Player::Player(int h, int c, int d, int s, int r, int sp):health(h),cash(c),debt(d),saving(s),reputation(r), space(sp){}
std::shared_ptr<AiApi> Player::aiApi = nullptr;

void Player::setAiApi(const std::shared_ptr<AiApi> &api){
    aiApi = api;
}

std::shared_ptr<Event> Player::randomEvent(){
    try{
        std::vector<std::vector<std::string>> params = aiApi->generateRandomEvent(getCash(), getHealth(), getReputation(), getLifeStory());
        std::vector<std::shared_ptr<ButtonWithAction>> selections;
        // std::cout<<params.size()<<std::endl;
        for(int i=1;i<=3;i++){
            try {
                int deltaCash = std::stoi(params[i][1]);
                int deltaHealth = std::stoi(params[i][2]);
                int deltaReputation = std::stoi(params[i][3]);
                std::string eventText = params[0][1] + GET_TEXT("COMMA_YOUR_DECISION") + params[i][0] + "\n";

                selections.push_back(std::make_shared<ButtonWithAction>(
                    params[i][0], 
                    [eventText, deltaCash, deltaHealth, deltaReputation, i, this]() {
                        setCash(getCash() + deltaCash);
                        setHealth(getHealth() + deltaHealth);
                        setReputation(getReputation() + deltaReputation);
                        setLifeStory(getLifeStory() + eventText);
                    }
                ));
            } catch (const std::exception &e) {
                std::cerr << "Error parsing event option " << i << ": " << e.what() << std::endl;
            }
        }
        // for(int i=0;i<params.size();i++){
        //     std::cout<<params[i].size()<<std::endl;
        // }
        return std::make_shared<Event>(params[0][0], params[0][1], selections);
    }
    catch(const std::exception &e){
        std::cerr << e.what() << std::endl;
        throw ;
    }
    return nullptr;
}


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