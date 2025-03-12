#include "Events.h"
#include "Item.h"
#include <unordered_map>
#include "AiApi.h"
#include "Player.h"

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
        text = "未发现任何事件";
        return;
    }

    switch(it->second){
        case NewsEventNames::FakeCigerrate:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "假烟供不应求！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = "市面上突然出现好多假烟贩子！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::UsedCar:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "走私二手车市场需求爆棚，价格暴涨！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = "警方大规模查封走私车市场，价格暴跌！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::FakeWine:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "假白酒销量突增，黑市交易火爆！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = "有人喝假白酒中毒住院，市场彻底崩盘！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::FakeMakeUp:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "假冒化妆品竟意外受到消费者青睐！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = "质检部门严查假化妆品，市场价格暴跌！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::FakeCD:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "新电影热映，盗版CD卖疯了！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = "政府严厉打击盗版产业，盗版CD市场惨淡！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::ShanghaiBaby:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "上海小宝贝被奉为文学经典，销量猛增！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = "上海小宝贝被禁售，市场价格急速下滑！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        case NewsEventNames::ImportToy:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "进口玩具成为热门潮流，家长们疯狂抢购！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() * 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            else{
                text = "进口玩具曝出质量问题，市场迅速降温！";
                buttons.push_back(std::make_shared<ButtonWithAction>("确定", [ & ](){
                    item->setPrice(item->getPrice() / 5);
                    ImGui::CloseCurrentPopup();
                }));
            }
            break;

        default:
            text = "没有特殊事件发生";
            label = "确认";
            buttons.push_back(std::make_shared<ButtonWithAction>("确定", nullptr));
            std::cout << "capture" << std::endl;
            break;
    }
    // std::cout << item->price << std::endl;
}
