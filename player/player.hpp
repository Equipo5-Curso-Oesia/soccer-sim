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

class Player
{
public:

    Player(string team_name, MinimalSocket::Port player_port,  bool is_goalie) noexcept;
    std::optional<MinimalSocket::ReceiveStringResult> getserverMessage();

    void getServer();


protected:

private:

    string team_name;
    
    bool is_goalie;
    int player_number;

    char side;

    std::size_t message_max_size = 8192;
    MinimalSocket::Port player_port;
    MinimalSocket::Port server_port = 6000;

    MinimalSocket::Address server_udp;
    MinimalSocket::udp::Udp<true> udp_socket;

    enum class GameState {
        before_kick_off = 0,
        unknown = 1
    };
    function<GameState(string)> hashString{[](string const& str) {
        if (str == "before_kick_off") return GameState::before_kick_off;
        return GameState::unknown;
    }};
    GameState game_state = GameState::unknown;

    class RecursiveMap;
    using RecursiveTypeMap = map<string, variant<int, double, string>>;//,unique_ptr<RecursiveMap>>>;
    class RecursiveMap: public map<string, RecursiveTypeMap> {};

    RecursiveTypeMap parseServerMessage(const string& message);

    RecursiveTypeMap server_params;
    RecursiveTypeMap player_params;
    vector<RecursiveTypeMap> player_types;


};