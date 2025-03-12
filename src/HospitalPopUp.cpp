#include "HospitalPopUp.h"
#include "TextPopUp.h"
#include "Language.h"

HospitalPopUp::HospitalPopUp(const std::shared_ptr<Player> &p):player(p){
    maxCure = Player::UPPER_LIMIT - player->getHealth();
    
}


void HospitalPopUp::Render(){
    if(isOpen){
        ImGui::OpenPopup(GET_TEXT("HOSPITAL_TITLE").c_str());
    }
    if(ImGui::BeginPopupModal(GET_TEXT("HOSPITAL_TITLE").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        if(player->getCash() < PRICE_PER_HEALTH){
            errorWin->Open(GET_TEXT("NO_MONEY_DOCTOR"), GET_TEXT("NO_MONEY"), nullptr);
            ImGui::CloseCurrentPopup();
            Close();
        }
        else {
            ImGui::InputInt("##", &quantity, 1, 5);
            if(quantity <0) quantity = 0;
            if(quantity > maxCure) quantity = maxCure;
            ImGui::Spacing();
            if(ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100, 40))){
                player->setCash(player->getCash() - quantity * PRICE_PER_HEALTH);
                player->setHealth(player->getHealth() + quantity);
                ImGui::CloseCurrentPopup();
                Close();
            }
            ImGui::SameLine();
            if(ImGui::Button(GET_TEXT("TOO_EXPENSIVE").c_str(), ImVec2(200, 40))){
                ImGui::CloseCurrentPopup();
                Close();
            }
        }
        ImGui::EndPopup();
    }
}

void HospitalPopUp::Open(){
    isOpen = true;
    quantity = Player::UPPER_LIMIT - player->getHealth();
    int tmp = (int)floor(player->getCash() / HospitalPopUp::PRICE_PER_HEALTH);
    if(quantity > tmp){
        quantity = tmp;
    }
    maxCure = quantity;
    
}   