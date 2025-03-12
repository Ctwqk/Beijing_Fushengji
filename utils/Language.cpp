#include "Language.h"

std::string Language::getText(const std::string& key) const {
    auto it = textMap.find(key);
    if (it != textMap.end()) {
        return currentLang == Lang::CHINESE ? it->second.first : it->second.second;
    }
    return key; // Return the key if text not found
}

void Language::printAllKeys() {
    for (const auto& entry : textMap) {
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
    textMap["LIFE_BACKGROUND"] = {
        "您扮演一位从外地到北京谋生的青年。一开始，您只有2000元钱，并且还欠村长（一个流氓头子）5500元债务。这些债务每天的利息高达10%，如果不及时还清，村长会叫在北京的老乡们来扁您，您可能牺牲在北京街头。您决定在北京地铁各黑市倒卖各种物品来发财致富，不仅还掉债务，而且要荣登北京富人排行榜",
        "You play as a young person from outside Beijing seeking a living. Initially, you have only 2000 yuan and owe 5500 yuan to the village chief (a gangster). The debt has a daily interest rate of 10%, and if not paid on time, the chief will send fellow villagers in Beijing to beat you up. You decide to trade various items in the Beijing subway black markets to get rich, not only to pay off the debt but also to make it onto Beijing's rich list."
    };
    
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
    textMap["SELL_BEFORE_HOME"] = {"明天俺就要回老家了，快把商品卖掉吧！", "Tomorrow I'm going home, sell your goods quickly!"};
    textMap["GO_HOME"] = {"回家", "Go Home"};
    textMap["ENDING"] = {"结局", "Ending"};
    textMap["SOLD_REMAINING_ITEMS"] = {"已将您库存中剩余的", "Sold remaining "};
    textMap["ITEMS_OF"] = {"个", " items of "};
    textMap["PROFIT"] = {"元获利", " yuan profit "};
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
    textMap["FINAL_PROFIT"] = {"您最终获利", "Your final profit"};
    textMap["FINAL_DEBT"] = {"您最终仍欠村长", "You still owe the village chief"};
    textMap["CANT_GO_HOME"] = {"元, 村子回不去了，你开始流浪", "yuan, you can't go back to the village and begin wandering"};

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
    textMap["NOT_ENOUGH_SPACE"] = {"没有足够空间存放，租赁公寓解锁更多空间", "Not enough space, rent an apartment to unlock more space"};
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
    textMap["USED_CAR_UP"] = {"走私二手车市场需求爆棚，价格暴涨！", "Smuggled used car market demand soars, prices skyrocket!"};
    textMap["USED_CAR_DOWN"] = {"警方大规模查封走私车市场，价格暴跌！", "Police crack down on smuggled car market, prices plummet!"};
    textMap["FAKE_WINE_UP"] = {"假白酒销量突增，黑市交易火爆！", "Fake liquor sales surge, black market booming!"};
    textMap["FAKE_WINE_DOWN"] = {"有人喝假白酒中毒住院，市场彻底崩盘！", "Someone hospitalized from fake liquor, market crashes!"};
    textMap["FAKE_MAKEUP_UP"] = {"假冒化妆品竟意外受到消费者青睐！", "Counterfeit cosmetics unexpectedly popular with consumers!"};
    textMap["FAKE_MAKEUP_DOWN"] = {"质检部门严查假化妆品，市场价格暴跌！", "Quality inspection cracks down on fake cosmetics, prices plunge!"};
    textMap["FAKE_CD_UP"] = {"新电影热映，盗版CD卖疯了！", "New movie release, pirated CDs selling like crazy!"};
    textMap["FAKE_CD_DOWN"] = {"政府严厉打击盗版产业，盗版CD市场惨淡！", "Government crackdown on piracy, CD market crashes!"};
    textMap["SHANGHAI_BABY_UP"] = {"上海小宝贝被奉为文学经典，销量猛增！", "Shanghai Baby hailed as literary classic, sales soar!"};
    textMap["SHANGHAI_BABY_DOWN"] = {"上海小宝贝被禁售，市场价格急速下滑！", "Shanghai Baby banned, market prices plummet!"};
    textMap["IMPORT_TOY_UP"] = {"进口玩具成为热门潮流，家长们疯狂抢购！", "Imported toys become trendy, parents rush to buy!"};
    textMap["IMPORT_TOY_DOWN"] = {"进口玩具曝出质量问题，市场迅速降温！", "Quality issues found in imported toys, market cools rapidly!"};

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
    textMap["COMMA_YOUR_DECISION"] = {"，你决定", ", your dicided to "};
    textMap["MODEL_NAME"] = {"llama-3.3-70b-versatile", "Deepseek-R1-Distill-Llama-70b"};

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
    textMap["AI_EVENT_PROMPT"] = {"作为21世纪初北京生存事件生成器，请结合人物故事和当前状态（现金，健康，声誉）生成一个事件或问题，格式为：事件标题[5字]|事件描述[30字]，不要包含其他符号。", 
                                 "As an early 21st century Beijing survival event generator, please create an event based on the character's story and current status (cash, health, reputation), format: Event Title[5 chars]|Event Description[30 chars], no other symbols."};
    textMap["AI_OPTIONS_PROMPT"] = {"作为21世纪初北京生存事件生成器，请根据上面的事件和当前状态及人物故事生成三个选项，每个选项格式为：选项描述[10字]|金钱影响[±200-±800]|健康影响[±5-±20]|声誉影响[±10-±30]，各选项用#分隔，不要包含换行或其它符号。",
                                   "As a Beijing survival event generator, please create three options based on the above event and current status, format: Option[10 chars]|Money[±200-±800]|Health[±5-±20]|Reputation[±10-±30], separate with #, no line breaks or other symbols."};
    textMap["CURRENT_STATUS"] = {"当前状态", "Current Status"};
    textMap["EVENT"] = {"事件", "Event"};
    textMap["EVENT_FORMAT_ERROR"] = {"事件格式错误", "Event format error"};
    textMap["OPTIONS_COUNT_ERROR"] = {"选项数量错误", "Options count error"};
    textMap["OPTION_FORMAT_ERROR"] = {"选项格式错误", "Option format error"};
    textMap["EXPECTED_TWO_PARTS"] = {"预期2部分", "Expected 2 parts"};
    textMap["EXPECTED_THREE_OPTIONS"] = {"预期3个选项", "Expected 3 options"};
    textMap["EXPECTED_FOUR_PARTS"] = {"预期4部分", "Expected 4 parts"};
    textMap["BUT_GOT"] = {"但实际获得", "but got"};
} 