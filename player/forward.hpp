#pragma once
#include "player.hpp"
class ForwardPlayer : public Player {
public:
    using Player::Player;
    void play() override;
};
