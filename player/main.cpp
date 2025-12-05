#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

#include <MinimalSocket/udp/UdpSocket.h>
#include <unistd.h>

// main with three args

struct Player
{
    int numeroJugador;
    char lado;

    void parseInit(string msg)
    {   
        // ejemplo msg: (init l 2 before_kick_off)
        size_t pos = msg.find(' ');
        lado = msg[pos + 1];
        size_t pos_inicio_numero = msg.find(' ', pos + 1);
        size_t pos_final_numero = msg.find(' ', pos_inicio_numero + 1);
        numeroJugador = stoi(msg.substr(pos_inicio_numero + 1, pos_final_numero - pos_inicio_numero - 1) );
    }
	
	bool buscarBalon(const string &message, double &angulo_balon, double &distancia_balon)
	{
		//Ejemplo de mensaje: "(see 0 ((b) 10.0 30.0) ... )"
		size_t ball_pos = message.find("(b)");
		if (ball_pos == string::npos) {
			return false;
		}
		
		size_t start = ball_pos + 3;
		size_t end = message.find(')', start);
		string ball_info = message.substr(start, end - start);
		size_t space_pos = ball_info.find(' ', ball_info.find(' ') + 1);
		if (space_pos == string::npos) {
			return false;
		}
		
		distancia_balon = stod(ball_info.substr(0, space_pos));
		angulo_balon = stod(ball_info.substr(space_pos + 1));
		return true;
	}
	
	void correrHaciaPosicion(MinimalSocket::udp::Udp<true> &udp_socket, const MinimalSocket::Address &server_udp, double angulo)
	{
		if (abs(angulo) > 10) {
			udp_socket.sendTo("(turn " + to_string(angulo) + ")", server_udp);
		} else {
			udp_socket.sendTo("(dash 100)", server_udp);
		}
	}
	
	void patearHaciaPorteria(MinimalSocket::udp::Udp<true> &udp_socket, const MinimalSocket::Address &server_udp, char lado)
	{
		double kick_direction = (lado == 'l') ? 0 : 180;  
		udp_socket.sendTo("(kick 100 " + to_string(kick_direction) + ")", server_udp);
	}

    friend ostream &operator<<(ostream &os, const Player &p)
    {
        os << "Soy el " << p.numeroJugador << " y mi lado es " << p.lado;
        return os;
    }
};


// main with two args
int main(int argc, char *argv[])
{
    // check if the number of arguments is correct
    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << " <team-name> <this-port> <is-goalie (false/true)>" << endl;
        return 1;
    }

    // get the team name and the port
    string team_name = argv[1];
    MinimalSocket::Port this_socket_port = std::stoi(argv[2]);

    cout << "Creating a UDP socket" << endl;

    MinimalSocket::udp::Udp<true> udp_socket(this_socket_port, MinimalSocket::AddressFamily::IP_V6);

    cout << "Socket created" << endl;

    bool success = udp_socket.open();

    if (!success)
    {
        cout << "Error opening socket" << endl;
        return 1;
    }

    MinimalSocket::Address other_recipient_udp = MinimalSocket::Address{"127.0.0.1", 6000};
    cout << "(init " + team_name + " (version 19))" << endl;

    bool is_goalie = (string(argv[3]) == "true");
    if(!is_goalie){
        udp_socket.sendTo("(init " + team_name + " (version 19))", other_recipient_udp);
    }
    else{
        udp_socket.sendTo("(init " + team_name + " (version 19) (goalie))", other_recipient_udp);
    }
    cout << "Init Message sent" << endl;

    std::size_t message_max_size = 1000;
    cout << "Waiting for a message" << endl;
    auto received_message = udp_socket.receive(message_max_size);
    std::string received_message_content = received_message->received_message;

    //cout << "Message received: " << received_message_content << endl;

    Player player;
    player.parseInit(received_message_content);
    cout << player << endl;

    MinimalSocket::Address other_sender_udp = received_message->sender;
    MinimalSocket::Address server_udp = MinimalSocket::Address{"127.0.0.1", other_sender_udp.getPort()};

    int posicionInicial[11][2] = {
        {-10, 0},   // Player 1
        {-40, -20}, // Player 2
        {-40, 0},   // Player 3
        {-40, 20},  // Player 4
        {-30, -30}, // Player 5
        {-30, 0},   // Player 6
        {-30, 30},  // Player 7
        {-20, -20}, // Player 8
        {-20, 0},   // Player 9
        {-20, 20},  // Player 10
        {-50, 0}    // Player 11
    };
    
    int x = posicionInicial[player.numeroJugador - 1][0];
    int y = posicionInicial[player.numeroJugador - 1][1];
    
    string move_command = "(move " + to_string(x) + " " + to_string(y) + ")";
    udp_socket.sendTo(move_command, server_udp);
    
    received_message = udp_socket.receive(message_max_size);
    received_message_content = received_message->received_message;
    
    cout << "Listo para jugar" << endl;

    while(true){
        received_message = udp_socket.receive(message_max_size);
        received_message_content = received_message->received_message;    
        
        double angulo_balon, distancia_balon;
        if (player.buscarBalon(received_message_content, angulo_balon, distancia_balon))
		{
            if (distancia_balon < 0.5)
			{
                player.patearHaciaPorteria(udp_socket, server_udp, player.lado);
            } 
			else if(!is_goalie)
			{
				player.correrHaciaPosicion(udp_socket, server_udp, angulo_balon);
            }
        }
    }

    
}
