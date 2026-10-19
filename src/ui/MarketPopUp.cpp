#include "MarketPopUp.h"

#include "Language.h"
#include "Market.h"
#include "Player.h"
#include "TextPopUp.h"

namespace {
constexpr const char *kPopupId = "###fsj_market";

std::string tradeErrorText(const TradeError &e) {
    switch (e.reason()) {
        case TradeError::Reason::NotEnoughCash:
            return GET_TEXT("NOT_ENOUGH_MONEY");
        case TradeError::Reason::NotEnoughSpace:
            return GET_TEXT("NOT_ENOUGH_SPACE");
        case TradeError::Reason::NotEnoughStock:
            return GET_TEXT("NOT_ENOUGH_STOCK");
        case TradeError::Reason::NotOnSale:
            return GET_TEXT("NOT_ON_SALE");
        default:
            return GET_TEXT("TRADE_FAILED");
    }
}
}  // namespace

void MarketPopUp::Open(const Item &i, bool buying) {
    curItem = i;
    isBuy = buying;
    unitPrice = i.getPrice();
    if (buying) {
        maxQuantity = player->maxAffordable(i);
        if (maxQuantity == 0) {
            // Explain why instead of opening a dialog that cannot be confirmed.
            const bool noCash = player->getCash() < i.getPrice();
            if (errorWin)
                errorWin->Open(noCash ? GET_TEXT("NOT_ENOUGH_MONEY") : GET_TEXT("NOT_ENOUGH_SPACE"),
                               GET_TEXT("PURCHASE_FAILED"));
            return;
        }
    } else {
        maxQuantity = player->getItemNumber(i);
        if (maxQuantity == 0)
            return;
    }
    quantity = maxQuantity;
    isOpen = true;
}

void MarketPopUp::Render() {
    EnsureImGuiPopupOpen(kPopupId);
    const std::string title = (isBuy ? GET_TEXT("BUY") : GET_TEXT("SELL")) + " " + curItem.getName() + kPopupId;
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s: %d", GET_TEXT("PRICE").c_str(), unitPrice);
        ImGui::SetNextItemWidth(200);
        ImGui::InputInt("##quantity", &quantity, 1, 10);
        ImGui::SameLine();
        if (ImGui::Button(GET_TEXT("MAX").c_str()))
            quantity = maxQuantity;
        if (quantity < 0)
            quantity = 0;
        if (quantity > maxQuantity)
            quantity = maxQuantity;
        ImGui::Text("= %lld %s", static_cast<long long>(quantity) * unitPrice, GET_TEXT("YUAN").c_str());
        ImGui::Spacing();

        ImGui::BeginDisabled(quantity == 0);
        if (ImGui::Button(GET_TEXT("CONFIRM").c_str(), ImVec2(100, 40))) {
            try {
                if (isBuy)
                    player->buy(curItem, quantity);
                else
                    player->sell(curItem, quantity);
            } catch (const TradeError &e) {
                if (errorWin)
                    errorWin->Open(tradeErrorText(e), GET_TEXT("TRADE_ERROR_TITLE"));
            }
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button(GET_TEXT("CANCEL").c_str(), ImVec2(100, 40))) {
            ImGui::CloseCurrentPopup();
            Close();
        }
        ImGui::EndPopup();
    }
}
