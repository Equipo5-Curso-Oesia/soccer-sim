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
    Player::getInstance(team_name, 
                        stoi(received_message_content.at(2)), 
                        received_message_content.at(1).at(0), 
                        is_goalie);
    // Assign role based on goalie flag; others default to forward/defense by simple rule
    {
        Player& p = Player::getInstance();
        if (is_goalie)
            p.setRole(std::unique_ptr<Role>(new Goalkeeper()));
        else {
            int num = stoi(received_message_content.at(2));
            // Formation: 1=striker/forward, 2-4=defense, 5-7=midfield, 8-11=forwards
            if (num >= 2 && num <= 4)
                p.setRole(std::unique_ptr<Role>(new Defense()));
            else
                p.setRole(std::unique_ptr<Role>(new Forward()));
        }
    }
    


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
    if (!debug) 
        response = getServerMessage()->received_message;   
    else {    
        auto before = chrono::high_resolution_clock::now().time_since_epoch().count();
        response = getServerMessage()->received_message;        
        auto now = (double)chrono::high_resolution_clock::now().time_since_epoch().count();
        cout << "Message received: " << now - before << endl << response << endl;
    }
    if (response.substr(0, 12) == "(sense_body ") {
        string token;
        istringstream timeStream(response);
        getline(timeStream, token, ' ');
        getline(timeStream, token, ' '); 
        time = stoi(token);

        istringstream responseStream(response);
        getline(responseStream, token, '(');
        getline(responseStream, token, '('); 
        token = "(" + token; // Equals to: "(sense_body Time "
        player.parseSense_body(time, response.substr(token.size(), response.size()-(token.size()+1)));
        field.calculatePositions(time);
    } else if (response.substr(0, 5) == "(see ") {
        string token;
        istringstream timeStream(response);
        getline(timeStream, token, ' ');
        getline(timeStream, token, ' '); 
        time = stoi(token);

        istringstream responseStream(response);
        getline(responseStream, token, '(');
        getline(responseStream, token, '('); 
        token = "(" + token; // Equals to: "(see Time "
        field.parseSee(time , response.substr(token.size(), response.size()-(token.size()+1)));
        getServer(debug);
    
    } else if (response.substr(0, 6) == "(hear ") {
        cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
        cout << "Message received: " << endl << response << endl;
        cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
        string token;
        istringstream timeStream(response);
        getline(timeStream, token, ' ');
        getline(timeStream, token, ' '); 
        time = stoi(token);

        token = "(hear " + token + " referee "; // Equals to: "(hear Time referee "
        auto state = hashString(response.substr(token.size(), response.size()-(token.size()+1)));
        if (state != GameState::unknown)
            game_state = state;
        else {
            // rest of hear args that are not states from referee
        }
    } else { // The rest
        // Game status and referee msg in 4.7.1.
        // (hear 0 referee kick_off_l)
        // (hear 39 referee yellow_card_l_1)
        // (hear 100 referee play_on)
        // (hear 100 referee drop_ball)
        // (score Time Our Opp)
        //(error unknown command) or (error illegal command form)
        /* 
        (hear Time Sender “Message”)
        (hear Time OnlineCoach CoachLanguageMessage)
            Time ::= simulation cycle of rcssserver
            Sender ::= online_coach_left | online_coach_right | coach | referee | self | Direction
            Direction ::= -180 ~ 180 degrees
            Message ::= string
            OnlineCoach ::= online_coach_left | online_coach_right
            CoachLanguageMessage ::= see the standard coach language section 
        */
        /*
        (fullstate Time
            (pmode {goalie_catch_ball_{l|r}|*PlayMode*})
            (vmode {high|low} {narrow|normal|wide})
            (count KickCount DashCount TurnCount CatchCount MoveCount TurnNeckCount ChangeViewCount SayCount)
            (arm (movable MovableCycles) (expires ExpireCycles)) (target Distance Direction) (count PointtoCount)
            (score Time Our Opp)
            ((b) X Y VelX VelY)
            Players+)
                Players ::= ((p {l|r} UniformNumber [g] PlayerType) X Y VelX VelY BodyDir NeckDir [PointtoDist PointtoDir] (stamina Stamina Effort Recovery Capacity) [k|t|f] [r|y]))        
        */
        if (response != "(warning message_not_null_terminated)"){
            cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
            cout << "Message received: " << endl << response << endl;
            cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
        }
        getServer(debug);
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
    auto received_message = udp_socket.receive(message_max_size);
    received_message->received_message.pop_back(); // remove last char eof
    return received_message;
}






