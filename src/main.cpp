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
#include <unordered_set>
#include <filesystem>
#include "AiApi.h"

namespace fs = std::filesystem;

class MainPage{
    private:
        SDL_Window* window;
        SDL_GLContext gl_context;

        static constexpr int TOTAL_STEP = 40;
        
        
        static const std::vector<std::string> subwayStations;
        static const std::vector<std::string> cityLocations ;

        int STEP = 0;
        int cash = 2000, debt = 5000, saving = 0, health = 100, reputation = 100;
        int selectedMarketIndex = -1, selectedPlayerIndex = -1;
        bool cityOrSubway = true;
        bool running = true;
        
        
        std::vector<std::shared_ptr<PopUp>> popUps;
        std::vector<std::shared_ptr<Event>> popupEvents;
        // 
        // HospitalPopUp* hospitalWinHandle;
        // MarketPopUp* marketWinHandle;
        // TextPopUp* textWinHandle;
        // BankPopUp* bankWinHandle;
        // PostPopUp* postWinHandle;
        std::vector<std::pair<Item,int>> playerInventory;
        std::vector<Item> allItems = {
            {"进口香烟", 206}, {"走私汽车", 15338}, {"假白酒（剧毒！）", 2634, -10}, 
            {"进口玩具", 610}, {"仿造化妆品", 235}, {"上海小宝贝（教科书）", 100000, +5}
        };

        std::vector<Item> marketItems;
        std::shared_ptr<Player> player = std::make_shared<Player>(health, cash, debt, saving, reputation, 100);
        std::shared_ptr<Market> market = std::make_shared<Market>(allItems);
        std::shared_ptr<MarketPopUp>marketWinHandle = std::make_shared<MarketPopUp>(player);
        std::shared_ptr<TextPopUp>textWinHandle = std::make_shared<TextPopUp>();
        std::shared_ptr<HospitalPopUp>hospitalWinHandle = std::make_shared<HospitalPopUp>(player);
        std::shared_ptr<PostPopUp>postWinHandle = std::make_shared<PostPopUp>(player);
        std::shared_ptr<BankPopUp>bankWinHandle = std::make_shared<BankPopUp>(player);
        std::shared_ptr<AiApi>aiApi;
        std::unordered_map<Item, bool> inPackageNotInMarket;
    protected:
        void RenderMapUI();
        void RenderMainWindow();
        void wrapUp();
        bool isRunning(){return running;}
        void SetOldFashionImGuiStyle(){
            ImGuiStyle& style = ImGui::GetStyle();

            style.WindowRounding = 1.0f;
            style.FrameRounding = 0.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabRounding = 0.0f;

            style.Colors[ImGuiCol_WindowBg] = ImColor(226, 218, 202, 255);
            style.Colors[ImGuiCol_FrameBg] = ImColor(210, 200, 185, 255);
            style.Colors[ImGuiCol_FrameBgHovered] = ImColor(220, 210, 195, 255);
            style.Colors[ImGuiCol_FrameBgActive] = ImColor(190, 180, 165, 255);

            style.Colors[ImGuiCol_PopupBg] = ImColor(226, 218, 202, 255); // Light gray popup
            style.Colors[ImGuiCol_Border] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);  // Classic gray borders

            style.Colors[ImGuiCol_Button] = ImColor(192, 182, 170, 255);
            style.Colors[ImGuiCol_ButtonHovered] = ImColor(205, 195, 180, 255);
            style.Colors[ImGuiCol_ButtonActive] = ImColor(180, 170, 155, 255);

            style.Colors[ImGuiCol_Text] = ImColor(30, 30, 30, 255); // 默认黑灰色


            style.FrameRounding = 0.0f;  
            style.WindowRounding = 0.0f;
            style.PopupRounding = 0.0f;
            style.ScrollbarRounding = 0.0f;

            style.FramePadding = ImVec2(4, 4);
            style.ItemSpacing = ImVec2(4, 4);

        }
    public:
        MainPage();
        ~MainPage();
        
        int run();
};
const std::vector<std::string> MainPage::cityLocations = {
    "海淀大街", "亚运村", "三元西桥",
    "八角西路", "翠微路", "府右街", "永安里",
    "玉泉宫", "永定门", "方庄"
};
const std::vector<std::string> MainPage::subwayStations = {
    "西直门", "积水潭", "东直门",
    "苹果园", "公主坟", "复兴门", "建国门",
    "长椿街", "茶文门", "北京站"
};

MainPage::MainPage()
    : STEP(0), selectedMarketIndex(-1), selectedPlayerIndex(-1), player(std::make_shared<Player>(health, cash, debt, saving, reputation, 100)), market(std::make_shared<Market>(allItems)),
    marketWinHandle(std::make_shared<MarketPopUp>(player)), textWinHandle(std::make_shared<TextPopUp>()), hospitalWinHandle(std::make_shared<HospitalPopUp>(player)),
    postWinHandle(std::make_shared<PostPopUp>(player)), bankWinHandle(std::make_shared<BankPopUp>(player))
{

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    window = SDL_CreateWindow("Beijing_Fushendji", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    glewInit();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
    SetOldFashionImGuiStyle();

    ImGuiIO &io = ImGui::GetIO();
    fs::path fontPath = fs::current_path().parent_path() / "Font" / "wqy-zenhei.ttc";
    fs::path apiPath = fs::current_path().parent_path() / "api_keys" / "api_key_1";
    aiApi = std::make_shared<AiApi>(apiPath, "https://api.groq.com/openai/v1/chat/completions", "llama-3.3-70b-versatile");
    // std::cout<<aiApi->run("说句吉祥话")<<std::endl;
    // std::cout<<fontPath<<std::endl;
    io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    check = {   //external in Events.h
        {Item("进口香烟", 206), NewsEventNames::FakeCigerrate},
        {Item("走私汽车", 15338), NewsEventNames::UsedCar},
        {Item("假白酒（剧毒！）", 2634, -10), NewsEventNames::FakeWine},
        {Item("进口玩具", 610), NewsEventNames::ImportToy},
        {Item("仿造化妆品", 235), NewsEventNames::FakeMakeUp},
        {Item("上海小宝贝（教科书）", 100000, 5), NewsEventNames::ShanghaiBaby},
        {Item("盗版CD", 100), NewsEventNames::FakeCD}
    };
    Player::setAiApi(aiApi);
    market->setPlayer(player);
    player->setMarket(market);
    std::string lifeBackground = "您扮演一位从外地到北京谋生的青年。一开始，您只有2000元钱，并且还欠村长（一个流氓头子）5500元债务。这些债务每天的利息高达10%，如果不及时还清，村长会叫在北京的老乡们来扁您，您可能牺牲在北京街头。您决定在北京地铁各黑市倒卖各种物品来发财致富，不仅还掉债务，而且要荣登北京富人排行榜";  // a general life background for the character in Beijing;
    marketWinHandle->setTextWin(textWinHandle);
    hospitalWinHandle->setTextWin(textWinHandle);
    player->setLifeStory(lifeBackground);
    // popUps.push_back(std::unique_ptr<PopUp>(std::move(marketWin)));
    // popUps.push_back(std::unique_ptr<PopUp>(std::move(textWin)));
    // popUps.push_back(std::unique_ptr<PopUp>(std::move(hospitalWin)));
    // popUps.push_back(std::unique_ptr<PopUp>(std::move(postWin)));
    // popUps.push_back(std::unique_ptr<PopUp>(std::move(bankWin)));
    popUps.push_back(marketWinHandle);
    popUps.push_back(textWinHandle);
    popUps.push_back(hospitalWinHandle);
    popUps.push_back(postWinHandle);
    popUps.push_back(bankWinHandle);

    marketItems = market->newDay();
    for(auto &i:allItems){
        inPackageNotInMarket[i] = false;
    }
    
}
MainPage::~MainPage(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int MainPage::run(){
    SDL_Event event;
    // std::cout<<popUps.size()<<std::endl;
    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        RenderMainWindow();
        if(!isRunning()){
            running = false;
        }
        // for(int i=0;i<mainPage.popUps.size();i++){
        //     if(mainPage.popUps[i]->IsOpen()){
        //         mainPage.popUps[i]->Render();
        //     }
        // }
        ImGui::Render();
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
    return 0;
}

void MainPage::RenderMapUI(){
    ImGui::BeginChild("SubwayMap", ImVec2(400, 200), true);
    ImGui::Text("北京地铁");
    int buttonPerRow = 4;
    int count = 0;
    auto renderButton = [&](bool &cityOrSubway){
        for(const auto& location: cityOrSubway?cityLocations:subwayStations){
            if(ImGui::Button(location.c_str(), ImVec2(80, 30))){
                if(STEP == TOTAL_STEP - 1 && playerInventory.size()){
                    // std::cout<<playerInventory.size()<<std::endl;
                    // popupEvents.push_back(std::make_shared<Event>("明天俺就要回老家了，快把商品卖掉吧！", "回家", std::make_shared<ButtonWithAction>("OK", [&](){
                    //     STEP++;
                    // })));
                    popupEvents.push_back(std::make_shared<Event>("明天俺就要回老家了，快把商品卖掉吧！", "回家", nullptr));
                }
                // for(auto & p:inPackageNotInMarket){
                //     std::cout<<p.first.getName()<<" "<<p.second<<std::endl;
                // }
                // for(auto & i:playerInventory){
                //     std::cout<<i.first.getName()<<std::endl;
                // }
                marketItems = market->newDay();
                player->newDay();
                STEP++;
                for(auto & i:marketItems){
                    if(i.getEvent()){
                        i.getEvent()->setTextPopUp(textWinHandle);
                        popupEvents.push_back(i.getEvent());
                    }
                }
                for (auto &p : playerInventory){
                    if(market->askPrice(p.first)!= -1) continue;
                    p.first.setEvent();
                    if(p.first.getEvent()){
                        p.first.getEvent()->setTextPopUp(textWinHandle);
                        popupEvents.push_back(p.first.getEvent());
                    }
                }
                try{
                    popupEvents.push_back(player->randomEvent());
                }
                catch(std::runtime_error &e){
                    std::cerr<< "failed to generate random event today"<<std::endl;
                }
                if(STEP < TOTAL_STEP)textWinHandle->setEvents(popupEvents);
                // textWinHandle->setEvents(popupEvents);
                
                popupEvents.clear();
            }
            count++;
            if (count % buttonPerRow != 0){
                ImGui::SameLine();
            }
        }
    };
    if(cityOrSubway){
        if(ImGui::Button("我要坐地铁", ImVec2(164, 50))){
            cityOrSubway = false;
        }
    }
    else{
        if(ImGui::Button("我要逛京城", ImVec2(164, 50))){
            cityOrSubway = true;
        }
    }
    renderButton(cityOrSubway);
    ImGui::SameLine();
    if(ImGui::Button("老板来力！（存档）", ImVec2(164, 30))){

    }
    ImGui::EndChild();
}

void MainPage::RenderMainWindow(){
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    std::string label = "北京浮生记" + std::to_string(STEP) + "/" + std::to_string(TOTAL_STEP);
    ImGui::Begin(label.c_str(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    ImGui::BeginChild("Market", ImVec2(300, 300), true);
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
        marketItems[i].setPrice(market->askPrice(marketItems[i]));
        std::sprintf(buffer, "%-30s  %16d", marketItems[i].getName().c_str(), marketItems[i].getPrice());
        if (ImGui::Selectable(buffer, selectedMarketIndex == i) ){
            selectedMarketIndex = i;
        }
        inPackageNotInMarket[marketItems[i]] = false;
    }
    int n = marketItems.size();
    for(auto &i:playerInventory){
        if(inPackageNotInMarket[i.first]){
            std::sprintf(buffer, "%-30s  %16d", i.first.getName().c_str(), market->askPrice(i.first));
            if (ImGui::Selectable(buffer, selectedMarketIndex == n) ){
                selectedMarketIndex = n++;
            }
            // std::cout<<"capture"<<std::endl;
            inPackageNotInMarket[i.first] = false;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Spacing(); ImGui::Spacing(); 
    if (ImGui::Button("买进  =>")) {
        // std::cout<<marketItems.size()<<std::endl;
        if (selectedMarketIndex >= 0 && selectedMarketIndex < marketItems.size() && marketItems[selectedMarketIndex].getPrice() <= cash) {
            // std::cout<<"here"<<std::endl;
            marketWinHandle->Open(marketItems[selectedMarketIndex], true);
            //std::cout<<"here"<<std::endl;
            
            inPackageNotInMarket[marketItems[selectedMarketIndex]] = true;
            // std::cout<<"here"<<std::endl;
            selectedMarketIndex = -1;
        }
        else{
            // render non-choosen error popup
        }
    }
    ImGui::Spacing(); ImGui::Spacing();
    if (ImGui::Button("<=  卖出")) {
        if (selectedPlayerIndex >= 0 && selectedPlayerIndex < playerInventory.size()) {
            marketWinHandle->Open(playerInventory[selectedPlayerIndex].first, false);
            
            inPackageNotInMarket[playerInventory[selectedPlayerIndex].first] = false;
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
        if (ImGui::Selectable((playerInventory[i].first.getName() + "      " + std::to_string(playerInventory[i].first.getPrice()) + "     " + std::to_string(playerInventory[i].second)).c_str(), selectedPlayerIndex == i)){
            selectedPlayerIndex = i;
        }
        inPackageNotInMarket[playerInventory[i].first] = true;
    }
    ImGui::EndChild();

    ImGui::BeginChild("PlayerStats", ImVec2(250, 200), true);
    ImGui::Text("状态");
    ImGui::Separator();
    ImGui::Text("现金:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0, 1), "%d", player->getCash());
    ImGui::Text("存款:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0, 1), "%d", player->getSaving());
    ImGui::Text("欠债:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0, 0, 1), "%d", player->getDebt());
    ImGui::Text("健康: %d  信誉: %d", player->getHealth(), player->getReputation());
    ImGui::EndChild();

    ImGui::SameLine();

    RenderMapUI();

    

    if (ImGui::BeginTable("bottom_table", 5, ImGuiTableFlags_SizingStretchSame))
    {
        // 只需要 1 行（row）
        ImGui::TableNextRow();

        // 这里我们用 5 个列（column），分别放 5 个按钮

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Button("银行", ImVec2(120, 40))) { 
            bankWinHandle->Open();
        }
        ImGui::TableSetColumnIndex(1);
        if (ImGui::Button("医院", ImVec2(120, 40))) { 
            hospitalWinHandle->Open();
        }
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("邮局", ImVec2(120, 40))) { 
            postWinHandle->Open();
        }
        ImGui::TableSetColumnIndex(3);
        if (ImGui::Button("租房中介", ImVec2(120, 40))) { /* TODO: Open Rental Agency */ }
        ImGui::TableSetColumnIndex(4);
        if (ImGui::Button("网吧", ImVec2(120, 40))) { /* TODO: Open Internet Café */ }

        ImGui::EndTable();
    }

    if(STEP == TOTAL_STEP){
        wrapUp();
        STEP++;
    }
    for(int i=0;i<popUps.size();i++){
        if(popUps[i]->IsOpen()){
            popUps[i]->Render();
        }
    }
    ImGui::End();
}

void MainPage::wrapUp(){
    int price;
    std::string text;
    // std::cout<<playerInventory.size()<<std::endl;
    for(auto & p:playerInventory){
        price = market->askPrice(p.first);
        text = "已将您库存中剩余的" + std::to_string(p.second) + "个" + p.first.getName() + "出售, 获利" + std::to_string((price * p.second)) + "元";
        // textWinHandle->Open(text, p.first.name, std::make_shared<ButtonWithAction>("OK", [player_ = player, &pi = p](){
        //     player_->sell(pi.first, pi.second);
        // }));
        std::cout<<text<<std::endl;
        popupEvents.push_back(std::make_shared<Event>(p.first.getName(), text, std::make_shared<ButtonWithAction>("OK", [&](){
            player->sell(p.first, p.second);
        })));
    }
    price = player->getCash() + player->getSaving() - player->getDebt();
    text = price >= 0? "您最终获利" + std::to_string(price) + "元":"您最终仍欠村长"+std::to_string(-price)+ "元, 村子回不去了，你开始流浪";
    // textWinHandle->Open(text, "结局", std::make_shared<ButtonWithAction>("OK", [popup = textWinHandle](){
    //     // popup->Close();
    //     // ImGui::CloseCurrentPopup();
    // }));
    popupEvents.push_back(std::make_shared<Event>("结局", text, std::make_shared<ButtonWithAction>("OK", [&](){
        // popup->Close();
        // ImGui::CloseCurrentPopup();
        running = false;
    })));
    textWinHandle->setEvents(popupEvents);
    popupEvents.clear();
}



int main(int argc, char* argv[]) {
    MainPage mainPage;    
    return mainPage.run();
}
