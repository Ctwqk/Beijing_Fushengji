#include "BankPopUp.h"
#include "Player.h"
#include "SelectPopUp.h"
#include "Button.h"

void BankPopUp::Open(){
    quantity = 1;
    // std::cout<<"outer open"<<std::endl;
    selectWin->Open();
    isOpen = true;
}

void BankPopUp::Render(){
    if(selectWin->IsOpen()){
        selectWin->Render();
    }
    if(isSelected){
        ImGui::OpenPopup("银行");
    }
    if(ImGui::BeginPopupModal("银行", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputInt("##", &quantity, 1, 5);
        if(quantity < 0) quantity = 0;
        if(quantity >  maxMoney) quantity = maxMoney;
        ImGui::Spacing();
        if(ImGui::Button("确定", ImVec2(100, 40))){
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
        if(ImGui::Button("取消", ImVec2(100, 40))){
            Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
}

BankPopUp::BankPopUp(Player *p): player(p){
    std::vector<ButtonWithAction> buttons = {
        {"存款", [&](){
            isSaving = true;
            isSelected = true;
            maxMoney = player->getCash();
            quantity = maxMoney;
            selectWin->Close();
            ImGui::CloseCurrentPopup();
        }},
        {"取款", [&](){
            isSaving = false;
            isSelected = true;
            maxMoney = player->getSaving();
            quantity = maxMoney;
            selectWin->Close();
            ImGui::CloseCurrentPopup();
        }},
        {"取消", [&](){
            selectWin->Close();
            ImGui::CloseCurrentPopup();
        }}
    };
    selectWin = new SelectPopUp(buttons, 3);
}

void BankPopUp::Close(){
    isOpen = false;
    isSelected = false;
}