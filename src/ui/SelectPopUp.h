#ifndef SELECTPOPUP_H
#define SELECTPOPUP_H

#include <string>
#include <vector>

#include "Button.h"
#include "PopUp.h"

// A modal with a row of buttons ("What would you like to do?").
class SelectPopUp : public PopUp {
public:
    SelectPopUp(std::vector<ButtonWithAction> selections, int buttonsPerRow);
    void Render() override;

private:
    std::vector<ButtonWithAction> selections;
    int buttonPerRow = 3;
};

#endif
