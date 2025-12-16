#include "forward.hpp"
#include "server.hpp"
#include "field.hpp"

void ForwardPlayer::play(){
    moveToInitial();
    Server& s = Server::getInstance();
    auto st = s.getState();

    // Avoid illegal actions during set-play states
    if (st == Server::GameState::before_kick_off ||
        st == Server::GameState::goal_l ||
        st == Server::GameState::goal_r) {
        return;
    }

    bool our_kickoff =
        (st == Server::GameState::kick_off_l && getSide() == 'l') ||
        (st == Server::GameState::kick_off_r && getSide() == 'r');

    // Only one forward takes the kickoff; others hold position
    if ((st == Server::GameState::kick_off_l || st == Server::GameState::kick_off_r) &&
        (!our_kickoff || getNumber() != 9)) {
        return;
    }

    if (st != Server::GameState::play_on && !our_kickoff) {
        return;
    }

    Field& f = Field::getInstance();
    auto ballDist = f.getBallDist();
    auto ballDir = f.getBallDir();

    if (ballDist && ballDir) {
        cout << "Ball seen: dist=" << ballDist.value() << " dir=" << ballDir.value() << endl;
        if (ballDist.value() < 0.7) {
            if (std::abs(ballDir.value()) > 10.0) {
                turn(ballDir.value());
                return;
            }
            cout << "Attempting kick!" << endl;
            kick(100, 0, true);
            return;
        }
        dash(80, ballDir.value());
        return;
    }
    // No ball: search by turning slowly and drifting forward
    turn(20);
}
