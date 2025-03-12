#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <unordered_map>
#include <random>
#include <functional>
#include "Button.h"
// Forward declarations
#include "TextPopUp.h"
#include <memory>

class Item;
// class AiApi;
class Event;
class Player;

enum class GoodEventName{
    OldFriendCar,
    OldFriendWine,
    OldFriendCigerrate,
};

enum class BadEventNames{
    Ganster,
    Aunt,
    Zhuantou,
    EnvironmentProblem1,
    EnvironmentPorblem2,
    SecurityGuard
};

enum class NewsEventNames{
    None,
    ShanghaiBaby,
    FakeCigerrate,
    FakeWine,
    FakeMakeUp,
    FakeCD,
    UsedCar,
    ImportToy
};

extern std::unordered_map<Item, NewsEventNames> check;

class Item; // Forward declare to prevent circular dependency
class TextPopUp; // Forward declaration

struct Event {
private:
    
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
    std::shared_ptr<TextPopUp> textWin;
    std::shared_ptr<Player> player;
    bool goodThingHappen();
    Item* item;

public:
    
    std::string text;
    std::string label;
    std::vector<std::shared_ptr<ButtonWithAction>> buttons;
    void setTextPopUp(const std::shared_ptr<TextPopUp> &tp);
    Event(const std::string& l, const std::string& t, std::vector<std::shared_ptr<ButtonWithAction>> bs);
    Event(Item* it);
    Event(const std::string& l, const std::string& t, std::shared_ptr<ButtonWithAction> b);
    // Event(const std::shared_ptr<Player> &p);

};

#endif // EVENTS_H
