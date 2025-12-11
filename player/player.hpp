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

    char getSide() {
        return side;
    }

    void play();
    void parseSense_body(string const& s);

protected:

private:
    
    Player() = default;
    Player(string team_name, int player_number, char side, bool is_goalie);
    inline static Player* instance = nullptr;

    string team_name;
    
    bool is_goalie;
    int player_number;

    char side = 'x';
};