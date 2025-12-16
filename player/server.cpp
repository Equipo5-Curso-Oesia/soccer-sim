//TODO: see is every 3 cycles but may be changed, static each 3 cycles will be implemented

//TODO: provide estimation for cycles in between see info

//TODO: make predictions about feacility of reaching the ball

//TODO: coordinate pases
#include <server.hpp>
#include <utils.hpp>
#include <field.hpp>
#include <player.hpp>
#include <roles.hpp>

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
    // Create player instance (subclass chosen in factory)
    Player::getInstance(team_name, 
                        stoi(received_message_content.at(2)), 
                        received_message_content.at(1).at(0), 
                        is_goalie);
    


    server_port = received_message->sender.getPort();
    server_udp = MinimalSocket::Address{"127.0.0.1", server_port};

    // Request synchronous see mode if supported. If the server ignores or
    // rejects it, we keep running in async mode.
    udp_socket.sendTo("(synch_see)", server_udp);

    //udp_socket.sendTo("(compression 11)", server_udp);
    //udp_socket.sendTo("(gaussian_see)", server_udp);

    bool got_server_param = false;
    bool got_player_param = false;
    int got_player_types = 0;

    for (int i = 0; i < 200; ++i) {
        auto msg = this->getServerMessage();
        if (!msg.has_value()) break;
        const string& response = msg->received_message;

        if (response.find("(server_param") == 0) {
            int n = response.find('(', 1);
            server_params = parseServerMessage(response.substr(n, response.size() - n - 1));
            got_server_param = true;
            continue;
        }
        if (response.find("(player_param") == 0) {
            int n = response.find('(', 1);
            player_params = parseServerMessage(response.substr(n, response.size() - n - 1));
            got_player_param = true;
            continue;
        }
        if (response.find("(player_type") == 0) {
            int n = response.find('(', 1);
            player_types.push_back(parseServerMessage(response.substr(n, response.size() - n - 1)));
            ++got_player_types;
            continue;
        }
        if (response == "(ok synch_see)") {
            synch_see_enabled = true;
            if (got_server_param && got_player_param && got_player_types > 0) break;
            continue;
        }
        if (response.find("(error") == 0) {
            // If synch_see is not supported, proceed without it.
            if (got_server_param && got_player_param && got_player_types > 0) {
                pending_message = std::move(*msg);
                break;
            }
            continue;
        }

        // The server may start sending game messages (sense_body/see/hear/...) at any time.
        // Stash the first one so the main loop can process it.
        pending_message = std::move(*msg);

        // If we already have the params, we can start.
        if (got_server_param && got_player_param && got_player_types > 0) break;

        // Otherwise keep trying to collect params, but don't hard-fail.
    }
}

void Server::sendDone(){
    if (!synch_see_enabled) return;
    udp_socket.sendTo("(done)", server_udp);
}

void Server::getServer(bool debug) {
    Field& field = Field::getInstance();
    Player& player = Player::getInstance();

    // Consume messages until we have processed a sense_body for this cycle.
    // This avoids unbounded recursion (stack overflow) and keeps the main loop stable.
    while (true) {
        string response;
        if (!debug) {
            response = getServerMessage()->received_message;
        } else {
            auto before = chrono::high_resolution_clock::now().time_since_epoch().count();
            response = getServerMessage()->received_message;
            auto now = (double)chrono::high_resolution_clock::now().time_since_epoch().count();
            cout << "Message received: " << now - before << endl << response << endl;
        }

        if (response.size() >= 12 && response.substr(0, 12) == "(sense_body ") {
            string token;
            istringstream timeStream(response);
            getline(timeStream, token, ' ');
            getline(timeStream, token, ' ');
            time = stoi(token);

            istringstream responseStream(response);
            getline(responseStream, token, '(');
            getline(responseStream, token, '(');
            token = "(" + token; // "(sense_body Time "
            player.parseSense_body(time, response.substr(token.size(), response.size() - (token.size() + 1)));
            field.calculatePositions(time);
            return;
        }

        if (response.size() >= 5 && response.substr(0, 5) == "(see ") {
            string token;
            istringstream timeStream(response);
            getline(timeStream, token, ' ');
            getline(timeStream, token, ' ');
            time = stoi(token);

            istringstream responseStream(response);
            getline(responseStream, token, '(');
            getline(responseStream, token, '(');
            token = "(" + token; // "(see Time "
            field.parseSee(time, response.substr(token.size(), response.size() - (token.size() + 1)));
            continue;
        }

        if (response.size() >= 6 && response.substr(0, 6) == "(hear ") {
            string token;
            istringstream timeStream(response);
            getline(timeStream, token, ' ');
            getline(timeStream, token, ' ');
            time = stoi(token);

            token = "(hear " + token + " referee ";
            auto state = hashString(response.substr(token.size(), response.size() - (token.size() + 1)));
            if (state != GameState::unknown) game_state = state;
            continue;
        }

        // The rest: score/error/warning/fullstate/etc.
        if (response != "(warning message_not_null_terminated)") {
            cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
            cout << "Message received: " << endl << response << endl;
            cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
        }
    }
}

map<string, variant<int, double, string>> Server::parseServerMessage(const string& message){
    // This asume is (key value)()()..() no nested structures no multiple values
    map<string, variant<int, double, string>> parse;
    size_t pos =0;
    while(pos != string::npos && pos < message.size()){
        auto openParPos = message.find('(', pos);
        auto closeParPos = message.find(')', pos);

        vector<string> v = split(message.substr(openParPos+1, closeParPos-openParPos-1), ' ');

        try {
            if (count(v.at(1).begin(), v.at(1).end(), '.') == 0 && 
                count(v.at(1).begin(), v.at(1).end(), 'E') == 0 && 
                count(v.at(1).begin(), v.at(1).end(), 'e') == 0)
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

std::optional<MinimalSocket::ReceiveStringResult> Server::getServerMessage(){
    if (pending_message.has_value()) {
        auto tmp = std::move(pending_message);
        pending_message.reset();
        return tmp;
    }
    auto received_message = udp_socket.receive(message_max_size);
    received_message->received_message.pop_back(); // remove last char eof
    return received_message;
}






