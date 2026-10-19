#include "BankPopUp.h"

#include "Language.h"
#include "Player.h"

namespace {
constexpr const char *kPopupId = "###fsj_bank";
}

BankPopUp::BankPopUp(std::shared_ptr<Player> p) : player(std::move(p)) {
    std::vector<ButtonWithAction> buttons = {{GET_TEXT("DEPOSIT"),
                                              [this]() {
                                                  isSaving = true;
                                                  maxMoney = player->getCash();
                                                  quantity = maxMoney;
                                                  stage = Stage::ChooseAmount;
                                                  ImGui::CloseCurrentPopup();
                                              }},
                                             {GET_TEXT("WITHDRAW"),
                                              [this]() {
                                                  isSaving = false;
                                                  maxMoney = player->getSaving();
                                                  quantity = maxMoney;
                                                  stage = Stage::ChooseAmount;
                                                  ImGui::CloseCurrentPopup();
                                              }},
                                             {GET_TEXT("CANCEL"), [this]() {
                                                  ImGui::CloseCurrentPopup();
                                                  Close();
                                              }}};
    selectWin = std::make_unique<SelectPopUp>(buttons, 3);
}

void BankPopUp::Open() {
    stage = Stage::ChooseAction;
    isOpen = true;
}

void BankPopUp::Close() {
    stage = Stage::Closed;
    isOpen = false;
}

void BankPopUp::Render() {
    if (stage == Stage::ChooseAction) {
        selectWin->Render();
        return;
    }
    if (stage != Stage::ChooseAmount)
        return;

    EnsureImGuiPopupOpen(kPopupId);
    const std::string title = GET_TEXT("BANK_TITLE") + kPopupId;
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s: %d    %s: %d", GET_TEXT("CASH").c_str(), player->getCash(), GET_TEXT("SAVINGS").c_str(),
                    player->getSaving());
        ImGui::SetNextItemWidth(200);
        ImGui::InputInt("##amount", &quantity, 1, 100);
        if (quantity < 0)
            quantity = 0;
        if (quantity > maxMoney)
            quantity = maxMoney;
        ImGui::Spacing();
        ImGui::BeginDisabled(quantity == 0);
        if (ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100, 40))) {
            if (isSaving)
                player->deposit(quantity);
            else
                player->withdraw(quantity);
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button(GET_TEXT("CANCEL").c_str(), ImVec2(100, 40))) {
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndPopup();
    }
}
