#include "PopUp.h"
#include "TextPopUp.h"

void PopUp::setTextWin(std::shared_ptr<TextPopUp> tp){
    errorWin = tp;
}