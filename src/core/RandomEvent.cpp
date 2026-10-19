#include "RandomEvent.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "util.h"

namespace {

struct OptionDef {
    const char *zh;
    const char *en;
    int cash;
    int health;
    int reputation;
};

struct EventDef {
    const char *titleZh;
    const char *titleEn;
    const char *descZh;
    const char *descEn;
    OptionDef options[3];
};

// Every option trades something for something else, so there is no free lunch.
const EventDef kEvents[] = {
    {"黑中介",
     "Shady Agent",
     "一个黑中介说有份高薪的好工作，却要你先交一笔押金。",
     "A shady agent promises a well-paid job, but wants a deposit up front.",
     {{"乖乖交押金", "Pay the deposit", -400, 0, -10},
      {"据理力争", "Argue back", -100, -12, 5},
      {"报警举报", "Report him", 200, -3, 20}}},
    {"查暂住证",
     "Permit Check",
     "地铁口有警察在查暂住证，人群开始骚动。",
     "Police are checking residence permits at the subway entrance and the crowd stirs.",
     {{"出示证件", "Show your papers", -50, 0, 5},
      {"塞钱了事", "Slip him some cash", -300, 0, -15},
      {"拔腿就跑", "Run for it", 0, -10, -10}}},
    {"老乡来访",
     "Friend From Home",
     "一位老乡来北京找活干，想向你借点钱。",
     "A fellow villager has come to Beijing looking for work and wants to borrow money.",
     {{"借他钱", "Lend him money", -500, 0, 25},
      {"请他吃饭", "Treat him to dinner", -150, 5, 10},
      {"婉言谢绝", "Politely refuse", 0, 0, -15}}},
    {"吃坏肚子",
     "Bad Lunch",
     "路边摊的便宜午饭让你上吐下泻。",
     "A cheap street-stall lunch leaves you doubled over.",
     {{"回家休息", "Rest at home", -100, -5, 0},
      {"买药硬撑", "Buy medicine", -200, -8, 0},
      {"咬牙硬扛", "Tough it out", 0, -18, 0}}},
    {"捡到钱包",
     "Lost Wallet",
     "你在地铁站台捡到一个鼓鼓的钱包。",
     "You find a bulging wallet on the subway platform.",
     {{"还给失主", "Return it", 100, 0, 25},
      {"据为己有", "Keep the cash", 600, 0, -20},
      {"交给警察", "Hand it to the police", 0, 0, 10}}},
    {"扒手",
     "Pickpocket",
     "拥挤的车厢里，一只手悄悄伸向你的口袋。",
     "On the crowded train a hand slips toward your pocket.",
     {{"当场抓住他", "Grab his wrist", 0, -10, 15},
      {"假装没发现", "Pretend not to notice", -400, 0, 0},
      {"大声呼救", "Shout for help", -100, 0, 5}}},
    {"房东涨租",
     "Rent Hike",
     "房东毫无预兆地涨了房租。",
     "Your landlord raises the rent without warning.",
     {{"认了照付", "Pay up", -350, 0, 0},
      {"讨价还价", "Haggle", -150, -5, 5},
      {"搬去地下室", "Move to a basement", -50, -15, 0}}},
    {"发传单",
     "Flyer Gig",
     "一家小店老板想雇你发一天传单。",
     "A shop owner offers you a day of handing out flyers.",
     {{"接下兼职", "Take the gig", 300, -8, 0},
      {"要求加钱", "Ask for more pay", 450, -10, -10},
      {"婉言谢绝", "Decline and rest", 0, 5, 0}}},
    {"街头牌局",
     "Street Card Game",
     "人行道上围着一群人，正在打牌赌钱。",
     "A crowd has gathered around a gambling card game on the pavement.",
     {{"加入牌局", "Join the game", 500, -6, -15},
      {"在旁围观", "Just watch", 0, 0, 5},
      {"劝人离开", "Warn a friend away", -50, 0, 15}}},
    {"寒流来袭",
     "Cold Snap",
     "一股寒流突然袭击北京，气温骤降。",
     "A cold snap suddenly hits Beijing.",
     {{"买件棉袄", "Buy a padded coat", -300, 0, 5},
      {"喝碗热汤", "Have some hot soup", -80, -6, 0},
      {"咬牙硬撑", "Grit your teeth", 0, -16, 0}}},
    {"饭馆老板",
     "Restaurant Owner",
     "一位饭馆老板欣赏你的老实，想让你帮忙送外卖。",
     "A restaurant owner likes your honesty and wants you to help with deliveries.",
     {{"答应帮忙", "Accept", 250, -5, 10},
      {"要钱不要饭", "Ask for cash only", 400, -10, -5},
      {"谢绝好意", "Decline politely", 0, 0, 5}}},
    {"村长的人",
     "The Chief's Men",
     "村长派来的人堵住了你：“债什么时候还？”",
     "Men sent by the village chief block your way: \"When do you pay back what you owe?\"",
     {{"先给点钱", "Hand over some cash", -600, 0, 10},
      {"求宽限几天", "Beg for more time", -100, -10, -10},
      {"溜之大吉", "Slip away", 0, -8, -20}}},
};
constexpr std::size_t kEventCount = sizeof(kEvents) / sizeof(kEvents[0]);

int clampInt(long long v, int lo, int hi) {
    return static_cast<int>(std::max<long long>(lo, std::min<long long>(hi, v)));
}

// Cuts a UTF-8 string to at most `maxBytes` without splitting a code point.
std::string truncateUtf8(const std::string &s, std::size_t maxBytes) {
    if (s.size() <= maxBytes)
        return s;
    std::size_t end = maxBytes;
    while (end > 0 && (static_cast<unsigned char>(s[end]) & 0xC0) == 0x80) --end;
    return s.substr(0, end);
}

// Reads an integer from a JSON number or a string like "+300" / "-15点".
int jsonToInt(const nlohmann::json &v) {
    if (v.is_number())
        return clampInt(std::llround(v.get<double>()), -1000000, 1000000);
    if (v.is_string()) {
        const std::string s = trim(v.get<std::string>());
        std::size_t i = 0;
        bool negative = false;
        if (i < s.size() && (s[i] == '+' || s[i] == '-'))
            negative = s[i++] == '-';
        long long value = 0;
        bool any = false;
        for (; i < s.size() && s[i] >= '0' && s[i] <= '9' && value < 1000000; ++i) {
            value = value * 10 + (s[i] - '0');
            any = true;
        }
        if (any)
            return static_cast<int>(negative ? -value : value);
    }
    return 0;
}

std::string requireString(const nlohmann::json &obj, const char *field) {
    auto it = obj.find(field);
    if (it == obj.end() || !it->is_string())
        throw std::runtime_error(std::string("missing text field \"") + field + "\"");
    std::string s = trim(it->get<std::string>());
    if (s.empty())
        throw std::runtime_error(std::string("empty text field \"") + field + "\"");
    return s;
}

}  // namespace

// ---------------------------------------------------------------------------
OfflineEventSource::OfflineEventSource(std::uint32_t seed) : rng_(seed) {}

std::size_t OfflineEventSource::eventCount() {
    return kEventCount;
}

RandomEvent OfflineEventSource::generate(const EventContext &ctx) {
    std::lock_guard<std::mutex> lock(mutex_);
    const EventDef &def = kEvents[static_cast<std::size_t>(rng_.range(0, static_cast<int>(kEventCount) - 1))];
    const bool zh = ctx.lang == Language::Lang::CHINESE;
    RandomEvent ev;
    ev.title = zh ? def.titleZh : def.titleEn;
    ev.description = zh ? def.descZh : def.descEn;
    for (const OptionDef &o : def.options) {
        ev.options.push_back({zh ? o.zh : o.en, o.cash, o.health, o.reputation});
    }
    return ev;
}

// ---------------------------------------------------------------------------
FallbackEventSource::FallbackEventSource(std::shared_ptr<RandomEventSource> primary,
                                         std::shared_ptr<RandomEventSource> fallback)
    : primary_(std::move(primary)), fallback_(std::move(fallback)) {}

RandomEvent FallbackEventSource::generate(const EventContext &ctx) {
    try {
        return primary_->generate(ctx);
    } catch (const std::exception &e) {
        std::cerr << "[events] " << primary_->describe() << " source failed (" << e.what()
                  << "), using an offline event instead\n";
        return fallback_->generate(ctx);
    }
}

// ---------------------------------------------------------------------------
std::string buildEventSystemPrompt(Language::Lang lang) {
    return Language::getInstance().getText("AI_SYSTEM_PROMPT", lang);
}

std::string buildEventUserPrompt(const EventContext &ctx) {
    const Language &L = Language::getInstance();
    std::string out;
    out += L.getText("CURRENT_STATUS", ctx.lang) + ": " + L.getText("DAY", ctx.lang) + " " + std::to_string(ctx.day) + ", " +
           L.getText("CASH", ctx.lang) + " " + std::to_string(ctx.cash) + ", " + L.getText("HEALTH", ctx.lang) + " " +
           std::to_string(ctx.health) + ", " + L.getText("REPUTATION", ctx.lang) + " " + std::to_string(ctx.reputation) +
           "\n";
    out += ctx.lifeStory;
    return out;
}

void sanitizeRandomEvent(RandomEvent &event) {
    if (event.options.size() > EventLimits::MAX_OPTIONS)
        event.options.resize(EventLimits::MAX_OPTIONS);
    for (EventOption &o : event.options) {
        o.dCash = clampInt(o.dCash, -EventLimits::MAX_CASH, EventLimits::MAX_CASH);
        o.dHealth = clampInt(o.dHealth, -EventLimits::MAX_HEALTH, EventLimits::MAX_HEALTH);
        o.dReputation = clampInt(o.dReputation, -EventLimits::MAX_REPUTATION, EventLimits::MAX_REPUTATION);
    }
}

RandomEvent parseRandomEvent(const std::string &reply) {
    std::string body = reply;
    // Reasoning models may prepend a <think> block that itself contains braces.
    const std::string closeThink = "</think>";
    if (auto t = body.rfind(closeThink); t != std::string::npos)
        body = body.substr(t + closeThink.size());

    const auto first = body.find('{');
    const auto last = body.rfind('}');
    if (first == std::string::npos || last == std::string::npos || last < first)
        throw std::runtime_error("model reply contains no JSON object");

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(body.substr(first, last - first + 1));
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("model reply is not valid JSON: ") + e.what());
    }
    if (!j.is_object())
        throw std::runtime_error("model reply is not a JSON object");

    RandomEvent ev;
    ev.title = truncateUtf8(requireString(j, "title"), 120);
    ev.description = truncateUtf8(requireString(j, "description"), 600);

    auto opts = j.find("options");
    if (opts == j.end() || !opts->is_array())
        throw std::runtime_error("missing \"options\" array");
    for (const auto &o : *opts) {
        if (!o.is_object())
            continue;
        EventOption option;
        option.text = truncateUtf8(requireString(o, "text"), 120);
        if (o.contains("cash"))
            option.dCash = jsonToInt(o.at("cash"));
        if (o.contains("health"))
            option.dHealth = jsonToInt(o.at("health"));
        if (o.contains("reputation"))
            option.dReputation = jsonToInt(o.at("reputation"));
        ev.options.push_back(std::move(option));
    }
    if (ev.options.size() < 2)
        throw std::runtime_error("model reply has fewer than two options");
    sanitizeRandomEvent(ev);
    return ev;
}
