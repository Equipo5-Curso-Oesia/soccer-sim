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
    template <typename T>
    static T& getInstance(string team_name, int player_number, char side, bool is_goalie)
    {
        if (instance) {
            throw std::logic_error("Player instance already exists");
        }
        instance = new T(team_name, player_number, side, is_goalie);    
        return static_cast<T&>(*instance); 
    };
    template <typename T>
    static T& getInstance()
    {
        if (!instance) {
            throw std::logic_error("Player must be initialized before use");
        }
        
        if (typeid(T) != typeid(Player)){ 
            string team_name = instance->team_name; int player_number = instance->player_number; char side = instance->side; bool is_goalie = instance->is_goalie;
            delete instance;
            instance = new T(team_name, player_number, side, is_goalie);
        }    
        return static_cast<T&>(*instance);  
    };
    ~Player() = default;

    // Getter and setter
    char getSide() {
        return side;
    }
    int getPlayerNumber() {
        return player_number;
    }

    // Player functions form main and other class
    virtual void play();
    void parseSense_body(int time, string const& s);

protected:
private:
    // Singleton vars
    Player() = default;
    Player(string team_name, int player_number, char side, bool is_goalie);
    inline static Player* instance = nullptr;

    friend class PlayerTest;
    friend class Goalkeeper;
    friend class Defender;

    // Basic vars
    string team_name;
    bool is_goalie;
    int player_number;
    char side;

    pair<double,double> posForNumber(int n){
        switch(n){
            case 1: return {-10.0 + 52.5, 10.0};   // Player 1: (-10, 0) → (42.5, 0)
            //case 1: return {-10.0 + 52.5, 0.0};   // Player 1: (-10, 0) → (42.5, 0)
            case 2: return {-40.0 + 52.5, 20.0};  // Player 2: (-40, -20) → (12.5, 20)
            case 3: return {-40.0 + 52.5, 0.0};   // Player 3: (-40, 0) → (12.5, 0)
            case 4: return {-40.0 + 52.5, -20.0}; // Player 4: (-40, 20) → (12.5, -20)
            case 5: return {-30.0 + 52.5, 30.0};  // Player 5: (-30, -30) → (22.5, 30)
            case 6: return {-30.0 + 52.5, 0.0};   // Player 6: (-30, 0) → (22.5, 0)
            case 7: return {-30.0 + 52.5, -30.0}; // Player 7: (-30, 30) → (22.5, -30)
            case 8: return {-20.0 + 52.5, 20.0};  // Player 8: (-20, -20) → (32.5, 20)
            case 9: return {-20.0 + 52.5, 0.0};   // Player 9: (-20, 0) → (32.5, 0)
            case 10: return {-20.0 + 52.5, -20.0};// Player 10: (-20, 20) → (32.5, -20)
            case 11: return {-50.0 + 52.5, 0.0}; // Player 11 Goalkeeper: (-50, 0) → (2.5, 0)
            //default: return {-20.0 + 52.5, -0.0};
        }
    }
    
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

    // Once per cycle, only one per cicle
    void turn(double dir, bool override = false);
    void turnNeck(double dir, bool override = false); // Can be exec in the same cycle as turn, dash, and kick
    void dash(double power, optional<double> dir = nullopt, bool is_left = true, bool is_right = true, optional<double> powerR = nullopt, optional<double> dirR = nullopt, bool override = false); // Only power is mandatory // maybe simplificaction
    void kick(double power, double dir, bool override = false);
    void tackle(double powerOrAngle, bool foul, bool override = false);
    void move(double posX, double posY, bool override = false); //can be executed only before kick off and after a goal
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