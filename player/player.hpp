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
    static Player& getInstance(string team_name, MinimalSocket::Port player_port, char side, bool is_goalie) noexcept
    {
        if (instance)
            throw "you cant create another one";
        instance = new Player(team_name, player_port, side, is_goalie);    
        return *instance;
    };
    static Player& getInstance()noexcept
    {
        if (!instance)
            throw "must be correctly inifialzated before";
        return *instance;  
    };
    ~Player() = default;

    // Getter and setter
    char getSide() {
        return side;
    }

    // Player functions form main and other class
    void play();
    void parseSense_body(int time, string const& s);

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

    // Private methods
};