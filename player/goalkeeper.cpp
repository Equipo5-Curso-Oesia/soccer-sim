#include "goalkeeper.hpp"
#include "server.hpp"
#include "field.hpp"

void GoalkeeperPlayer::play(){
    moveToInitial();
    Server& s = Server::getInstance();
    auto st = s.getState();

    if (st == Server::GameState::before_kick_off ||
        st == Server::GameState::kick_off_l ||
        st == Server::GameState::kick_off_r ||
        st == Server::GameState::goal_l ||
        st == Server::GameState::goal_r) {
        return;
    }

    Field& f = Field::getInstance();
    auto ballDist = f.getBallDist();
    auto ballDir = f.getBallDir();

    if (ballDist && ballDir) {
        if (ballDist.value() < 0.7) {
            if (std::abs(ballDir.value()) > 10.0) {
                turn(ballDir.value());
                return;
            }
            kick(80, 0, true);
            return;
        } else if (ballDist.value() < 8.0) {
            dash(80, ballDir.value());
            return;
        } else {
            turn(ballDir.value());
            return;
        }
    }

    // If no ball in sight, stay oriented toward opponent goal
    turn(attackDirection());
}
