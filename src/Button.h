#ifndef BUTTON_H
#define BUTTON_H
#include <string>
#include <vector>
#include <functional>

struct ButtonWithAction{
    std::string label;
    std::function<void()> action;
    ButtonWithAction(std::string text, std::function<void()> act):label(text),action(act){
        
    }
};

#endif