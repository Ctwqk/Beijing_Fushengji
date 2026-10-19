#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <utility>
#include <random>
#include <iostream>
#include "Events.h"

class Item {
private:
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
    static constexpr double THRESHOLD = 0.500;
    std::string name;
    std::pair<int, int> priceRange;
    int price;
    int reputationPrice;
    double goodProbability; 
    double badProbability;  
    std::shared_ptr<Event> event;

    int getRandomPrice() {
        return static_cast<int>(std::round(price * (0.5 + dist(gen))));
    }

public:
    Item()
        : gen(std::random_device{}()), dist(0.0, 1.0), priceRange{100, 500}, price(getRandomPrice()), name(""), event(nullptr) {}

    Item(const std::string& n, int p)
        : name(n), price(p), gen(std::random_device{}()), dist(0.0, 1.0), priceRange{100, 500}, reputationPrice(0), event(nullptr) {}

    Item(const std::string& n, int p, int rp)
        : name(n), price(p), reputationPrice(rp), gen(std::random_device{}()), dist(0.0, 1.0), priceRange{100, 500}, event(nullptr) {}

    void setEvent() {
        // std::cout<<this->getName()<<std::endl;
        if (dist(gen) >= THRESHOLD)
            event = std::make_shared<Event>(this);
        else
            event = nullptr;
    }

    int calPossibleNumber(int totalCash) const {
        return totalCash / price;
    }

    int getPrice() const { return price; }

    void setPrice(int p) { price = p; }

    std::string getName() const { return name; }

    void setName(const std::string& n) { name = n; }

    std::pair<int, int> getPriceRange() const { return priceRange; }

    void setPriceRange(const std::pair<int, int>& pr) { priceRange = pr; }

    int getReputationPrice() const { return reputationPrice; }

    void setReputationPrice(int rp) { reputationPrice = rp; }

    double getGoodProbability() const { return goodProbability; }

    void setGoodProbability(double gp) { goodProbability = gp; }

    double getBadProbability() const { return badProbability; }

    void setBadProbability(double bp) { badProbability = bp; }

    std::shared_ptr<Event> getEvent() const { return event; }

    void setEvent(std::shared_ptr<Event> e) { event = e; }

    bool operator==(const Item& other) const {
        return name == other.name;
    }

    void renewPrice() {
        price = getRandomPrice();
    }
};

namespace std {
    template<>
    struct hash<Item> {
        size_t operator()(const Item& item) const {
            return hash<std::string>()(item.getName());
        }
    };
}

#endif 
