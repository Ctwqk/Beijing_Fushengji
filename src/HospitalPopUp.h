#ifndef HOSPITAL_H
#define HOSPITAL_H
#include "PopUp.h"
#include "Player.h"

class HospitalPopUp : public PopUp{
    private:
        Player* player;

        int quantity;
        int maxCure;
    public:
        static constexpr float PRICE_PER_HEALTH = 1750;
        HospitalPopUp(Player* player);
        void Render();
        void Open();
};




#endif