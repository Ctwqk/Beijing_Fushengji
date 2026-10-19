#include "TextPopUp.h"

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
            ImGui::PushID(static_cast<int>(i));
            const bool clicked = ImGui::Button(buttons[i]->label.c_str(), ImVec2(-1, 36));
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
