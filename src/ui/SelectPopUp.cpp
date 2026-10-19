#include "SelectPopUp.h"

#include "Language.h"

namespace {
constexpr const char *kPopupId = "###fsj_select";
}

SelectPopUp::SelectPopUp(std::vector<ButtonWithAction> buttons, int bpr)
    : selections(std::move(buttons)), buttonPerRow(bpr > 0 ? bpr : 1) {}

void SelectPopUp::Render() {
    EnsureImGuiPopupOpen(kPopupId);
    const std::string title = GET_TEXT("SELECT_TITLE") + kPopupId;
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", GET_TEXT("WHAT_TO_DO").c_str());
        int count = 0;
        for (size_t i = 0; i < selections.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            const bool clicked = ImGui::Button(selections[i].label.c_str(), ImVec2(90, 32));
            ImGui::PopID();
            if (clicked && selections[i].action) {
                selections[i].action();
                break;  // the action may close this popup
            }
            if (++count % buttonPerRow != 0) ImGui::SameLine();
        }
        ImGui::EndPopup();
    }
}
