#include "PostPopUp.h"

void PostPopUp::Open(){
    isOpen = true;
    quantity = player->getCash();
    if(quantity > player->getDebt()){
        quantity = player->getDebt();
    }
    maxMoney = quantity;
}
void PostPopUp::Render(){
    if(isOpen){
        ImGui::OpenPopup("邮局");
    }
    if(ImGui::BeginPopupModal("邮局", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputInt("##", &quantity, 1, 5);
        if(quantity < 0) quantity = 0;
        if(quantity >  maxMoney) quantity = maxMoney;
        ImGui::Spacing();

        if(ImGui:: Button("确定", ImVec2(100,40))){
            player->setCash(player->getCash() - quantity);
            player->setDebt(player->getDebt() - quantity);
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("容我缓缓", ImVec2(100, 40))){
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

PostPopUp::PostPopUp(Player* p):player(p){
    
}