#ifndef POPUP_H
#define POPUP_H

#include <iostream>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <memory>
class TextPopUp;
class PopUp {
protected:
    bool isOpen = false;
    std::shared_ptr<TextPopUp> errorWin;
public:
    virtual void Render() = 0;
    virtual ~PopUp() = default;
    void Open() {isOpen = true;};
    void Close() {isOpen = false;}
    bool IsOpen() { return isOpen;}
    void setTextWin(std::shared_ptr<TextPopUp> tp);
};
#endif
