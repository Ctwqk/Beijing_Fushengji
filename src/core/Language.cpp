#include "Language.h"

std::string Language::getText(const std::string &key) const {
    return getText(key, currentLang);
}

std::string Language::getText(const std::string &key, Lang lang) const {
    auto it = textMap.find(key);
    if (it != textMap.end()) {
        return lang == Lang::CHINESE ? it->second.first : it->second.second;
    }
    return key;  // Return the key if text not found
}

std::string Language::substitute(std::string text, const std::vector<std::string> &args) {
    // Single pass so that substituted text is never scanned for placeholders again.
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '{') {
            size_t j = i + 1;
            size_t index = 0;
            while (j < text.size() && text[j] >= '0' && text[j] <= '9' && index < 1000) {
                index = index * 10 + static_cast<size_t>(text[j] - '0');
                ++j;
            }
            if (j > i + 1 && j < text.size() && text[j] == '}' && index < args.size()) {
                out += args[index];
                i = j;
                continue;
            }
        }
        out += text[i];
    }
    return out;
}

std::string Language::format(const std::string &key, const std::vector<std::string> &args) const {
    return substitute(getText(key), args);
}

void Language::printAllKeys() {
    for (const auto &entry : textMap) {
        std::cout << entry.first << " -> (" << entry.second.first << ", " << entry.second.second << ")\n";
    }
}

void Language::initialize() {
    // Game title and main menu
    textMap["TITLE"] = {"我的北京地狱", "My Beijing Hell"};
    textMap["START"] = {"开始游戏", "Start Game"};
    textMap["LANGUAGE"] = {"语言", "Language"};

    // Player stats
    textMap["HEALTH"] = {"健康", "Health"};
    textMap["MONEY"] = {"金钱", "Money"};
    textMap["HAPPINESS"] = {"快乐", "Happiness"};

    // Locations
    textMap["HOSPITAL"] = {"医院", "Hospital"};
    textMap["BANK"] = {"银行", "Bank"};
    textMap["MARKET"] = {"市场", "Market"};
    textMap["POST"] = {"邮局", "Post Office"};

    // Common actions
    textMap["BUY"] = {"购买", "Buy"};
    textMap["SELL"] = {"出售", "Sell"};
    textMap["CONFIRM"] = {"确定", "Confirm"};
    textMap["CANCEL"] = {"取消", "Cancel"};
    textMap["CLOSE"] = {"关闭", "Close"};
    textMap["CHINESE"] = {"中文", "Chinese"};
    textMap["ENGLISH"] = {"英文", "English"};

    // 新增的翻译
    // {0} = starting cash, {1} = starting debt, {2} = daily interest on the debt in percent
    textMap["LIFE_BACKGROUND"] = {
        "您扮演一位从外地到北京谋生的青年。一开始，您只有{0}元钱，并且还欠村长（一个流氓头子）{1}元债务。这些债务每天的利息高达{2}%，如果不及时还清，村长会叫在北京的老乡们来扁您，您可能牺牲在北京街头。您决定在北京地铁各黑市倒卖各种物品来发财致富，不仅还掉债务，而且要荣登北京富人排行榜。",
        "You play as a young person from outside Beijing seeking a living. Initially, you have only {0} yuan and owe {1} yuan to the village chief (a gangster). The debt grows by {2}% every day, and if it is not paid back in time the chief will send fellow villagers in Beijing to beat you up. You decide to trade various goods in the Beijing black markets to get rich, not only to pay off the debt but also to make it onto Beijing's rich list."};

    // 城市地名
    textMap["LOC_HAIDIAN"] = {"海淀大街", "Haidian Street"};
    textMap["LOC_YAYUN"] = {"亚运村", "Asian Games Village"};
    textMap["LOC_SANYUAN"] = {"三元西桥", "Sanyuan West Bridge"};
    textMap["LOC_BAJIAO"] = {"八角西路", "Bajiao West Road"};
    textMap["LOC_CUIWEI"] = {"翠微路", "Cuiwei Road"};
    textMap["LOC_FUYOU"] = {"府右街", "Fuyou Street"};
    textMap["LOC_YONGANLI"] = {"永安里", "Yonganli"};
    textMap["LOC_YUQUAN"] = {"玉泉宫", "Yuquan Palace"};
    textMap["LOC_YONGDINGMEN"] = {"永定门", "Yongdingmen"};
    textMap["LOC_FANGZHUANG"] = {"方庄", "Fangzhuang"};

    // 地铁站名
    textMap["STATION_XIZHIMEN"] = {"西直门", "Xizhimen"};
    textMap["STATION_JISHUITAN"] = {"积水潭", "Jishuitan"};
    textMap["STATION_DONGZHIMEN"] = {"东直门", "Dongzhimen"};
    textMap["STATION_PINGGUOYUAN"] = {"苹果园", "Pingguoyuan"};
    textMap["STATION_GONGZUFEN"] = {"公主坟", "Gongzhufen"};
    textMap["STATION_FUXINGMEN"] = {"复兴门", "Fuxingmen"};
    textMap["STATION_JIANGUOMEN"] = {"建国门", "Jianguomen"};
    textMap["STATION_CHANGCHUN"] = {"长椿街", "Changchun Street"};
    textMap["STATION_CHAWEN"] = {"茶文门", "Chawenmen"};
    textMap["STATION_BEIJING"] = {"北京站", "Beijing Station"};

    // UI 元素
    textMap["BLACK_MARKET"] = {"黑市", "Black Market"};
    textMap["GOODS"] = {"商品", "Goods"};
    textMap["PRICE"] = {"价格", "Price"};
    textMap["YOUR_ROOM"] = {"您的出租屋", "Your Room"};
    textMap["STATUS"] = {"状态", "Status"};
    textMap["CASH"] = {"现金", "Cash"};
    textMap["SAVINGS"] = {"存款", "Savings"};
    textMap["DEBT"] = {"欠债", "Debt"};
    textMap["REPUTATION"] = {"信誉", "Reputation"};
    textMap["SUBWAY_MAP"] = {"北京地铁", "Beijing Subway"};
    textMap["GAME_TITLE"] = {"北京浮生记", "Beijing Life"};
    textMap["BUY_ARROW"] = {"买进  =>", "Buy  =>"};
    textMap["SELL_ARROW"] = {"<=  卖出", "<= Sell"};
    textMap["BUY_PRICE"] = {"买进价格", "Buy Price"};
    textMap["QUANTITY"] = {"数", "Qty"};
    textMap["SELL_BEFORE_HOME"] = {"明天俺就要回老家了，快把商品卖掉吧！",
                                   "Tomorrow I'm going home, sell your goods quickly!"};
    textMap["GO_HOME"] = {"回家", "Go Home"};
    textMap["ENDING"] = {"结局", "Ending"};
    textMap["DAY_COUNTER"] = {"第{0}/{1}天", "Day {0}/{1}"};
    textMap["SOLD_REMAINING"] = {"已将剩余的{0}件{1}卖出，收入{2}元。", "Sold the remaining {0} x {1} for {2} yuan."};
    textMap["YUAN"] = {"元", " yuan"};

    // 按钮文本
    textMap["TAKE_SUBWAY"] = {"我要坐地铁", "Take Subway"};
    textMap["EXPLORE_CITY"] = {"我要逛京城", "Explore City"};
    textMap["SAVE_GAME"] = {"老板来力！（存档）", "Save Game"};
    textMap["BANK"] = {"银行", "Bank"};
    textMap["HOSPITAL"] = {"医院", "Hospital"};
    textMap["POST_OFFICE"] = {"邮局", "Post Office"};
    textMap["RENTAL_AGENCY"] = {"租房中介", "Rental Agency"};
    textMap["INTERNET_CAFE"] = {"网吧", "Internet Cafe"};

    // 游戏结局相关
    textMap["FINAL_PROFIT"] = {"您带着{0}元回到了老家。", "You head home with {0} yuan."};
    textMap["FINAL_DEBT"] = {
        "您最终仍欠村长{0}元，村子回不去了，你开始流浪。",
        "You still owe the village chief {0} yuan and cannot go back to the village. You start wandering the streets."};
    textMap["DEAD_ENDING"] = {"您倒在了北京街头，再也没能站起来……",
                              "You collapsed on the streets of Beijing and never got up again..."};
    textMap["REPUTATION_LINE"] = {"信誉：{0}", "Reputation: {0}"};

    // Bank related
    textMap["BANK_TITLE"] = {"银行", "Bank"};
    textMap["DEPOSIT"] = {"存款", "Deposit"};
    textMap["WITHDRAW"] = {"取款", "Withdraw"};
    textMap["CANCEL"] = {"取消", "Cancel"};

    // Hospital related
    textMap["HOSPITAL_TITLE"] = {"医院", "Hospital"};
    textMap["NO_MONEY_DOCTOR"] = {"医生：没钱看什么病", "Doctor: No money, no treatment"};
    textMap["NO_MONEY"] = {"没钱了", "No Money"};
    textMap["TOO_EXPENSIVE"] = {"你们太黑了！告你们去！", "You're too expensive! I'll report you!"};

    // Market related
    textMap["MARKET_TITLE"] = {"商店", "Shop"};
    textMap["NOT_ENOUGH_MONEY"] = {"没有足够金钱购买", "Not enough money to buy"};
    textMap["NOT_ENOUGH_SPACE"] = {"没有足够空间存放", "Not enough space to store that"};
    textMap["PURCHASE_FAILED"] = {"购买失败", "Purchase Failed"};

    // Post Office related
    textMap["POST_OFFICE_TITLE"] = {"邮局", "Post Office"};
    textMap["WAIT_A_MOMENT"] = {"容我缓缓", "Let me think about it"};

    // News Events
    textMap["NEWS_EVENT"] = {"新闻事件", "News Event"};
    textMap["NO_EVENT"] = {"未发现任何事件", "No events found"};
    textMap["NO_SPECIAL_EVENT"] = {"没有特殊事件发生", "No special events occurred"};

    // Market Events
    textMap["FAKE_CIG_UP"] = {"假烟供不应求！", "Fake cigarettes in high demand!"};
    textMap["FAKE_CIG_DOWN"] = {"市面上突然出现好多假烟贩子！", "Suddenly many fake cigarette sellers appeared!"};
    textMap["USED_CAR_UP"] = {"走私二手车市场需求爆棚，价格暴涨！",
                              "Smuggled used car market demand soars, prices skyrocket!"};
    textMap["USED_CAR_DOWN"] = {"警方大规模查封走私车市场，价格暴跌！",
                                "Police crack down on smuggled car market, prices plummet!"};
    textMap["FAKE_WINE_UP"] = {"假白酒销量突增，黑市交易火爆！", "Fake liquor sales surge, black market booming!"};
    textMap["FAKE_WINE_DOWN"] = {"有人喝假白酒中毒住院，市场彻底崩盘！",
                                 "Someone hospitalized from fake liquor, market crashes!"};
    textMap["FAKE_MAKEUP_UP"] = {"假冒化妆品竟意外受到消费者青睐！",
                                 "Counterfeit cosmetics unexpectedly popular with consumers!"};
    textMap["FAKE_MAKEUP_DOWN"] = {"质检部门严查假化妆品，市场价格暴跌！",
                                   "Quality inspection cracks down on fake cosmetics, prices plunge!"};
    textMap["FAKE_CD_UP"] = {"新电影热映，盗版CD卖疯了！", "New movie release, pirated CDs selling like crazy!"};
    textMap["FAKE_CD_DOWN"] = {"政府严厉打击盗版产业，盗版CD市场惨淡！",
                               "Government crackdown on piracy, CD market crashes!"};
    textMap["SHANGHAI_BABY_UP"] = {"上海小宝贝被奉为文学经典，销量猛增！",
                                   "Shanghai Baby hailed as literary classic, sales soar!"};
    textMap["SHANGHAI_BABY_DOWN"] = {"上海小宝贝被禁售，市场价格急速下滑！", "Shanghai Baby banned, market prices plummet!"};
    textMap["IMPORT_TOY_UP"] = {"进口玩具成为热门潮流，家长们疯狂抢购！",
                                "Imported toys become trendy, parents rush to buy!"};
    textMap["IMPORT_TOY_DOWN"] = {"进口玩具曝出质量问题，市场迅速降温！",
                                  "Quality issues found in imported toys, market cools rapidly!"};

    // Items
    textMap["IMPORTED_CIGARETTES"] = {"进口香烟", "Imported Cigarettes"};
    textMap["SMUGGLED_CARS"] = {"走私汽车", "Smuggled Cars"};
    textMap["FAKE_LIQUOR"] = {"假白酒（剧毒！）", "Fake Liquor (Toxic!)"};
    textMap["IMPORTED_TOYS"] = {"进口玩具", "Imported Toys"};
    textMap["FAKE_COSMETICS"] = {"仿造化妆品", "Counterfeit Cosmetics"};
    textMap["SHANGHAI_BABY"] = {"上海小宝贝（教科书）", "Shanghai Baby (Textbook)"};
    textMap["PIRATED_CD"] = {"盗版CD", "Pirated CD"};

    // AI-related
    textMap["RANDOM_EVENT"] = {"随机事件", "Random Event"};
    textMap["LIFE_EVENT"] = {"生活事件", "Life Event"};
    textMap["COMMA_YOUR_DECISION"] = {"，你决定", ", you decided to "};

    // Player status messages
    textMap["HEALTH_LOW"] = {"健康状况不佳", "Poor Health"};
    textMap["DEBT_HIGH"] = {"债务过高", "High Debt"};
    textMap["MONEY_LOW"] = {"现金不足", "Low Cash"};

    // Market messages
    textMap["PRICE_CHANGE"] = {"价格变动", "Price Change"};
    textMap["MARKET_CLOSED"] = {"市场关闭", "Market Closed"};
    textMap["TRADE_SUCCESS"] = {"交易成功", "Trade Successful"};
    textMap["TRADE_FAILED"] = {"交易失败", "Trade Failed"};
    // std::cout<<textMap.size()<<std::endl;

    textMap["SELECT_TITLE"] = {"选择", "Select"};
    textMap["WHAT_TO_DO"] = {"你要干什么", "What would you like to do?"};
    // AI prompts and errors
    textMap["AI_SYSTEM_PROMPT"] = {
        "你是一款以21世纪初北京为背景的生存经营游戏的随机事件生成器。请根据人物故事和当前状态，生成一个与日常生活相关的小事件，并给出三个有取舍的选项（不能全是好处）。只输出一个JSON对象，不要输出任何其他文字或markdown，格式如下：{\"title\":\"事件标题（不超过8个字）\",\"description\":\"事件描述（不超过60个字）\",\"options\":[{\"text\":\"选项描述（不超过12个字）\",\"cash\":整数,\"health\":整数,\"reputation\":整数}]}，options恰好3项。数值范围：cash在-800到800之间，health在-20到20之间，reputation在-30到30之间。所有文字使用简体中文。",
        "You are the random-event generator of a survival and trading game set in early-2000s Beijing. Based on the character's story and current status, create one small everyday event and three options with real trade-offs (not all upside). Output exactly one JSON object and nothing else, no markdown, in this shape: {\"title\":\"event title (at most 5 words)\",\"description\":\"event description (at most 30 words)\",\"options\":[{\"text\":\"option text (at most 8 words)\",\"cash\":integer,\"health\":integer,\"reputation\":integer}]} with exactly 3 options. Ranges: cash between -800 and 800, health between -20 and 20, reputation between -30 and 30. Write all text in English."};
    textMap["CURRENT_STATUS"] = {"当前状态", "Current Status"};
    textMap["EVENT"] = {"事件", "Event"};
    textMap["DAY"] = {"天数", "Day"};
    textMap["FREE_SPACE"] = {"剩余空位", "Free space"};
    textMap["MAX"] = {"最大", "Max"};
    textMap["NO_DEBT"] = {"您现在没有欠债", "You do not owe anything right now"};
    textMap["EVENT_SOURCE_AI"] = {"随机事件：{0}", "Random events: {0}"};
    textMap["EVENT_SOURCE_OFFLINE"] = {"随机事件：离线", "Random events: offline"};
    textMap["COMING_SOON"] = {"（敬请期待）", " (coming soon)"};
    textMap["NOT_ON_SALE"] = {"今天没有这种货", "That good is not on sale today"};
    textMap["NOT_ENOUGH_STOCK"] = {"您没有这么多货", "You do not hold that many"};
    textMap["TRADE_ERROR_TITLE"] = {"交易失败", "Trade failed"};
    textMap["HEALTHY"] = {"您的身体很健康，不需要治疗", "You are in perfect health, no treatment needed"};
    textMap["SELECT_FIRST"] = {"请先选择一件商品", "Please select a good first"};
}