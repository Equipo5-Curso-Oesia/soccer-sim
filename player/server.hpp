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

    void getServer(bool debug = false);

    // Game state functions
    enum class GameState {
        unknown = -1,
        before_kick_off = 0,
        play_on = 1,

        kick_off_l = 2,
        kick_off_r = 3,
        kick_in_l = 4,
        kick_in_r = 5,
        free_kick_l = 6,
        free_kick_r = 7,
        corner_kick_l = 8,
        corner_kick_r = 9,
        goal_kick_l = 10,
        goal_kick_r = 11,
        penalty_kick_l = 12,
        penalty_kick_r = 13,

        goal_l = 14,
        goal_r = 15,

        drop_ball = 16,
        offside_l = 17,
        offside_r = 18,
        foul_charge_l = 19,
        foul_charge_r = 20,
        back_pass_l = 21,
        back_pass_r = 22,
        free_kick_fault_l = 23,
        free_kick_fault_r = 24,
        indirect_free_kick_l = 25,
        indirect_free_kick_r = 26,
        illegal_defense_l = 27,
        illegal_defense_r = 28,

        time_over = 29
    };
    function<GameState(string)> hashString{[](string const& str) {
        // --- Core States ---
        if (str == "before_kick_off") return GameState::before_kick_off;
        if (str == "play_on") return GameState::play_on;
        if (str == "time_over") return GameState::time_over;
        if (str == "drop_ball") return GameState::drop_ball;

        // --- Scoring & Restarts (l/r pairs) ---
        if (str == "kick_off_l") return GameState::kick_off_l;
        if (str == "kick_off_r") return GameState::kick_off_r;
        
        if (str == "kick_in_l") return GameState::kick_in_l;
        if (str == "kick_in_r") return GameState::kick_in_r;
        
        if (str == "free_kick_l") return GameState::free_kick_l;
        if (str == "free_kick_r") return GameState::free_kick_r;
        
        if (str == "corner_kick_l") return GameState::corner_kick_l;
        if (str == "corner_kick_r") return GameState::corner_kick_r;
        
        if (str == "goal_kick_l") return GameState::goal_kick_l;
        if (str == "goal_kick_r") return GameState::goal_kick_r;
        
        if (str == "penalty_kick_l") return GameState::penalty_kick_l;
        if (str == "penalty_kick_r") return GameState::penalty_kick_r;

        if (str == "goal_l") return GameState::goal_l;
        if (str == "goal_r") return GameState::goal_r;

        // --- Infractions & Specific Restarts (l/r pairs) ---
        if (str == "offside_l") return GameState::offside_l;
        if (str == "offside_r") return GameState::offside_r;
        
        if (str == "foul_charge_l") return GameState::foul_charge_l;
        if (str == "foul_charge_r") return GameState::foul_charge_r;
        
        if (str == "back_pass_l") return GameState::back_pass_l;
        if (str == "back_pass_r") return GameState::back_pass_r;
        
        if (str == "free_kick_fault_l") return GameState::free_kick_fault_l;
        if (str == "free_kick_fault_r") return GameState::free_kick_fault_r;
        
        if (str == "indirect_free_kick_l") return GameState::indirect_free_kick_l;
        if (str == "indirect_free_kick_r") return GameState::indirect_free_kick_r;
        
        if (str == "illegal_defense_l") return GameState::illegal_defense_l;
        if (str == "illegal_defense_r") return GameState::illegal_defense_r;

        // --- Default / Error ---
        return GameState::unknown;
    }};
    GameState getState() {
        return game_state;
    };
    
    // basura
    std::optional<MinimalSocket::ReceiveStringResult> getServerMessage();
    
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