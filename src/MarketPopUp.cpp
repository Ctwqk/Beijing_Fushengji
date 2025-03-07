#include "MarketPopUp.h"
#include "Player.h"
#include "Market.h"
void MarketPopUp::Render(){
    if (isOpen){
        ImGui::OpenPopup("商店");
    }
    if (ImGui::BeginPopupModal("商店", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputInt("##", &quantity, 1, 5);
        if(quantity < 0) quantity = 0;
        if(quantity>totalNumber) quantity = totalNumber;
        ImGui::Spacing();
        // std::cout<<"hhh"<<std::endl;
        if(ImGui::Button("确定", ImVec2(100,40))) {
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
        if(ImGui::Button("取消", ImVec2(100, 40))) {
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
    totalNumber = curItem.calPossibleNumber(totalCash);
    quantity = totalNumber;
}


MarketPopUp::MarketPopUp( Player * p):player(p){
    availableSpace = p->getSpace();
    totalCash = p->getCash();
    noMoney = "没有足够金钱购买";
    noSpace = "没有足够空间存放，租赁公寓解锁更多空间";
    label = "购买失败";
    if(totalNumber > availableSpace) totalNumber = availableSpace;
}

