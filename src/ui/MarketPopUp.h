#ifndef MARKETPOPUP_H
#define MARKETPOPUP_H

#include <memory>
#include <string>

#include "Item.h"
#include "PopUp.h"

class Player;

// Quantity dialog for buying or selling one good.
class MarketPopUp : public PopUp {
public:
    explicit MarketPopUp(std::shared_ptr<Player> p) : player(std::move(p)) {}
    void Render() override;
    // Opens the dialog; if the trade cannot happen at all the reason is shown instead.
    void Open(const Item &i, bool buying);

private:
    std::shared_ptr<Player> player;
    Item curItem;
    int unitPrice = 0;
    int maxQuantity = 0;
    int quantity = 0;
    bool isBuy = true;
};

#endif
