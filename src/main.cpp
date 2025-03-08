#include <SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <vector>
#include "Player.h"
#include "Market.h"
#include "MarketPopUp.h"
#include "PopUp.h"
#include <utility>
#include "HospitalPopUp.h"
#include "PostPopUp.h"
#include "BankPopUp.h"
#include "Events.h"


std::vector<Item> allItems = {
    {"进口香烟", 206}, {"走私汽车", 15338}, {"假白酒（剧毒！）", 2634, -10}, 
    {"进口玩具", 610}, {"仿造化妆品", 235}, {"上海小宝贝（教科书）", 100000, +5}
};

std::vector<Item> marketItems = {
    {"进口香烟", 206}, {"走私汽车", 15338}, {"假白酒（剧毒！）", 2634, -10}, 
    {"进口玩具", 610}, {"仿造化妆品", 235}
};



int  TOTAL_STEP = 5;
int cash = 2000, debt = 5000, saving = 0, health = 100, reputation = 100;
int selectedMarketIndex = -1, selectedPlayerIndex = -1;
Player * player = new Player(health, cash, debt, saving, reputation, 100);
Market * market = new Market(allItems);
std::vector<std::unique_ptr<PopUp>> popUps;
std::unique_ptr<MarketPopUp>marketWin = std::make_unique<MarketPopUp>(player);
std::unique_ptr<TextPopUp>textWin = std::make_unique<TextPopUp>();
std::unique_ptr<HospitalPopUp>hospitalWin = std::make_unique<HospitalPopUp>(player);
std::unique_ptr<PostPopUp>postWin = std::make_unique<PostPopUp>(player);
std::unique_ptr<BankPopUp>bankWin = std::make_unique<BankPopUp>(player);
HospitalPopUp* hospitalWinHandle;
MarketPopUp* marketWinHandle;
TextPopUp* textWinHandle;
BankPopUp* bankWinHandle;
PostPopUp* postWinHandle;
std::vector<std::pair<Item,int>> playerInventory;
bool cityOrSubway = true;

std::vector<std::string> subwayStations = {
    "西直门", "积水潭", "东直门",
    "苹果园", "公主坟", "复兴门", "建国门",
    "长椿街", "茶文门", "北京站"
};

std::vector<std::string> cityLocations = {
    "海淀大街", "亚运村", "三元西桥",
    "八角西路", "翠微路", "府右街", "永安里",
    "玉泉宫", "永定门", "方庄"
};

std::vector<Event*> popupEvents;

void RenderMapUI(int &STEP){
    
    ImGui::BeginChild("SubwayMap", ImVec2(400, 150), true);
    ImGui::Text("北京地铁");
    int buttonPerRow = 4;
    int count = 0;
    if(cityOrSubway){
        if (ImGui::Button("我要坐地铁")){
            cityOrSubway = false;
        }
        for(const auto& station: cityLocations){
            if (ImGui::Button(station.c_str(), ImVec2(80, 30))) {
                // popup random events;
                if(STEP == TOTAL_STEP - 1 && playerInventory.size()){
                    textWinHandle->Open("明天俺就要回老家了，快把商品卖掉吧！", "回家", nullptr);
                }
                if(STEP == TOTAL_STEP){
                    // pop up : it ends
                    // close other windows
                }
                marketItems = market->newDay();
                
                player->newDay();
                STEP++;
                for(auto & i:marketItems){
                    if(i.event){
                        i.event->setTextPopUp(textWinHandle);
                        popupEvents.push_back(i.event);
                    }
                }
                for (auto &p : playerInventory){
                    if(market->askPrice(p.first)!= -1) continue;
                    p.first.setEvent();
                    if(p.first.event){
                        p.first.event->setTextPopUp(textWinHandle);
                        popupEvents.push_back(p.first.event);
                    }
                }
                textWinHandle->setEvents(popupEvents);
                popupEvents.clear();
            }
            count++;
            if (count % buttonPerRow != 0){
                ImGui::SameLine();
            }
        }
    }
    else{
        if (ImGui::Button("我要逛京城")){
            cityOrSubway = true;
        }
        for(const auto& station: subwayStations){
            if (ImGui::Button(station.c_str(), ImVec2(80, 30))) {
                // popup random events;
                if(STEP == TOTAL_STEP - 1 && playerInventory.size()){
                    textWinHandle->Open("明天俺就要回老家了，快把商品卖掉吧！", "回家", nullptr);
                }
                if(STEP == TOTAL_STEP){
                    // pop up : it ends
                    // close other windows
                }
                marketItems = market->newDay();
                player->newDay();
                STEP++;
                for(auto & i:marketItems){
                    if(i.event){
                        i.event->setTextPopUp(textWinHandle);
                        popupEvents.push_back(i.event);
                    }
                }
                for (auto &p : playerInventory){
                    try{
                        market->askPrice(p.first);
                    }
                    catch(const std::runtime_error &e){
                        p.first.setEvent();
                        if(p.first.event){
                            p.first.event->setTextPopUp(textWinHandle);
                            popupEvents.push_back(p.first.event);
                        }
                    }
                    
                }
                textWinHandle->setEvents(popupEvents);
                popupEvents.clear();
            }
            count++;
            if (count % buttonPerRow != 0){
                ImGui::SameLine();
            }
        }
    }
    ImGui::EndChild();
}


void RenderMainWindow(int &STEP) {
    if(STEP >= TOTAL_STEP){
        return;
    }
    std::string label = "北京浮生记" + std::to_string(STEP) + "/" + std::to_string(TOTAL_STEP);
    ImGui::Begin(label.c_str());
    ImGui::BeginChild("Market", ImVec2(250, 300), true);
    ImGui::Text("黑市");
    ImGui::Separator();
    ImGui::Columns(2);
    ImGui::Text("商品"); ImGui::NextColumn();
    ImGui::Text("黑市价格"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Columns(1);
    char buffer[128];
    // std::cout<<marketItems.size()<<std::endl;
    for (int i = 0; i < marketItems.size(); i++) {
        marketItems[i].price = market->askPrice(marketItems[i]);
        std::sprintf(buffer, "%-30s  %16d", marketItems[i].name.c_str(), marketItems[i].price);
        if (ImGui::Selectable(buffer, selectedMarketIndex == i) ){
            selectedMarketIndex = i;
        }
    }
    
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Spacing(); ImGui::Spacing(); 
    if (ImGui::Button("买进  =>")) {
        
        if (selectedMarketIndex >= 0 && selectedMarketIndex < marketItems.size() && marketItems[selectedMarketIndex].price <= cash) {
            marketWinHandle->Open(marketItems[selectedMarketIndex], true);
            selectedMarketIndex = -1;
            
        }
        else{
            // render non-choosen error popup
        }
    }
    ImGui::Spacing(); ImGui::Spacing();
    playerInventory = player->getInventory();
    if (ImGui::Button("<=  卖出")) {
        if (selectedPlayerIndex >= 0 && selectedPlayerIndex < playerInventory.size()) {
            marketWinHandle->Open(playerInventory[selectedPlayerIndex].first, false);
            selectedPlayerIndex = -1;
        }
        else{
            // render non-choosen error popup
        }
    }
    playerInventory = player->getInventory();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginChild("Inventory", ImVec2(250, 300), true);
    ImGui::Text("您的出租屋");
    ImGui::Separator();
    ImGui::Columns(3);
    ImGui::Text("商品"); ImGui::NextColumn();
    ImGui::Text("买进价格"); ImGui::NextColumn();
    ImGui::Text("数/100"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Columns(1);
    for (int i = 0; i < playerInventory.size(); i++) {
        if (ImGui::Selectable((playerInventory[i].first.name + "  " + std::to_string(playerInventory[i].first.price) + "  " + std::to_string(playerInventory[i].second)).c_str(), selectedPlayerIndex == i)){
            selectedPlayerIndex = i;
        }
    }
    ImGui::EndChild();

    ImGui::BeginChild("PlayerStats", ImVec2(250, 150), true);
    ImGui::Text("状态");
    ImGui::Separator();
    ImGui::Text("现金:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0, 1), "%d", player->getCash());
    ImGui::Text("存款:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0, 1), "%d", player->getSaving());
    ImGui::Text("欠债:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0, 0, 1), "%d", player->getDebt());
    ImGui::Text("健康: %d  信誉: %d", player->getHealth(), player->getReputation());
    ImGui::EndChild();

    ImGui::SameLine();

    RenderMapUI(STEP);

    if (ImGui::Button("银行")) { 
        bankWinHandle->Open();
     }
    ImGui::SameLine();
    if (ImGui::Button("医院")) { 
        hospitalWinHandle->Open();
    }
    ImGui::SameLine();
    if (ImGui::Button("邮局")) { 
        postWinHandle->Open();
     }
    ImGui::SameLine();
    if (ImGui::Button("租房中介")) { /* TODO: Open Rental Agency */ }
    ImGui::SameLine();
    if (ImGui::Button("网吧")) { /* TODO: Open Internet Café */ }

    ImGui::End();
}

void wrapUp(){
    int price;
    std::string text;
    std::cout<<playerInventory.size()<<std::endl;
    for(auto & p:playerInventory){
        price = market->askPrice(p.first);
        text = "以将您库存中剩余的" + std::to_string(p.second) + "个" + p.first.name + "出售, 获利" + std::to_string((price * p.second)) + "元";
        textWinHandle->Open(text, p.first.name, new ButtonWithAction("OK", [player_ = player, &pi = p](){
            player_->sell(pi.first, pi.second);
        }));
    }
    price = player->getCash() + player->getSaving() - player->getDebt();
    text = price >= 0? "您最终获利" + std::to_string(price) + "元":"您最终仍欠村长"+std::to_string(-price)+ "元, 村子回不去了，你开始流浪";
    textWinHandle->Open(text, "结局", new ButtonWithAction("OK", [popup = textWinHandle](){
        // popup->Close();
        // ImGui::CloseCurrentPopup();
    }));
}


void initCheckMap(){  //extern in event.h, to avoid SIGFPE crash
    check = {
        {Item("进口香烟", 206), NewsEventNames::FakeCigerrate},
        {Item("走私汽车", 15338), NewsEventNames::UsedCar},
        {Item("假白酒（剧毒！）", 2634, -10), NewsEventNames::FakeWine},
        {Item("进口玩具", 610), NewsEventNames::ImportToy},
        {Item("仿造化妆品", 235), NewsEventNames::FakeMakeUp},
        {Item("上海小宝贝（教科书）", 100000, 5), NewsEventNames::ShanghaiBaby},
        {Item("盗版CD", 100), NewsEventNames::FakeCD}
    };
}
int main(int argc, char* argv[]) {
    int STEP = 1;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_Window* window = SDL_CreateWindow("商品交易", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    glewInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../wqy-zenhei.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

    //init values
    initCheckMap();
    
    market->setPlayer(player);
    player->setMarket(market);
    marketWinHandle = marketWin.get();
    hospitalWinHandle = hospitalWin.get();
    textWinHandle = textWin.get();
    postWinHandle = postWin.get();
    bankWinHandle = bankWin.get();
    marketWinHandle->setTextWin(textWinHandle);
    hospitalWinHandle->setTextWin(textWinHandle);
    popUps.push_back(std::unique_ptr<PopUp>(std::move(marketWin)));
    popUps.push_back(std::unique_ptr<PopUp>(std::move(textWin)));
    popUps.push_back(std::unique_ptr<PopUp>(std::move(hospitalWin)));
    popUps.push_back(std::unique_ptr<PopUp>(std::move(postWin)));
    popUps.push_back(std::unique_ptr<PopUp>(std::move(bankWin)));
    bool running = true;
    SDL_Event event;
    // std::cout<<popUps.size()<<std::endl;
    marketItems = market->newDay();
    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        if(STEP == TOTAL_STEP+1){
            wrapUp();
            STEP++;
        }
        if(STEP == TOTAL_STEP){
            textWinHandle->Open("你要离开了","结束",new ButtonWithAction( "回家！", [popUp = textWinHandle, &STEP](){
            popUp -> Close();
            ImGui::CloseCurrentPopup();
            STEP++;
        }));
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        
        RenderMainWindow(STEP);
        for(int i=0;i<popUps.size();i++){
            if(popUps[i]->IsOpen()){
                popUps[i]->Render();
            }
        }
        

        ImGui::Render();
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
