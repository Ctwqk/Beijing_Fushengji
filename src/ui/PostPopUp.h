#ifndef POSTPOPUP_H
#define POSTPOPUP_H

#include <memory>

#include "PopUp.h"

class Player;

// Post office: pay back (part of) the debt to the village chief.
class PostPopUp : public PopUp {
public:
    explicit PostPopUp(std::shared_ptr<Player> p) : player(std::move(p)) {}
    void Render() override;
    void Open();

private:
    std::shared_ptr<Player> player;
    int quantity = 0;
    int maxMoney = 0;
};

#endif
