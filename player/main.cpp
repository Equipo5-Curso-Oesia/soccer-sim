#include <iostream>

using namespace std;

#include <MinimalSocket/udp/UdpSocket.h>

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
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <team-name> <this-port>" << endl;
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

    udp_socket.sendTo("(init " + team_name + " (version 19))", other_recipient_udp);
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
        sleep(1);
    }
    
}
