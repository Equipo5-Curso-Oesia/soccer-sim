#pragma once
#include "player.hpp"
class GoalkeeperPlayer : public Player {
public:
    using Player::Player; // inherit constructor
    void play() override;
};
