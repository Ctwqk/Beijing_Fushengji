#include "HospitalPopUp.h"
#include "TextPopUp.h"

HospitalPopUp::HospitalPopUp(Player* p):player(p){
    maxCure = Player::UPPER_LIMIT - player->getHealth();
    
}

void HospitalPopUp::Render(){
    if(isOpen){
        ImGui::OpenPopup("医院");
    }
    if(ImGui::BeginPopupModal("医院", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        if(player->getCash() < PRICE_PER_HEALTH){
            errorWin->Open("医生：没钱看什么病", "没钱了", nullptr);
            ImGui::CloseCurrentPopup();
            Close();
        }
        else {
            ImGui::InputInt("##", &quantity, 1, 5);
            if(quantity <0) quantity = 0;
            if(quantity > maxCure) quantity = maxCure;
            ImGui::Spacing();
            if(ImGui::Button("确定", ImVec2(100, 40))){
                player->setCash(player->getCash() - quantity * PRICE_PER_HEALTH);
                player->setHealth(player->getHealth() + quantity);
                ImGui::CloseCurrentPopup();
                Close();
            }
            ImGui::SameLine();
            if(ImGui::Button("你们太黑了！告你们去！", ImVec2(100, 40))){
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