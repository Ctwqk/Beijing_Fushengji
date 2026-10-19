#include "HospitalPopUp.h"

#include "Language.h"
#include "Player.h"
#include "TextPopUp.h"

namespace {
constexpr const char *kPopupId = "###fsj_hospital";
}

void HospitalPopUp::Open() {
    if (player->getHealth() >= Player::UPPER_LIMIT) {
        if (errorWin) errorWin->Open(GET_TEXT("HEALTHY"), GET_TEXT("HOSPITAL_TITLE"));
        return;
    }
    maxCure = player->maxHealable();
    if (maxCure == 0) {
        if (errorWin) errorWin->Open(GET_TEXT("NO_MONEY_DOCTOR"), GET_TEXT("NO_MONEY"));
        return;
    }
    quantity = maxCure;
    isOpen = true;
}

void HospitalPopUp::Render() {
    EnsureImGuiPopupOpen(kPopupId);
    const std::string title = GET_TEXT("HOSPITAL_TITLE") + kPopupId;
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s: %d / %d   (%d %s / %s)", GET_TEXT("HEALTH").c_str(), player->getHealth(), Player::UPPER_LIMIT,
                    Player::PRICE_PER_HEALTH, GET_TEXT("YUAN").c_str(), GET_TEXT("HEALTH").c_str());
        ImGui::SetNextItemWidth(200);
        ImGui::InputInt("##cure", &quantity, 1, 5);
        if (quantity < 0) quantity = 0;
        if (quantity > maxCure) quantity = maxCure;
        ImGui::Text("= %d %s", quantity * Player::PRICE_PER_HEALTH, GET_TEXT("YUAN").c_str());
        ImGui::Spacing();
        ImGui::BeginDisabled(quantity == 0);
        if (ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100, 40))) {
            try {
                player->heal(quantity);
            } catch (const std::exception &) {
                // Cash changed under us; nothing to heal.
            }
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button(GET_TEXT("TOO_EXPENSIVE").c_str(), ImVec2(260, 40))) {
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndPopup();
    }
}
