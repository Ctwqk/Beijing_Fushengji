#include "PostPopUp.h"

#include <algorithm>

#include "Language.h"
#include "Player.h"
#include "TextPopUp.h"

namespace {
constexpr const char *kPopupId = "###fsj_post";
}

void PostPopUp::Open() {
    if (player->getDebt() <= 0) {
        if (errorWin)
            errorWin->Open(GET_TEXT("NO_DEBT"), GET_TEXT("POST_OFFICE_TITLE"));
        return;
    }
    maxMoney = std::max(0, std::min(player->getCash(), player->getDebt()));
    quantity = maxMoney;
    isOpen = true;
}

void PostPopUp::Render() {
    EnsureImGuiPopupOpen(kPopupId);
    const std::string title = GET_TEXT("POST_OFFICE_TITLE") + kPopupId;
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s: %d    %s: %d", GET_TEXT("CASH").c_str(), player->getCash(), GET_TEXT("DEBT").c_str(),
                    player->getDebt());
        ImGui::SetNextItemWidth(200);
        ImGui::InputInt("##repay", &quantity, 1, 100);
        if (quantity < 0)
            quantity = 0;
        if (quantity > maxMoney)
            quantity = maxMoney;
        ImGui::Spacing();
        ImGui::BeginDisabled(quantity == 0);
        if (ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100, 40))) {
            player->repayDebt(quantity);
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button(GET_TEXT("WAIT_A_MOMENT").c_str(), ImVec2(150, 40))) {
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndPopup();
    }
}
