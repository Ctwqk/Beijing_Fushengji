#include "TextPopUp.h"

#include <algorithm>

#include "Events.h"
#include "Language.h"

namespace {
constexpr const char *kPopupId = "###fsj_event";
}

void TextPopUp::Render() {
    if (events.empty()) {
        Close();
        return;
    }
    EnsureImGuiPopupOpen(kPopupId);

    const std::shared_ptr<Event> current = events.front();
    const std::string title = current->label + kPopupId;
    ImGui::SetNextWindowSizeConstraints(ImVec2(320, 0), ImVec2(560, 400));
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 340.0f);
        ImGui::TextWrapped("%s", current->text.c_str());
        ImGui::PopTextWrapPos();
        ImGui::Spacing();

        auto buttons = current->buttons;  // the action may modify the queue
        if (buttons.empty())
            buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM")));
        for (size_t i = 0; i < buttons.size(); ++i) {
            const std::string &label = buttons[i]->label;
            const ImVec2 padding = ImGui::GetStyle().FramePadding;
            const float width = std::max(1.0f, ImGui::GetContentRegionAvail().x - 1.0f);
            const float wrapWidth = std::max(1.0f, width - 2.0f * padding.x);
            const ImVec2 textSize = ImGui::CalcTextSize(label.c_str(), nullptr, false, wrapWidth);
            const float height = std::max(36.0f, textSize.y + 2.0f * padding.y);
            ImGui::PushID(static_cast<int>(i));
            // Button() clips to a single line. Draw the complete choice as wrapped
            // text over a normally interactive button with an independent ID.
            const bool clicked = ImGui::Button("##choice", ImVec2(width, height));
            const ImVec2 pos = ImGui::GetItemRectMin();
            const ImVec2 textPos(pos.x + std::max(padding.x, (width - textSize.x) * 0.5f),
                                pos.y + (height - textSize.y) * 0.5f);
            ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textPos,
                                              ImGui::GetColorU32(ImGuiCol_Text), label.c_str(), nullptr, wrapWidth);
            ImGui::PopID();
            if (!clicked)
                continue;
            if (buttons[i]->action)
                buttons[i]->action();
            if (!events.empty() && events.front() == current)
                events.pop_front();
            if (events.empty()) {
                Close();
                ImGui::CloseCurrentPopup();
            }
            break;
        }
        ImGui::EndPopup();
    }
}

void TextPopUp::setEvents(const std::vector<std::shared_ptr<Event>> &e) {
    events.insert(events.end(), e.begin(), e.end());
    if (!events.empty())
        isOpen = true;
}

void TextPopUp::Open(const std::string &text, const std::string &label, std::shared_ptr<ButtonWithAction> b) {
    events.push_back(std::make_shared<Event>(label, text, std::move(b)));
    isOpen = true;
}
