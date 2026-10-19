#ifndef BANKPOPUP_H
#define BANKPOPUP_H

#include <memory>

#include "PopUp.h"
#include "SelectPopUp.h"

class Player;

// Bank: first choose deposit or withdrawal, then the amount.
class BankPopUp : public PopUp {
public:
    explicit BankPopUp(std::shared_ptr<Player> p);
    void Render() override;
    void Open();
    void Close() override;

private:
    enum class Stage { Closed, ChooseAction, ChooseAmount };

    std::shared_ptr<Player> player;
    std::unique_ptr<SelectPopUp> selectWin;
    Stage stage = Stage::Closed;
    int quantity = 0;
    int maxMoney = 0;
    bool isSaving = true;
};

#endif
