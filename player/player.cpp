#include <player.hpp>
#include <utils.hpp>
#include <server.hpp>
#include <iomanip>
#include "field.hpp"
#include "goalkeeper.hpp"
#include "defense.hpp"
#include "forward.hpp"

Player::Player(string team_name, int player_number, char side, bool is_goalie):
team_name{team_name}, player_number{player_number}, side{side}, is_goalie{is_goalie}{};
// Factory: instantiate subclass per role
Player& Player::getInstance(string team_name, MinimalSocket::Port player_port, char side, bool is_goalie) {
    if (instance) {
        std::cerr << "Player singleton already created" << std::endl;
        std::terminate();
    }
    int num = player_port; // player_port param actually passes player number from server
    if (is_goalie) {
        instance = new GoalkeeperPlayer(team_name, num, side, is_goalie);
    } else if (num >= 2 && num <= 4) {
        instance = new DefensePlayer(team_name, num, side, is_goalie);
    } else {
        instance = new ForwardPlayer(team_name, num, side, is_goalie);
    }
    return *instance;
}

void Player::parseSense_body(int time, string const& s){
    function<ScalarType(string)> conversion{[](string s)-> ScalarType{
        try {
            if (count(s.begin(), s.end(), '.') == 0 && 
                count(s.begin(), s.end(), 'E') == 0 && 
                count(s.begin(), s.end(), 'e') == 0)
                return stoi(s);
            else
                return stod(s);
        }catch(const invalid_argument &e) {
            return s;
        }
    }};
    function<VectorType(vector<string>)> conversion_verctor{[conversion](vector<string> s) -> VectorType {
        VectorType v;
        for (auto elem: s)
            v.push_back(conversion(elem));
        return v;
    }};

    auto exprs = split(s.substr(1, s.size()-2), ") (");

    bool is_nested {false};
    string nested_map_key;

    for (auto expr: exprs){
        if (expr.find("(", 0) != string::npos) {
            is_nested = true;
            auto parts = split(expr, " ("); // Only 2 parts should be there
            nested_map_key = parts.at(0);
            sense_body[nested_map_key] = NestedMap{};

            auto x = split(parts.at(1), ' ');
           if (x.size() == 2){
                if (x.at(1) == "none")
                    get<NestedMap>(sense_body[nested_map_key])[x.at(0)] = nullopt;
                else
                    get<NestedMap>(sense_body[nested_map_key])[x.at(0)] = stoi(x.at(1));
            } else 
                    get<NestedMap>(sense_body[nested_map_key])[x.at(0)] = make_pair(stoi(x.at(1)), stoi(x.at(2)));
        } else {
            if (is_nested){ 
                if (expr.find(")", 0) != string::npos) {
                    is_nested = false;
                    expr.pop_back();
                }
                auto x = split(expr, ' ');
                if (x.size() == 2){
                    if (x.at(1) == "none")
                        get<NestedMap>(sense_body[nested_map_key])[x.at(0)] = nullopt;
                    else
                        get<NestedMap>(sense_body[nested_map_key])[x.at(0)] = stoi(x.at(1));
                } else 
                    get<NestedMap>(sense_body[nested_map_key])[x.at(0)] = make_pair(stoi(x.at(1)), stoi(x.at(2))); 
            } else {
                auto tokens = split(expr, " ");
                if (tokens.empty()) continue;

                string key = tokens[0];
                tokens.erase(tokens.begin());
                if (tokens.size() == 1)
                    sense_body[key] = conversion(tokens[0]);
                else
                    sense_body[key] = conversion_verctor(tokens);
            }
        }    
    }
/*     std::cout << "\n=== Datos Parseados de Sense Body (Tiempo " << parse_time << ") ===" << std::endl;
    
    // --- Lambdas Auxiliares (Definidas en Scope Local) ---

    // 1. Lambda para imprimir la variante más interna (optional<int> o pair<int, int>)
    auto print_nested_value = [](std::ostream& os, const auto& nested_val) {
        std::visit([&os](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::optional<int>>) {
                if (arg) {
                    os << "Opt(" << *arg << ")";
                } else {
                    os << "none";
                }
            } else if constexpr (std::is_same_v<T, std::pair<int, int>>) {
                os << "Pair(" << arg.first << ", " << arg.second << ")";
            }
            // Añade más 'else if constexpr' si el variant anidado tiene más tipos
        }, nested_val);
    };

    // 2. Lambda para imprimir la variante principal (SenseBodyType)
    // Captura 'print_nested_value' por valor (o referencia si es necesario)
    auto visit_sense_body = [print_nested_value](std::ostream& os, const SenseBodyType& sbt) {
        
        std::visit([&os, &print_nested_value](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, ScalarType>) {
                // Caso 1: Contiene otra variante (ScalarType: int, double, string)
                // Se usa std::visit de nuevo (recursivo) para imprimir el valor
                std::visit([&os](const auto& scalar_val) {
                    os << "Scalar(" << scalar_val << ")";
                }, arg);
            } 
            else if constexpr (std::is_same_v<T, VectorType>) {
                // Caso 2: Contiene un Vector (VectorType)
                os << "Vector[";
                for (size_t i = 0; i < arg.size(); ++i) {
                    // Llama de nuevo a la impresión de ScalarType
                    std::visit([&os](const auto& scalar_val) { os << scalar_val; }, arg[i]);
                    if (i < arg.size() - 1) { os << ", "; }
                }
                os << "]";
            }
            else if constexpr (std::is_same_v<T, NestedMap>) {
                // Caso 3: Contiene el mapa anidado (NestedMap)
                os << "Map{";
                bool first = true;
                for (const auto& pair : arg) {
                    if (!first) os << ", ";
                    os << pair.first << ": ";
                    // Usa la lambda auxiliar para imprimir el valor del mapa
                    print_nested_value(os, pair.second); 
                    first = false;
                }
                os << "}";
            }
            
        }, sbt);
    };

    // --- Ejecución Principal ---

    for (const auto& entry : sense_body) {
        const std::string& key = entry.first;
        const SenseBodyType& value = entry.second;

        std::cout << "-> " << key << ": ";
        
        // Llamada a la lambda principal para imprimir el valor
        visit_sense_body(std::cout, value);
        
        std::cout << std::endl;
    }
    std::cout << "=====================================================" << std::endl; */
};
/* 
view_mode high normal
stamina 8000 1 130600
speed 0 0
head_angle 0
kick 0
dash 0
turn 1
say 0
turn_neck 0
catch 0
move 1
change_view 0
change_focus 0
arm (movable 0
    expires 0
    target 0 0
    count 0)

focus (target none
    count 0)

tackle (expires 0
    count 0)

collision none

foul (charged 0
    card none)

focus_point 0 0



(view_mode high normal) 
(stamina 8000 1 130600) 
(speed 0 0) 
(head_angle 0) 
(kick 0) (dash 0) 
(turn 1) 
(say 0) 
(turn_neck 0) 
(catch 0) 
(move 1) 
(change_view 0) 
(change_focus 0) 
(arm (movable 0) (expires 0) (target 0 0) (count 0)) 
(focus (target none) (count 0)) 
(tackle (expires 0) (count 0)) 
(collision none) 
(foul (charged 0) (card none)) 
(focus_point 0 0)    
    */

void Player::play(){
    Server& server = Server::getInstance();
    moveToInitial();
    // Default simple fallback if no subclass override
    turn(15);
};

void Player::x(string s) {
    Server& server = Server::getInstance();
    server.udp_socket.sendTo(s, server.server_udp);
    // Don't call getServer() here - let main loop handle message receipt
};

// Once per cycle, only one per cicle
void Player::turn(double dir, bool override){
    stringstream ss;
    ss << fixed << setprecision(3) << ((override) ? dir : dir * -1);
    x("(turn " + ss.str() + ")");
};

void Player::turnNeck(double dir, bool override){ // Can be exec in the same cycle as turn, dash, and kick
    stringstream ss;
    ss << fixed << setprecision(3) << ((override) ? dir : dir * -1);
    x("(turn_neck " + ss.str() + ")");
};

void Player::dash(double power, optional<double> dir, bool is_left, bool is_right, optional<double> powerR, optional<double> dirR, bool override){ // Only power is mandatory // maybe simplificaction
    // TODO: hay que implementar el sistema bipedo
    stringstream ss;
    ss << fixed << setprecision(3) << power;
    if (dir.has_value())
        ss << " " << ((override) ? dir.value() : dir.value() * -1);
    x("(dash " + ss.str() + ")");
};

void Player::kick(double power, double direction, bool override){
    stringstream ss;
    ss << fixed << setprecision(3) << power << " " << ((override) ? direction : direction * -1);
    string cmd = "(kick " + ss.str() + ")";
    cout << "KICK CMD: " << cmd << endl;
    x(cmd);
};

void Player::tackle(double powerOrAngle, bool foul, bool override){
    
};

void Player::move(double posX, double posY, bool override){ //can be executed only before kick off and after a goal
    stringstream ss;
    ss << fixed << setprecision(3) << ((override) ? posX : posX - 52.5)
                            << " " << ((override) ? posY : posY * -1);
    x("(move " + ss.str() + ")");
};

void Player::done(){
    x("(done)");
};

// Not once per cycle
void Player::changeView(int quality, optional<int> width){ // TODO: Hacerlo con enums
    
};

void Player::say(string s){
    
};

void Player::pointto(double dist, double dir){
    
};
void Player::notPointto(){
    
};
void Player::attentionto(bool our_team, int number){
    
};
void Player::notAttentionto(){
    
};

void Player::moveToInitial(){
    if (initial_positioned) return;
    Server& server = Server::getInstance();
    auto st = server.getState();
    // Per protocol: (move) is available only in before_kick_off and after a goal.
    if (st != Server::GameState::before_kick_off &&
        st != Server::GameState::kick_off_l &&
        st != Server::GameState::kick_off_r &&
        st != Server::GameState::goal_l &&
        st != Server::GameState::goal_r) {
        return;
    }

    // Formation is defined in server-centered coordinates for the left team.
    // Right-side team is mirrored to stay in its own half.
    double x = -20.0;
    double y = 0.0;

    switch(player_number){
        case 1: x = -50.0; y =  0.0; break; // goalkeeper-ish
        case 2: x = -40.0; y = -20.0; break;
        case 3: x = -40.0; y =  0.0; break;
        case 4: x = -40.0; y =  20.0; break;
        case 5: x = -30.0; y = -30.0; break;
        case 6: x = -30.0; y =  0.0; break;
        case 7: x = -30.0; y =  30.0; break;
        case 8: x = -20.0; y = -20.0; break;
        case 9: x = -20.0; y =  0.0; break;
        case 10: x = -20.0; y =  20.0; break;
        case 11: x = -50.0; y =  0.0; break;
        default: x = -20.0; y =  0.0; break;
    }

    // Put the kickoff taker close enough to kick immediately.
    // Keep within our half: slightly inside the center line.
    if (player_number == 9 &&
        (st == Server::GameState::before_kick_off ||
         (st == Server::GameState::kick_off_l && side == 'l') ||
         (st == Server::GameState::kick_off_r && side == 'r'))) {
        x = -0.5;
        y = 0.0;
    }

    if (side == 'r') {
        x = -x;
        y = -y;
    }

    // This project uses an internal x-offset and y-flip inside Player::move().
    move(x + 52.5, -y);
    initial_positioned = true;
}
