#include "BankPopUp.h"
#include "Player.h"
#include "Button.h"
#include "Language.h"

void BankPopUp::Open(){
    quantity = 1;
    // std::cout<<"outer open"<<std::endl;
    selectWin->Open();
    isOpen = true;
    isSelected = false;
}

void BankPopUp::Render(){

    if(selectWin->IsOpen()){
        selectWin->Render();
        // std::cout<<"selectWin open"<<std::endl;
    }
    if(isSelected){
        selectWin->Close();
        ImGui::OpenPopup(GET_TEXT("BANK_TITLE").c_str());
    }
    // ImGui::SetNextWindowSize(ImVec2(400, 300));
    if(ImGui::BeginPopupModal(GET_TEXT("BANK_TITLE").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputInt("##", &quantity, 1, 5);
        if(quantity < 0) quantity = 0;
        if(quantity >  maxMoney) quantity = maxMoney;
        ImGui::Spacing();
        if(ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100, 40))){
            if(isSaving){
                player->setCash(player->getCash() - quantity);
                player->setSaving(player->getSaving() + quantity);
            }
            else{
                player->setCash(player->getCash() + quantity);
                player->setSaving(player->getSaving() - quantity);
            }
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button(GET_TEXT("CANCEL").c_str(), ImVec2(100, 40))){
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
}

BankPopUp::BankPopUp(const std::shared_ptr<Player> &p): player(p){
    std::vector<ButtonWithAction> buttons = {
        {GET_TEXT("DEPOSIT"), [&](){
            isSaving = true;
            isSelected = true;
            maxMoney = player->getCash();
            quantity = maxMoney;
            selectWin->Close();
            ImGui::CloseCurrentPopup();
        }},
        {GET_TEXT("WITHDRAW"), [&](){
            isSaving = false;
            isSelected = true;
            maxMoney = player->getSaving();
            quantity = maxMoney;
            selectWin->Close();
            ImGui::CloseCurrentPopup();
        }},
        {GET_TEXT("CANCEL"), [&](){
            selectWin->Close();
            ImGui::CloseCurrentPopup();
        }}
    };
    selectWin = std::make_unique<SelectPopUp>(buttons, 3);
}

void BankPopUp::Close(){
    isOpen = false;
    isSelected = false;
}