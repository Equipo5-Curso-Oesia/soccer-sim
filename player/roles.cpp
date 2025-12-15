#include "roles.hpp"
#include "player.hpp"
#include "server.hpp"
#include "field.hpp"

// Helpers to access minimal Player API
namespace {
    // Formation positions: field-center coordinates
    // Player::move() applies: x' = x - 52.5, y' = -y (unless override=true)
    // So to get position (px, py) on field, pass (px + 52.5, -py) to move()
    std::pair<double,double> posForNumber(int n){
        switch(n){
            case 1: return {-10.0 + 52.5, -0.0};     // Player 1: (-10, 0) → (42.5, 0)
            case 2: return {-40.0 + 52.5, -(-20.0)}; // Player 2: (-40, -20) → (12.5, 20)
            case 3: return {-40.0 + 52.5, -0.0};     // Player 3: (-40, 0) → (12.5, 0)
            case 4: return {-40.0 + 52.5, -(20.0)};  // Player 4: (-40, 20) → (12.5, -20)
            case 5: return {-30.0 + 52.5, -(-30.0)}; // Player 5: (-30, -30) → (22.5, 30)
            case 6: return {-30.0 + 52.5, -0.0};     // Player 6: (-30, 0) → (22.5, 0)
            case 7: return {-30.0 + 52.5, -(30.0)};  // Player 7: (-30, 30) → (22.5, -30)
            case 8: return {-20.0 + 52.5, -(-20.0)}; // Player 8: (-20, -20) → (32.5, 20)
            case 9: return {-20.0 + 52.5, -0.0};     // Player 9: (-20, 0) → (32.5, 0)
            case 10: return {-20.0 + 52.5, -(20.0)}; // Player 10: (-20, 20) → (32.5, -20)
            case 11: return {-50.0 + 52.5, -0.0};    // Player 11 Goalkeeper: (-50, 0) → (2.5, 0)
            default: return {-20.0 + 52.5, -0.0};
        }
    }
    
    // Get dash direction for forward attacking toward opponent goal
    // For left ('l'): positive X (right on field, toward right goal)
    // For right ('r'): negative X (left on field, toward left goal)
    double getAttackDirection(char side) {
        return (side == 'l') ? 0.0 : 180.0;  // 0° toward right goal, 180° toward left goal
    }
}

void Goalkeeper::setInitialPosition(Player& p){
    auto [x,y] = posForNumber(p.getNumber());
    p.move(x, y);
}

void Goalkeeper::playCycle(Player& p, Server& s, Field& f){
    (void)s; (void)f;
    // Goalkeeper: face forward toward opposing goal
    double faceDir = getAttackDirection(p.getSide());
    p.turn(faceDir);
}

void Defense::setInitialPosition(Player& p){
    auto [x,y] = posForNumber(p.getNumber());
    p.move(x, y);
}

void Defense::playCycle(Player& p, Server& s, Field& f){
    (void)s; (void)f;
    // Defender: face toward opponent goal and prepare to intercept
    double faceDir = getAttackDirection(p.getSide());
    p.turn(faceDir);
}

void Forward::setInitialPosition(Player& p){
    auto [x,y] = posForNumber(p.getNumber());
    p.move(x, y);
}

void Forward::playCycle(Player& p, Server& s, Field& f){
    (void)s; (void)f;
    // Forward: dash toward opponent goal
    double attackDir = getAttackDirection(p.getSide());
    p.dash(60, attackDir);
}
