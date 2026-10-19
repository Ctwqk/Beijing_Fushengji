// Beijing Fushengji - SDL2 + OpenGL + Dear ImGui front end.
// All game rules live in the core library (src/core); this file only draws the
// state and forwards the player's clicks.
#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <cfloat>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <future>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "AiConfig.h"
#include "BankPopUp.h"
#include "EventSourceFactory.h"
#include "Events.h"
#include "Game.h"
#include "HospitalPopUp.h"
#include "Language.h"
#include "MarketPopUp.h"
#include "PopUp.h"
#include "PostPopUp.h"
#include "TextPopUp.h"

namespace fs = std::filesystem;

namespace {

struct Options {
    bool languageChosen = false;
    Language::Lang lang = Language::Lang::ENGLISH;
    bool haveSeed = false;
    std::uint32_t seed = 0;
    bool offline = false;
    int days = 40;
    std::string fontPath;
};

void printUsage(const char *argv0) {
    std::cout << "Usage: " << argv0 << " [options]\n"
              << "  --lang zh|en     skip the language selector\n"
              << "  --seed N         seed the random number generators (reproducible games)\n"
              << "  --offline        never contact an LLM, use the built-in random events\n"
              << "  --days N         length of the game in days (default 40)\n"
              << "  --font PATH      CJK font file (also: FUSHENGJI_FONT)\n"
              << "  --help           show this help\n\n"
              << "LLM events are configured through FUSHENGJI_AI_API_KEY, FUSHENGJI_AI_BASE_URL,\n"
              << "FUSHENGJI_AI_MODEL and FUSHENGJI_AI_TIMEOUT; see README.md.\n";
}

// Returns false when the program should exit (after --help or on a bad option).
bool parseArgs(int argc, char **argv, Options &opt, int &exitCode) {
    exitCode = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto value = [&](const char *name) -> const char * {
            if (i + 1 >= argc) {
                std::cerr << "missing value for " << name << "\n";
                exitCode = 2;
                return nullptr;
            }
            return argv[++i];
        };
        if (a == "--help" || a == "-h") {
            printUsage(argv[0]);
            return false;
        } else if (a == "--offline") {
            opt.offline = true;
        } else if (a == "--lang") {
            const char *v = value("--lang");
            if (!v)
                return false;
            const std::string l = v;
            if (l == "zh" || l == "cn" || l == "chinese")
                opt.lang = Language::Lang::CHINESE;
            else if (l == "en" || l == "english")
                opt.lang = Language::Lang::ENGLISH;
            else {
                std::cerr << "unknown language '" << l << "' (use zh or en)\n";
                exitCode = 2;
                return false;
            }
            opt.languageChosen = true;
        } else if (a == "--seed") {
            const char *v = value("--seed");
            if (!v)
                return false;
            opt.seed = static_cast<std::uint32_t>(std::strtoul(v, nullptr, 10));
            opt.haveSeed = true;
        } else if (a == "--days") {
            const char *v = value("--days");
            if (!v)
                return false;
            opt.days = std::atoi(v);
            if (opt.days < 1) {
                std::cerr << "--days must be at least 1\n";
                exitCode = 2;
                return false;
            }
        } else if (a == "--font") {
            const char *v = value("--font");
            if (!v)
                return false;
            opt.fontPath = v;
        } else {
            std::cerr << "unknown option '" << a << "'\n";
            printUsage(argv[0]);
            exitCode = 2;
            return false;
        }
        if (exitCode != 0)
            return false;
    }
    return true;
}

// Looks for a font with CJK glyphs: explicit path, next to the executable
// (the build copies Font/ there), the source tree, then common system locations.
std::string findFont(const std::string &explicitPath) {
    std::vector<fs::path> candidates;
    if (!explicitPath.empty())
        candidates.emplace_back(explicitPath);
    if (const char *env = std::getenv("FUSHENGJI_FONT"))
        candidates.emplace_back(env);
    if (char *base = SDL_GetBasePath()) {
        const fs::path b(base);
        SDL_free(base);
        candidates.push_back(b / "Font" / "wqy-zenhei.ttc");
        candidates.push_back(b / ".." / "share" / "beijing-fushengji" / "Font" / "wqy-zenhei.ttc");
        candidates.push_back(b / ".." / "Font" / "wqy-zenhei.ttc");
        candidates.push_back(b / ".." / ".." / "Font" / "wqy-zenhei.ttc");
    }
    candidates.emplace_back(fs::path("Font") / "wqy-zenhei.ttc");
    candidates.emplace_back("/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc");
    candidates.emplace_back("/usr/share/fonts/wenquanyi/wqy-zenhei/wqy-zenhei.ttc");
    candidates.emplace_back("C:/Windows/Fonts/msyh.ttc");
    candidates.emplace_back("/System/Library/Fonts/PingFang.ttc");
    for (const auto &c : candidates) {
        std::error_code ec;
        if (fs::is_regular_file(c, ec))
            return c.string();
    }
    return {};
}

const char *kCityKeys[] = {"LOC_HAIDIAN", "LOC_YAYUN",    "LOC_SANYUAN", "LOC_BAJIAO",      "LOC_CUIWEI",
                           "LOC_FUYOU",   "LOC_YONGANLI", "LOC_YUQUAN",  "LOC_YONGDINGMEN", "LOC_FANGZHUANG"};
const char *kSubwayKeys[] = {"STATION_XIZHIMEN",  "STATION_JISHUITAN", "STATION_DONGZHIMEN", "STATION_PINGGUOYUAN",
                             "STATION_GONGZUFEN", "STATION_FUXINGMEN", "STATION_JIANGUOMEN", "STATION_CHANGCHUN",
                             "STATION_CHAWEN",    "STATION_BEIJING"};

}  // namespace

class MainPage {
public:
    explicit MainPage(Options options);
    ~MainPage();
    int run();

private:
    struct MarketRow {
        Item item;
        bool onSale;
    };

    void startGame(Language::Lang lang);
    void renderLanguageSelector();
    void renderMainWindow();
    void renderMapUI();
    void renderPopUps();
    void travel();
    void requestRandomEvent();
    void pollRandomEvent();
    void queueEnding();
    void setupStyle();
    void loadFont();
    void showMessage(const std::string &text, const std::string &label);

    Options opt;
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;
    bool running = true;

    Language::Lang lang = Language::Lang::ENGLISH;
    std::unique_ptr<Game> game;
    EventSourceSetup events;
    std::future<RandomEvent> pendingEvent;
    bool endingQueued = false;

    std::shared_ptr<MarketPopUp> marketWin;
    std::shared_ptr<TextPopUp> textWin;
    std::shared_ptr<HospitalPopUp> hospitalWin;
    std::shared_ptr<PostPopUp> postWin;
    std::shared_ptr<BankPopUp> bankWin;

    std::string selectedMarketKey;
    std::string selectedHoldingKey;
    bool showCity = true;
    std::string currentLocation;
    std::vector<std::shared_ptr<Event>> queuedEvents;
};

MainPage::MainPage(Options options) : opt(std::move(options)) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }
#if defined(__APPLE__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    const char *glslVersion = "#version 150";
#else
    const char *glslVersion = "#version 130";
#endif
    window = SDL_CreateWindow("Beijing Fushengji", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 760, 660,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    glContext = SDL_GL_CreateContext(window);
    if (!glContext)
        throw std::runtime_error(std::string("SDL_GL_CreateContext failed: ") + SDL_GetError());
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;  // do not litter the working directory
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glslVersion);
    setupStyle();
    loadFont();

    textWin = std::make_shared<TextPopUp>();
    if (opt.languageChosen)
        startGame(opt.lang);
}

MainPage::~MainPage() {
    if (events.cancel)
        events.cancel();  // abort a running LLM request so that exit is quick
    if (pendingEvent.valid())
        pendingEvent.wait();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    if (glContext)
        SDL_GL_DeleteContext(glContext);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

void MainPage::setupStyle() {
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.FramePadding = ImVec2(4, 4);
    style.ItemSpacing = ImVec2(4, 4);

    style.Colors[ImGuiCol_WindowBg] = ImColor(226, 218, 202, 255);
    style.Colors[ImGuiCol_PopupBg] = ImColor(226, 218, 202, 255);
    style.Colors[ImGuiCol_ChildBg] = ImColor(0, 0, 0, 0);
    style.Colors[ImGuiCol_FrameBg] = ImColor(210, 200, 185, 255);
    style.Colors[ImGuiCol_FrameBgHovered] = ImColor(220, 210, 195, 255);
    style.Colors[ImGuiCol_FrameBgActive] = ImColor(190, 180, 165, 255);
    style.Colors[ImGuiCol_Border] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImColor(192, 182, 170, 255);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(205, 195, 180, 255);
    style.Colors[ImGuiCol_ButtonActive] = ImColor(180, 170, 155, 255);
    style.Colors[ImGuiCol_Header] = ImColor(190, 170, 140, 255);
    style.Colors[ImGuiCol_HeaderHovered] = ImColor(205, 185, 155, 255);
    style.Colors[ImGuiCol_HeaderActive] = ImColor(180, 160, 130, 255);
    style.Colors[ImGuiCol_TableHeaderBg] = ImColor(205, 195, 175, 255);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImColor(0, 0, 0, 12);
    style.Colors[ImGuiCol_Text] = ImColor(30, 30, 30, 255);
}

void MainPage::loadFont() {
    ImGuiIO &io = ImGui::GetIO();
    const std::string path = findFont(opt.fontPath);
    if (path.empty()) {
        std::cerr << "warning: no CJK font found (looked for Font/wqy-zenhei.ttc next to the executable, "
                     "FUSHENGJI_FONT and system fonts); Chinese text will not render. Use --font PATH.\n";
        return;
    }
    if (!io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull())) {
        std::cerr << "warning: could not load font " << path << "\n";
    }
}

void MainPage::startGame(Language::Lang l) {
    lang = l;
    LANG.setLanguage(l);

    GameConfig cfg;
    cfg.totalDays = opt.days;
    if (opt.haveSeed)
        cfg.seed = opt.seed;
    game = std::make_unique<Game>(cfg);

    AiConfig ai = AiConfig::fromEnv();
    if (opt.offline)
        ai.forceOffline = true;
    events = makeEventSource(ai, cfg.seed ^ 0x9e3779b9u);
    std::cout << LANG.format(events.ai ? "EVENT_SOURCE_AI" : "EVENT_SOURCE_OFFLINE", {events.description}) << "\n";

    auto player = game->player();
    marketWin = std::make_shared<MarketPopUp>(player);
    hospitalWin = std::make_shared<HospitalPopUp>(player);
    postWin = std::make_shared<PostPopUp>(player);
    bankWin = std::make_shared<BankPopUp>(player);
    marketWin->setTextWin(textWin);
    hospitalWin->setTextWin(textWin);
    postWin->setTextWin(textWin);

    currentLocation = GET_TEXT(kCityKeys[0]);
    selectedMarketKey.clear();
    selectedHoldingKey.clear();
    endingQueued = false;
}

int MainPage::run() {
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        if (!game) {
            renderLanguageSelector();
        } else {
            pollRandomEvent();
            renderMainWindow();
        }
        ImGui::Render();

        int w = 0, h = 0;
        SDL_GL_GetDrawableSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        SDL_Delay(2);  // keeps the loop from spinning when vsync is unavailable
    }
    return 0;
}

void MainPage::renderLanguageSelector() {
    // Title behind the language dialog.
    {
        ImDrawList *dl = ImGui::GetBackgroundDrawList();
        const ImVec2 c = ImGui::GetMainViewport()->GetCenter();
        ImFont *font = ImGui::GetFont();
        const char *lines[2] = {"北京浮生记", "Beijing Fushengji"};
        const float sizes[2] = {56.0f, 26.0f};
        const float offsets[2] = {-190.0f, -115.0f};
        for (int i = 0; i < 2; ++i) {
            const ImVec2 sz = font->CalcTextSizeA(sizes[i], FLT_MAX, 0.0f, lines[i]);
            dl->AddText(font, sizes[i], ImVec2(c.x - sz.x * 0.5f, c.y + offsets[i]), IM_COL32(70, 55, 40, 255), lines[i]);
        }
    }
    const char *id = "Choose your language / 选择语言";
    if (!ImGui::IsPopupOpen(id))
        ImGui::OpenPopup(id);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::Button("中文", ImVec2(120, 40))) {
            ImGui::CloseCurrentPopup();
            startGame(Language::Lang::CHINESE);
        } else {
            ImGui::SameLine();
            if (ImGui::Button("English", ImVec2(120, 40))) {
                ImGui::CloseCurrentPopup();
                startGame(Language::Lang::ENGLISH);
            }
        }
        ImGui::EndPopup();
    }
}

void MainPage::showMessage(const std::string &text, const std::string &label) {
    textWin->Open(text, label);
}

void MainPage::requestRandomEvent() {
    if (pendingEvent.valid())
        return;  // one request at a time
    const EventContext ctx = game->eventContext(lang);
    auto source = events.source;
    pendingEvent = std::async(std::launch::async, [source, ctx]() { return source->generate(ctx); });
}

void MainPage::pollRandomEvent() {
    if (!pendingEvent.valid() || pendingEvent.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        return;
    try {
        RandomEvent ev = pendingEvent.get();
        if (!game->isOver())
            textWin->setEvents({makeRandomEventPopup(ev, game->player())});
    } catch (const std::exception &e) {
        std::cerr << "[events] no random event this time: " << e.what() << "\n";
    }
}

void MainPage::travel() {
    if (game->isOver())
        return;
    const Game::DayReport report = game->advanceDay();
    std::vector<std::shared_ptr<Event>> popups;
    for (const auto &news : report.news) popups.push_back(makeNewsEvent(news));
    if (report.lastDay && !game->player()->getHoldings().empty())
        popups.push_back(std::make_shared<Event>(GET_TEXT("GO_HOME"), GET_TEXT("SELL_BEFORE_HOME")));
    if (report.randomEventDue)
        requestRandomEvent();
    if (!popups.empty())
        textWin->setEvents(popups);
}

void MainPage::queueEnding() {
    endingQueued = true;
    const Game::Ending ending = game->finish();
    std::vector<std::shared_ptr<Event>> popups;
    std::string text;
    if (ending.died) {
        text = GET_TEXT("DEAD_ENDING");
    } else {
        for (const auto &sale : ending.sales) {
            popups.push_back(std::make_shared<Event>(
                sale.item.getName(), LANG.format("SOLD_REMAINING", {std::to_string(sale.quantity), sale.item.getName(),
                                                                    std::to_string(sale.proceeds)})));
        }
        text = ending.netWorth >= 0 ? LANG.format("FINAL_PROFIT", {std::to_string(ending.netWorth)})
                                    : LANG.format("FINAL_DEBT", {std::to_string(-ending.netWorth)});
    }
    text += "\n\n" + LANG.format("REPUTATION_LINE", {std::to_string(game->player()->getReputation())});
    popups.push_back(std::make_shared<Event>(
        GET_TEXT("ENDING"), text, std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [this]() { running = false; })));
    textWin->setEvents(popups);
}

void MainPage::renderMapUI() {
    ImGui::BeginChild("SubwayMap", ImVec2(0, 250), true);
    ImGui::Text("%s  -  %s", GET_TEXT("SUBWAY_MAP").c_str(), currentLocation.c_str());
    ImGui::Separator();

    if (ImGui::Button(GET_TEXT(showCity ? "TAKE_SUBWAY" : "EXPLORE_CITY").c_str(), ImVec2(170, 40)))
        showCity = !showCity;
    ImGui::Spacing();

    const bool locked = game->isOver();
    ImGui::BeginDisabled(locked);
    const int perRow = 3;
    const float buttonWidth =
        (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * (perRow - 1)) / static_cast<float>(perRow);
    int count = 0;
    const size_t n = showCity ? std::size(kCityKeys) : std::size(kSubwayKeys);
    for (size_t i = 0; i < n; ++i) {
        const std::string name = GET_TEXT(showCity ? kCityKeys[i] : kSubwayKeys[i]);
        if (ImGui::Button((name + "##loc").c_str(), ImVec2(buttonWidth, 34))) {
            currentLocation = name;
            travel();
        }
        if (++count % perRow != 0 && i + 1 < n)
            ImGui::SameLine();
    }
    ImGui::EndDisabled();
    ImGui::EndChild();
}

void MainPage::renderPopUps() {
    // Popups are modal and must not fight over ImGui's popup stack: the message
    // window only shows once every other popup has been dismissed.
    bool otherOpen = false;
    for (const std::shared_ptr<PopUp> &p :
         std::initializer_list<std::shared_ptr<PopUp>>{marketWin, hospitalWin, postWin, bankWin}) {
        if (p->IsOpen()) {
            p->Render();
            otherOpen = true;
        }
    }
    if (!otherOpen && textWin->IsOpen())
        textWin->Render();
}

void MainPage::renderMainWindow() {
    int windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    const std::string title = GET_TEXT("GAME_TITLE") + "  -  " +
                              LANG.format("DAY_COUNTER", {std::to_string(game->day()), std::to_string(game->totalDays())}) +
                              "###main";
    ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

    const auto player = game->player();
    const auto market = game->market();

    // ---- market: today's offers, plus goods we hold that are not on sale today ----
    std::vector<MarketRow> rows;
    for (const Item &i : market->getAvailableItems()) rows.push_back({i, true});
    for (const auto &h : player->getHoldings())
        if (!market->isAvailable(h.item))
            rows.push_back({market->getItem(h.item), false});

    ImGui::BeginChild("Market", ImVec2(300, 290), true);
    ImGui::Text("%s", GET_TEXT("BLACK_MARKET").c_str());
    ImGui::Separator();
    if (ImGui::BeginTable("market_table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn(GET_TEXT("GOODS").c_str(), ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn(GET_TEXT("PRICE").c_str(), ImGuiTableColumnFlags_WidthStretch, 1.2f);
        ImGui::TableHeadersRow();
        for (const MarketRow &row : rows) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (!row.onSale)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
            if (ImGui::Selectable((row.item.getName() + "##m" + row.item.getKey()).c_str(),
                                  selectedMarketKey == row.item.getKey(), ImGuiSelectableFlags_SpanAllColumns)) {
                selectedMarketKey = row.item.getKey();
            }
            ImGui::TableNextColumn();
            ImGui::Text("%d", row.item.getPrice());
            if (!row.onSale)
                ImGui::PopStyleColor();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::BeginDisabled(game->isOver());
    if (ImGui::Button(GET_TEXT("BUY_ARROW").c_str(), ImVec2(84, 34))) {
        const MarketRow *sel = nullptr;
        for (const MarketRow &r : rows)
            if (r.item.getKey() == selectedMarketKey)
                sel = &r;
        if (!sel) {
            showMessage(GET_TEXT("SELECT_FIRST"), GET_TEXT("BUY"));
        } else if (!sel->onSale) {
            showMessage(GET_TEXT("NOT_ON_SALE"), GET_TEXT("BUY"));
        } else {
            marketWin->Open(sel->item, true);
        }
    }
    ImGui::Spacing();
    ImGui::Spacing();
    if (ImGui::Button(GET_TEXT("SELL_ARROW").c_str(), ImVec2(84, 34))) {
        // Sell the selected inventory line, or the selected market line when we hold it.
        std::string key = selectedHoldingKey;
        if (key.empty() && player->getItemNumber(Item(selectedMarketKey, 1)) > 0)
            key = selectedMarketKey;
        const Item *target = nullptr;
        for (const MarketRow &r : rows)
            if (r.item.getKey() == key)
                target = &r.item;
        if (!target) {
            showMessage(GET_TEXT("SELECT_FIRST"), GET_TEXT("SELL"));
        } else if (player->getItemNumber(*target) == 0) {
            showMessage(GET_TEXT("NOT_ENOUGH_STOCK"), GET_TEXT("SELL"));
        } else {
            marketWin->Open(*target, false);
        }
    }
    ImGui::EndDisabled();
    ImGui::EndGroup();

    // ---- inventory ("your room") ----
    ImGui::SameLine();
    ImGui::BeginChild("Inventory", ImVec2(0, 290), true);
    ImGui::Text("%s", GET_TEXT("YOUR_ROOM").c_str());
    ImGui::Separator();
    if (ImGui::BeginTable("inventory_table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn(GET_TEXT("GOODS").c_str(), ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn(GET_TEXT("BUY_PRICE").c_str(), ImGuiTableColumnFlags_WidthStretch, 1.6f);
        ImGui::TableSetupColumn(GET_TEXT("QUANTITY").c_str(), ImGuiTableColumnFlags_WidthStretch, 0.9f);
        ImGui::TableHeadersRow();
        for (const auto &h : player->getHoldings()) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Selectable((h.item.getName() + "##h" + h.item.getKey()).c_str(),
                                  selectedHoldingKey == h.item.getKey(), ImGuiSelectableFlags_SpanAllColumns)) {
                selectedHoldingKey = h.item.getKey();
            }
            ImGui::TableNextColumn();
            ImGui::Text("%d", h.averageCost());
            ImGui::TableNextColumn();
            ImGui::Text("%d", h.quantity);
        }
        ImGui::EndTable();
    }
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 6);
    ImGui::Text("%s: %d", GET_TEXT("FREE_SPACE").c_str(), player->getSpace());
    ImGui::EndChild();
    if (player->getItemNumber(Item(selectedHoldingKey, 1)) == 0)
        selectedHoldingKey.clear();

    // ---- status ----
    ImGui::BeginChild("PlayerStats", ImVec2(220, 250), true);
    ImGui::Text("%s", GET_TEXT("STATUS").c_str());
    ImGui::Separator();
    ImGui::Text("%s:", GET_TEXT("CASH").c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 0.5f, 0.0f, 1), "%d", player->getCash());
    ImGui::Text("%s:", GET_TEXT("SAVINGS").c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 0.5f, 0.0f, 1), "%d", player->getSaving());
    ImGui::Text("%s:", GET_TEXT("DEBT").c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1), "%d", player->getDebt());
    ImGui::Text("%s: %d / %d", GET_TEXT("HEALTH").c_str(), player->getHealth(), Player::UPPER_LIMIT);
    ImGui::Text("%s: %d", GET_TEXT("REPUTATION").c_str(), player->getReputation());
    ImGui::EndChild();

    ImGui::SameLine();
    renderMapUI();

    // ---- town buildings ----
    if (ImGui::BeginTable("bottom_table", 5, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextRow();
        ImGui::BeginDisabled(game->isOver());
        ImGui::TableSetColumnIndex(0);
        if (ImGui::Button(GET_TEXT("BANK").c_str(), ImVec2(-1, 40)))
            bankWin->Open();
        ImGui::TableSetColumnIndex(1);
        if (ImGui::Button(GET_TEXT("HOSPITAL").c_str(), ImVec2(-1, 40)))
            hospitalWin->Open();
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button(GET_TEXT("POST_OFFICE").c_str(), ImVec2(-1, 40)))
            postWin->Open();
        ImGui::EndDisabled();
        // Not implemented yet; shown greyed out so the layout matches the original game.
        ImGui::BeginDisabled();
        ImGui::TableSetColumnIndex(3);
        ImGui::Button(GET_TEXT("RENTAL_AGENCY").c_str(), ImVec2(-1, 40));
        ImGui::TableSetColumnIndex(4);
        ImGui::Button(GET_TEXT("INTERNET_CAFE").c_str(), ImVec2(-1, 40));
        ImGui::EndDisabled();
        ImGui::EndTable();
    }

    ImGui::TextDisabled("%s",
                        LANG.format(events.ai ? "EVENT_SOURCE_AI" : "EVENT_SOURCE_OFFLINE", {events.description}).c_str());

    // The game is over (last day reached or the player died): once every pending
    // message has been dismissed, settle the accounts and show the ending.
    if (game->isOver() && !endingQueued && !textWin->IsOpen() && !marketWin->IsOpen() && !bankWin->IsOpen() &&
        !hospitalWin->IsOpen() && !postWin->IsOpen()) {
        queueEnding();
    }
    renderPopUps();
    ImGui::End();
}

int main(int argc, char *argv[]) {
    Options options;
    int exitCode = 0;
    if (!parseArgs(argc, argv, options, exitCode))
        return exitCode;
    LANG.initialize();
    LANG.setLanguage(options.lang);
    try {
        MainPage page(options);
        return page.run();
    } catch (const std::exception &e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
