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

// This class manage everything related with server and its configuration.
// It is a singleton class, only a single element is possible.
// When called first time args must be pressent and it will send player connection
// This class parse see server info and process positions
// To access positions getters and setters

using namespace std;
class Server
{
public:
    // Singleton constructor
    static Server& getInstance(string team_name, MinimalSocket::Port player_port, bool is_goalie) noexcept
    {
        if (instance)
            throw "you cant create another one";
        instance = new Server(team_name, player_port, is_goalie);        
        return *instance;
    };
    static Server& getInstance()noexcept
    {
        if (!instance)
            throw "must be correctly inifialzated before";
        return *instance;
    };
    ~Server() = default;


    // basura
    std::optional<MinimalSocket::ReceiveStringResult> getServerMessage();
    void getServer(bool debug = false);
    //void x(string s);

protected:
private:
    // Singleton vars
    Server(string team_name, MinimalSocket::Port player_port,  bool is_goalie) noexcept;
    inline static Server* instance = nullptr;
    
    // Connection vars
    std::size_t message_max_size = 8192;
    MinimalSocket::Port player_port;
    MinimalSocket::Port server_port = 6000;

    MinimalSocket::Address server_udp;
    MinimalSocket::udp::Udp<true> udp_socket;

    // Game state vars
    enum class GameState {
        before_kick_off = 0,
        unknown = 1
    };
    function<GameState(string)> hashString{[](string const& str) {
        if (str == "before_kick_off") return GameState::before_kick_off;
        return GameState::unknown;
    }};
    GameState game_state = GameState::unknown;
    int time;

    // Parse function
    map<string, variant<int, double, string>> parseServerMessage(const string& message);

    // Parse vars
    map<string, variant<int, double, string>> server_params;
    map<string, variant<int, double, string>> player_params;
    vector<map<string, variant<int, double, string>>> player_types;

    // Private methods
    friend class Player;
};