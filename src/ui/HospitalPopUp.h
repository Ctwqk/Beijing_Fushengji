#ifndef HOSPITAL_H
#define HOSPITAL_H
#include "PopUp.h"
#include "Player.h"

class HospitalPopUp : public PopUp{
    private:
        std::shared_ptr<Player> player;

        int quantity;
        int maxCure;
    public:
        static constexpr float PRICE_PER_HEALTH = 1750;
        HospitalPopUp(const std::shared_ptr<Player> &player);
        void Render();
        void Open();
};


#endif