#pragma once
#include "player.hpp"
class DefensePlayer : public Player {
public:
    using Player::Player;
    void play() override;
};
