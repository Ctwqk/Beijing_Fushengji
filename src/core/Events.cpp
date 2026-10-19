#include "Events.h"
#include "Item.h"
#include <unordered_map>
#include "AiApi.h"
#include "Player.h"
#include "Language.h"

std::unordered_map<Item, NewsEventNames> check;


bool Event::goodThingHappen(){
    return dist(gen) >= 0.5;
}

void Event::setTextPopUp(const std::shared_ptr<TextPopUp> &tp){
    textWin = tp;   
}


// Event::Event(const std::shared_ptr<Player> &p){
//     player = p;
//     try{
//         std::vector<std::string> params = aiApi->generateRandomEvent(player->getCash(), player->getHealth(), player->getDebt());
//         text = params[0];
//         label = params[1];
//         int deltaCash = std::stoi(params[2]);
//         int deltaHealth = std::stoi(params[3]);
//         // 使用 buttons vector 存储按钮
//         buttons.push_back(std::make_shared<ButtonWithAction>("确定", [p, deltaCash, deltaHealth](){
//             p->setCash(p->getCash() + deltaCash);
//             p->setHealth(p->getHealth() + deltaHealth);
            
//         }));
//     }
//     catch(std::runtime_error &e){
//         std::cerr << e.what() << std::endl;
//     }
// }

Event::Event(const std::string& l, const std::string& t, std::vector<std::shared_ptr<ButtonWithAction>> bs)
    : label(l), text(t), gen(std::random_device{}()), dist(0.0, 1.0)
{
    buttons = bs;
}

Event::Event(const std::string& l, const std::string& t, std::shared_ptr<ButtonWithAction> b)
    : label(l), text(t), gen(std::random_device{}()), dist(0.0, 1.0)
{
    buttons .push_back(b);
}

Event::Event(Item* itm)
    : gen(std::random_device{}()), item(itm), dist(0.0, 1.0)
{
    text = "";
    label = "";
    auto it = check.find(*itm);
    if(it == check.end()){
        text = GET_TEXT("NO_EVENT");
        return;
    }

    switch(it->second){
        case NewsEventNames::FakeCigerrate:
            label = GET_TEXT("NEWS_EVENT");
            if(goodThingHappen()){
                text = GET_TEXT("FAKE_CIG_UP");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = GET_TEXT("FAKE_CIG_DOWN");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::UsedCar:
            label = GET_TEXT("NEWS_EVENT");
            if(goodThingHappen()){
                text = GET_TEXT("USED_CAR_UP");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = GET_TEXT("USED_CAR_DOWN");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::FakeWine:
            label = GET_TEXT("NEWS_EVENT");
            if(goodThingHappen()){
                text = GET_TEXT("FAKE_WINE_UP");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = GET_TEXT("FAKE_WINE_DOWN");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::FakeMakeUp:
            label = GET_TEXT("NEWS_EVENT");
            if(goodThingHappen()){
                text = GET_TEXT("FAKE_MAKEUP_UP");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = GET_TEXT("FAKE_MAKEUP_DOWN");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::FakeCD:
            label = GET_TEXT("NEWS_EVENT");
            if(goodThingHappen()){
                text = GET_TEXT("FAKE_CD_UP");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = GET_TEXT("FAKE_CD_DOWN");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::ShanghaiBaby:
            label = GET_TEXT("NEWS_EVENT");
            if(goodThingHappen()){
                text = GET_TEXT("SHANGHAI_BABY_UP");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = GET_TEXT("SHANGHAI_BABY_DOWN");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::ImportToy:
            label = GET_TEXT("NEWS_EVENT");
            if(goodThingHappen()){
                text = GET_TEXT("IMPORT_TOY_UP");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = GET_TEXT("IMPORT_TOY_DOWN");
                buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        default:
            text = GET_TEXT("NO_SPECIAL_EVENT");
            label = GET_TEXT("CONFIRM");
            buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), nullptr));
            break;
    }
    // std::cout << item->price << std::endl;
}
