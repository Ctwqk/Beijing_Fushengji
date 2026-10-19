#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <memory>

#include "PopUp.h"

class Player;

class HospitalPopUp : public PopUp {
public:
    explicit HospitalPopUp(std::shared_ptr<Player> p) : player(std::move(p)) {}
    void Render() override;
    void Open();

private:
    std::shared_ptr<Player> player;
    int quantity = 0;
    int maxCure = 0;
};

#endif
