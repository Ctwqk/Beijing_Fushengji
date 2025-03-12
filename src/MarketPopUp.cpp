#include "MarketPopUp.h"
#include "Player.h"
#include "Market.h"
#include "Language.h"


void MarketPopUp::Render(){
    if (isOpen){
        ImGui::OpenPopup(GET_TEXT("MARKET_TITLE").c_str());
    }
    if (ImGui::BeginPopupModal(GET_TEXT("MARKET_TITLE").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputInt("##", &quantity, 1, 5);
        if(quantity < 0) quantity = 0;
        if(quantity>totalNumber) quantity = totalNumber;
        ImGui::Spacing();
        // std::cout<<"hhh"<<std::endl;
        if(ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100,40))) {
            try{
                if(isBuy)player->buy(curItem, quantity);
                else player->sell(curItem, quantity);
            }
            catch(const std::runtime_error &e){
                // std::cout<<label<<std::endl;
                errorWin->Open(noMoney, label);
                std::cerr<< "failed to trade: " << e.what() << std::endl;
            }
            ImGui::CloseCurrentPopup();
            Close();
        }
            
        // if(errorWin->IsOpen()){
        //     errorWin->Render();
        // }
        ImGui::SameLine();
        if(ImGui::Button(GET_TEXT("CANCEL").c_str(), ImVec2(100, 40))) {
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void MarketPopUp::Open(Item &i, bool iB){
    isOpen = true;
    isBuy = iB;
    curItem = i;
    // std::cout<<totalCash<<" "<<i.price<<std::endl;
    if(iB){
        totalNumber = curItem.calPossibleNumber(player->getCash());
        quantity = totalNumber;
    }
    else{
        totalNumber = player->getItemNumber(curItem);
        quantity = totalNumber;
    }
}


MarketPopUp::MarketPopUp(std::shared_ptr<Player> p):player(p){
    availableSpace = p->getSpace();
    totalCash = p->getCash();
    noMoney = GET_TEXT("NOT_ENOUGH_MONEY");
    noSpace = GET_TEXT("NOT_ENOUGH_SPACE");
    label = GET_TEXT("PURCHASE_FAILED");
    if(totalNumber > availableSpace) totalNumber = availableSpace;
}

