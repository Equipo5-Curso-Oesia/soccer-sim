#include <server.hpp>
#include <utils.hpp>
#include <field.hpp>
#include <player.hpp>

Server::Server(string team_name, MinimalSocket::Port player_port, bool is_goalie) noexcept: 
player_port{player_port},
server_udp{"127.0.0.1", server_port}, udp_socket{player_port, MinimalSocket::AddressFamily::IP_V6}
{
    string init_message_content = "(init " + team_name + " (version 19)";
    if(is_goalie)
        init_message_content += " (goalie)";
    init_message_content += ")";

    udp_socket = MinimalSocket::udp::Udp<true>(player_port, MinimalSocket::AddressFamily::IP_V6);

    if (!udp_socket.open())
        throw runtime_error("Error opening socket");

    server_udp = MinimalSocket::Address{"127.0.0.1", server_port};
    udp_socket.sendTo(init_message_content, server_udp);

    auto received_message = getServerMessage();
    auto received_message_content = split(received_message->received_message, ' ');

    received_message_content.at(3).pop_back(); // remove last char ")"
    game_state = hashString(received_message_content.at(3));
    Player::getInstance(team_name, 
                        stoi(received_message_content.at(2)), 
                        received_message_content.at(1).at(0), 
                        is_goalie);

    


    server_port = received_message->sender.getPort();
    server_udp = MinimalSocket::Address{"127.0.0.1", server_port};

    //udp_socket.sendTo("(compression 11)", server_udp);
    //udp_socket.sendTo("(gaussian_see)", server_udp);

    string response;
    do{
        response = this->getServerMessage()->received_message;

        if(response.find("(server_param") == 0){
            int n = response.find('(', 1);
            server_params = parseServerMessage(response.substr(n, response.size() - n-1)); // remove (server_param ... )
        } else if(response.find("(player_param") == 0){
            int n = response.find('(', 1);
            player_params = parseServerMessage(response.substr(n, response.size() - n-1));
        } else if(response.find("(player_type") == 0){
            int n = response.find('(', 1);
            player_types.push_back(parseServerMessage(response.substr(n, response.size() - n-1)));
        } else if (response != "(ok synch_see)")
            throw runtime_error("Unexpected message received during initialization: \n" + response.substr(0, 20));

    } while (response != "(ok synch_see)");  
}

void Server::getServer(bool debug) {
    //Receive from the server
    // Parse message
    // If next message is see
        // receive see
        // parse see 
    string response;
    Field& field = Field::getInstance();
    Player& player = Player::getInstance();
    if (debug) 
     response = getServerMessage()->received_message;   
    else {    
        auto before = chrono::high_resolution_clock::now().time_since_epoch().count();
        response = getServerMessage()->received_message;        
        auto now = (double)chrono::high_resolution_clock::now().time_since_epoch().count()/1000000;
        cout << "Message received: " << now - before << endl << response << endl;
    }
    if (response.substr(0, 12) == "(sense_body ") {
        string token;
        istringstream responseStream(response);
        getline(responseStream, token, ' ');
        getline(responseStream, token, ' '); 
        time = stoi(token);

        istringstream responseStream(response);
        getline(responseStream, token, '(');
        getline(responseStream, token, '('); 
        token = "(" + token; // Equals to: "(sense_body Time "
        //TODO: implement player
        player.parseSense_body(time, response.substr(token.size(), response.size()-(token.size()+1)));
        field.calculatePositions();
    } else if (response.substr(0, 5) == "(see ") {
        string token;
        istringstream responseStream(response);
        getline(responseStream, token, ' ');
        getline(responseStream, token, ' '); 
        time = stoi(token);

        istringstream responseStream(response);
        getline(responseStream, token, '(');
        getline(responseStream, token, '('); 
        token = "(" + token; // Equals to: "(see Time "
        field.parseSee(time , response.substr(token.size(), response.size()-(token.size()+1)));
        getServer();
    } else {
        cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
        cout << "Message received: " << endl << response << endl;
        cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
        getServer();
    }
}

Server::RecursiveTypeMap Server::parseServerMessage(const string& message){
    // This asume is (key value)()()..() no nested structures

    Server::RecursiveTypeMap parse;
    size_t pos =0;
    while(pos != string::npos && pos < message.size()){
        auto openParPos = message.find('(', pos);
        auto closeParPos = message.find(')', pos);

        vector<string> v = split(message.substr(openParPos+1, closeParPos-openParPos-1), ' ');

        try {
            if (count(v.at(1).begin(), v.at(1).end(), '.') == 0 && 
                count(v.at(1).begin(), v.at(1).end(), 'E') == 0 && 
                count(v.at(1).begin(), v.at(1).end(), 'E') == 0)
                parse[v.at(0)] = stoi(v.at(1));
            else
                parse[v.at(0)] = stod(v.at(1));
        }catch(const invalid_argument &e) {
            parse[v.at(0)] = v.at(1);
        }
                
        pos = ++closeParPos;
    }
    return parse;
}

//TODO: see is every 3 cycles but may be changed, static each 3 cycles will be implemented

//TODO: provide estimation for cycles in between see info

//TODO: make predictions about feacility of reaching the ball

//TODO: coordinate pases

std::optional<MinimalSocket::ReceiveStringResult> Server::getServerMessage(){
    auto received_message = udp_socket.receive(message_max_size);
    received_message->received_message.pop_back(); // remove last char eof
    return received_message;
}

void Server::x(string s) {
    cout << "before: ";
    getServer();
    udp_socket.sendTo(s, server_udp);
    cout << "after: ";
    getServer();
    
};

/* 
struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2() = default;
    Vec2(double _x, double _y) : x(_x), y(_y) {}

    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }

    double length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const {
        double L = length();
        return (L > 1e-9) ? Vec2(x / L, y / L) : Vec2(0, 0);
    }
}; */

/* class Server {
public:
    Server(int id, const std::string& name, const Vec2& startPos = Vec2{})
        : id_(id), name_(name), pos_(startPos) {}

    // Basic tick update, dt in seconds
    void update(double dt) {
        if (moving_) {
            Vec2 toTarget = target_ - pos_;
            double dist = toTarget.length();
            if (dist < 1e-6) {
                vel_ = Vec2{};
                moving_ = false;
            } else {
                Vec2 dir = toTarget.normalized();
                // simple speed interpolation
                double desiredSpeed = maxSpeed_;
                Vec2 desiredVel = dir * desiredSpeed;
                // simple smoothing
                vel_ = vel_ * 0.8 + desiredVel * 0.2;
                pos_ = pos_ + vel_ * dt;
                // stop when close
                if ((target_ - pos_).length() < 0.05) {
                    pos_ = target_;
                    vel_ = Vec2{};
                    moving_ = false;
                }
            }
            // stamina drains while moving
            stamina_ = std::max(0.0, stamina_ - staminaDrainPerSec_ * dt);
        }
    }

    void moveTo(const Vec2& target) {
        target_ = target;
        moving_ = true;
    }

    // Kick the ball with a given power in range [0,1]
    void kick(double power) {
        power = std::min(1.0, std::max(0.0, power));
        double actualPower = power * kickPower_ * (0.5 + 0.5 * (stamina_)); // stamina affects kick
        // In a real sim you'd apply this to the ball; here we just print it
        std::cout << "Server " << id_ << " (" << name_ << ") kicks with power " << actualPower << "\n";
        stamina_ = std::max(0.0, stamina_ - 0.1 * power);
    }

    // Accessors
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    Vec2 position() const { return pos_; }
    double stamina() const { return stamina_; }

    // Simple status print
    void debugPrint() const {
        std::cout << "Server " << id_ << " '" << name_ << "' pos(" << pos_.x << "," << pos_.y
                  << ") vel(" << vel_.x << "," << vel_.y << ") stamina=" << stamina_
                  << (moving_ ? " moving\n" : " idle\n");
    }

private:
    int id_;
    std::string name_;
    Vec2 pos_{}, vel_{}, target_{};
    bool moving_ = false;

    // Tunable parameters
    double maxSpeed_ = 3.0;             // m/s
    double kickPower_ = 20.0;           // arbitrary units
    double stamina_ = 1.0;              // 0..1
    double staminaDrainPerSec_ = 0.05;  // per second while moving
}; */

// Example usage (comment out or remove in production):
/*
int main() {
    Server p(7, "Striker", Vec2{0,0});
    p.moveTo(Vec2{10, 0});
    for (int i = 0; i < 100; ++i) {
        p.update(0.1);
        p.debugPrint();
    }
    p.kick(0.8);
    return 0;
}
*/