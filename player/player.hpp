#pragma once
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <chrono>
#include <stdexcept>
#include <map>
#include <variant>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <MinimalSocket/udp/UdpSocket.h>

using namespace std;
// Diferent types of player inherence from here
class Player
{
public:

    // Singleton constructor
    static Player& getInstance(string team_name, MinimalSocket::Port player_port, char side, bool is_goalie);
    static Player& getInstance() noexcept
    {
        if (!instance){
            std::cerr << "Player must be initialized before use" << std::endl;
            std::terminate();
        }
        return *instance;
    };
    virtual ~Player() = default;

    // Getter and setter
    char getSide() {
        return side;
    }
    int getNumber() { return player_number; }
    bool getIsGoalie() { return is_goalie; }

    // Player functions form main and other class
    virtual void play();
    void parseSense_body(int time, string const& s);

    // Public actions for roles
    void turn(double dir, bool override = false);
    void turnNeck(double dir, bool override = false);
    void dash(double power, optional<double> dir = nullopt, bool is_left = true, bool is_right = true, optional<double> powerR = nullopt, optional<double> dirR = nullopt, bool override = false);
    void kick(double power, double direction, bool override = false);
    void move(double posX, double posY, bool override = false);
    double attackDirection() const { return (side == 'l') ? 0.0 : 180.0; }
    void moveToInitial();

protected:
private:
    // Singleton vars
    Player() = default;
    Player(string team_name, int player_number, char side, bool is_goalie);
    inline static Player* instance = nullptr;

    // Basic vars
    string team_name;
    bool is_goalie;
    int player_number;
    char side;
    bool initial_positioned = false;
    
    // Parse vars
    using ScalarType = variant<int, double, std::string>;
    using VectorType = vector<ScalarType>;
    using NestedMap = map<string, variant<optional<int>, pair<int, int>>>;
    using SenseBodyType = std::variant<
        ScalarType, // tokens.size() == 1
        VectorType, // tokens.size() > 1
        NestedMap
    >;                    
    map<string, SenseBodyType> sense_body; 

    int parse_time;

    int test = 0;

    // Private methods 
    void x(string s);

    // Once per cycle, only one per cicle (now public above)
    void tackle(double powerOrAngle, bool foul, bool override = false);
    void done();
    // Not once per cycle
    void changeView(int quality, optional<int> width); // TODO: Hacerlo con enums
    void say(string s);
    void pointto(double dist, double dir);
    void notPointto();
    void attentionto(bool our_team, int number);
    void notAttentionto();

    //‘before kick off’ mode, players can turn and move, but they cannot dash
    //If during a step, several players kick the ball, all the kicks are applied to the ball and a resulting acceleration is calculated. If the resulting acceleration is larger than the maximum acceleration for the ball, acceleration is normalized to its maximum value
    
};

//void catchGoalie(double dir, bool override = false); // Only for goalie player