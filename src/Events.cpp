#include "Events.h"
#include "Item.h"
#include "TextPopUp.h"
#include <unordered_map>

std::unordered_map<Item, NewsEventNames> check;

bool Event::goodThingHappen(){
    return dist(gen) >= 0.5;
}
void Event::setTextPopUp(TextPopUp * tp){
    textWin = tp;
}
Event::Event(const std::string& l, const std::string& t, ButtonWithAction* b)
    : label(l), text(t), button(b), gen(std::random_device{}()), dist(0.0, 1.0){}

Event::Event(Item* itm)
    :gen(std::random_device{}()),item(itm), dist(0.0, 1.0), button(nullptr){

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
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price *= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            else{
                text = "市面上突然出现好多假烟贩子！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price /= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            break;

        case NewsEventNames::UsedCar:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "走私二手车市场需求爆棚，价格暴涨！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price *= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            else{
                text = "警方大规模查封走私车市场，价格暴跌！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price /= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            break;

        case NewsEventNames::FakeWine:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "假白酒销量突增，黑市交易火爆！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price *= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            else{
                text = "有人喝假白酒中毒住院，市场彻底崩盘！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price /= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            break;

        case NewsEventNames::FakeMakeUp:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "假冒化妆品竟意外受到消费者青睐！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price *= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            else{
                text = "质检部门严查假化妆品，市场价格暴跌！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price /= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            break;

        case NewsEventNames::FakeCD:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "新电影热映，盗版CD卖疯了！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price *= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            else{
                text = "政府严厉打击盗版产业，盗版CD市场惨淡！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price /= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            break;

        case NewsEventNames::ShanghaiBaby:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "上海小宝贝被奉为文学经典，销量猛增！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price *= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            else{
                text = "上海小宝贝被禁售，市场价格急速下滑！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price /= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            break;

        case NewsEventNames::ImportToy:
            label = "新闻事件";
            if(goodThingHappen()){
                text = "进口玩具成为热门潮流，家长们疯狂抢购！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price *= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            else{
                text = "进口玩具曝出质量问题，市场迅速降温！";
                button = new ButtonWithAction("确定", [ &  ](){
                    item->price /= 5;
                    
                    ImGui::CloseCurrentPopup();
                });
            }
            break;

        default:
            text = "没有特殊事件发生";
            label = "确认";
            // button = new ButtonWithAction("确定", [ ](){
            //     
            //     ImGui::CloseCurrentPopup();
            // });
            button = new ButtonWithAction("确定", nullptr);
            break;
    }
    // std::cout<<item->price<<std::endl;

}
