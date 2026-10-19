#ifndef BUTTON_H
#define BUTTON_H

#include <functional>
#include <string>
#include <utility>

struct ButtonWithAction {
    std::string label;
    std::function<void()> action;
    ButtonWithAction(std::string text, std::function<void()> act = nullptr)
        : label(std::move(text)), action(std::move(act)) {}
};

#endif
