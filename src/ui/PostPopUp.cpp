#include "PostPopUp.h"
#include "Language.h"

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
        ImGui::OpenPopup(GET_TEXT("POST_OFFICE_TITLE").c_str());
    }
    if(ImGui::BeginPopupModal(GET_TEXT("POST_OFFICE_TITLE").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputInt("##", &quantity, 1, 5);
        if(quantity < 0) quantity = 0;
        if(quantity >  maxMoney) quantity = maxMoney;
        ImGui::Spacing();

        if(ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100,40))){
            player->setCash(player->getCash() - quantity);
            player->setDebt(player->getDebt() - quantity);
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button(GET_TEXT("WAIT_A_MOMENT").c_str(), ImVec2(100, 40))){
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

PostPopUp::PostPopUp(const std::shared_ptr<Player> &p):player(p){
    
}