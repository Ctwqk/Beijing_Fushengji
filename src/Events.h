#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <unordered_map>
#include <random>
#include <functional>
#include "Button.h"
// Forward declarations
class Item;
class TextPopUp;

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
    TextPopUp* textWin;

    bool goodThingHappen();
    Item* item;

public:
    std::string text;
    std::string label;
    ButtonWithAction* button;
    void setTextPopUp(TextPopUp* tp);

    Event(const std::string& l, const std::string& t, ButtonWithAction* b);
    Event(Item* it);
};

#endif // EVENTS_H
