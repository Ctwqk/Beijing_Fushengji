#ifndef ITEM_H
#define ITEM_H
#include <string>
#include <utility>
#include <random>
#include <iostream>
#include "Events.h"
class Item{
    private:
        int getRandomPrice(){
            return (int)round(price * (0.5 + dist(gen)));
        }
        std::mt19937 gen;
        std::uniform_real_distribution<double> dist;
        static constexpr double THRESHOLD = 0.500;
    public:
        std::string name;
        std::pair<int, int> priceRange;
        int price;
        int reputationPrice;
        Event *  event;
        Item(): gen(std::random_device{}()), dist(0.0, 1.0){
            priceRange = {100, 500};
            price = getRandomPrice();
            // std::cout<<price<<std::endl;
            name = "";
            // event = new Event(*this);
        }
        Item(std::string n, int p):name(n), price(p), gen(std::random_device{}()), dist(0.0,1.0){
            priceRange = {100,500};
            reputationPrice = 0;
            // event = new Event(*this);
        }
        Item(std::string n, int p, int rp):name(n), price(p), reputationPrice(rp), gen(std::random_device{}()), dist(0.0, 1.0){
            priceRange = {100,500};
            // event = new Event(*this);
        }
        void setEvent(){
            if(dist(gen) >= THRESHOLD)
                event = new Event(this);
            else
                event = nullptr;
        }
        int calPossibleNumber(int totalCash){
            // std::cout<<totalCash<<" "<<price<<std::endl;
            // std::cout<<name<<" "<<price<<std::endl;
            // std::cout<<price<<std::endl;
            return totalCash/price;
        }
        int getPrice(){return price;}
        std::string getName(){return name;}
        void setPrice(int p){price = p;}
        void setName(std::string n){name = n;}
        bool operator==(const Item& other) const {
            return name == other.name;
        }
        void renewPrice(){
            price = getRandomPrice();
            // std::cout<<price<<std::endl;
        }
        
};
namespace std{
    template<>
    struct hash<Item> {
        size_t operator()(const Item& item) const {
            return hash<std::string>()(item.name);
        }
    };
}

#endif