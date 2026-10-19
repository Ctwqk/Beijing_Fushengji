#ifndef POPUP_H
#define POPUP_H

#include <imgui.h>

#include <memory>
#include <string>

class TextPopUp;

// Base class of the modal windows. A popup is "open" from Open() until Close();
// Render() is called every frame while it is open.
class PopUp {
protected:
    bool isOpen = false;
    std::shared_ptr<TextPopUp> errorWin;  // used to report problems to the player

    // ImGui forgets a modal when another popup opens at the same level, so every
    // popup re-opens itself whenever it is supposed to be open but is not.
    static void EnsureImGuiPopupOpen(const char *popupId) {
        if (!ImGui::IsPopupOpen(popupId))
            ImGui::OpenPopup(popupId);
    }

public:
    virtual void Render() = 0;
    virtual ~PopUp() = default;
    void Open() { isOpen = true; }
    virtual void Close() { isOpen = false; }
    bool IsOpen() const { return isOpen; }
    void setTextWin(std::shared_ptr<TextPopUp> tp);
};

#endif
