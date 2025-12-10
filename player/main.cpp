#include <iostream>

using namespace std;

#include <MinimalSocket/udp/UdpSocket.h>
#include <unistd.h>

/*
/////////////////////////////////////////////////////////////////////////////////

Importante:
Ejemplode como trabajar con git.

1º Nunca se trabaja sobre la rama main (master).
2º Se crea una rama nueva a partir de main (git branch <nombre de la rama nueva>).
3º Se cambia a la nueva rama (git checkout <nombre de la rama nueva>).
    (git checkout -b <nombre de la rama nueva>). Crea la rama y se cambia a ella automatiucamente.
3º En ella se desarrolla la funcionalidad nueva, una unica.
4º Cuando se termina, se hace un commit con un mensaje claro (git commit -a -m "mensaje claro").
5º Se sube la rama al repositorio remoto (git push --set-upstream origin <nombre de la rama nueva>). Es recomendable que periodicamente se haga push mientras se trabaja para evitar perdida de trabajo y permitir que el resto pueda ver que haces.
6º Si la rama ya está en el repositorio remoto, se hace (git push origin <nombre de la rama nueva>) o (git push).
7º Se abre un Pull Request en github para hacer el merge con la rama principal. Esto se hace con todo el equipo, nunca de forma individual. Se hace desde la web de GitHub
7º bis Las ramas se pueden dejar colgando en el repositorio remoto si contienen por ejemplo pruebas de concepto que no se van a integrar en la rama principal como tal.

/////////////////////////////////////////////////////////////////////////////////
*/
// main with three args

enum class RolJugador {
    Portero,
    Defensa,
    Medio,
    Delantero
};

struct Player
{
    int numeroJugador;
    char lado;
    RolJugador rol;

    void parseInit(string msg)
    {   
        // ejemplo msg: (init l 2 before_kick_off)
        size_t pos = msg.find(' ');
        lado = msg[pos + 1];
        size_t pos_inicio_numero = msg.find(' ', pos + 1);
        size_t pos_final_numero = msg.find(' ', pos_inicio_numero + 1);
        numeroJugador = stoi(msg.substr(pos_inicio_numero + 1, pos_final_numero - pos_inicio_numero - 1) );
        asignarRol();
    }

    void asignarRol() {
        // Asignación simple por número de jugador
        if (numeroJugador == 11) {
            rol = RolJugador::Portero;
        } else if (numeroJugador >= 1 && numeroJugador <= 4) {
            rol = RolJugador::Defensa;
        } else if (numeroJugador >= 5 && numeroJugador <= 7) {
            rol = RolJugador::Medio;
        } else {
            rol = RolJugador::Delantero;
        }
    }

    friend ostream &operator<<(ostream &os, const Player &p)
    {
        os << "Soy el " << p.numeroJugador << " y mi lado es " << p.lado << ". Mi rol es: ";
        switch (p.rol) {
            case RolJugador::Portero: os << "Portero"; break;
            case RolJugador::Defensa: os << "Defensa"; break;
            case RolJugador::Medio: os << "Medio"; break;
            case RolJugador::Delantero: os << "Delantero"; break;
        }
        return os;
    }
};

// Busca la posicion del balon en el mensaje de vision
// Retorna: true si encuentra el balon, false si no
// angulo_balon: angulo relativo hacia el balon
// distancia_balon: distancia al balon
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

// Corre hacia la posicion
// Gira hacia el; si esta mirando en la direccion, corre
void correrHaciaPosicion(MinimalSocket::udp::Udp<true> &udp_socket, const MinimalSocket::Address &server_udp, double angulo)
{
    if (abs(angulo) > 10) {
        udp_socket.sendTo("(turn " + to_string(angulo) + ")", server_udp);
    } else {
        udp_socket.sendTo("(dash 100)", server_udp);
    }
}

// Patear el balon hacia la porteria del rival
// Si el lado es 'l' (left), patear hacia la derecha
// Si el lado es 'r' (right), patear hacia la izquierda
void patearHaciaPorteria(MinimalSocket::udp::Udp<true> &udp_socket, const MinimalSocket::Address &server_udp, char lado)
{
    double kick_direction = (lado == 'l') ? 0 : 180;  
    udp_socket.sendTo("(kick 100 " + to_string(kick_direction) + ")", server_udp);
}

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
        {-40, -20}, // Player 1
        {-40, 0},   // Player 2
        {-40, 20},  // Player 3
        {-30, -30}, // Player 4
        {-30, 0},   // Player 5
        {-30, 30},  // Player 6
        {-20, -20}, // Player 7
        {-20, 0},   // Player 8
        {-20, 20},  // Player 9
        {-10, 0},   // Player 10
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
        if (buscarBalon(received_message_content, angulo_balon, distancia_balon)) {
            if (distancia_balon < 0.5) {
                patearHaciaPorteria(udp_socket, server_udp, player.lado);
            } else {
                correrHaciaPosicion(udp_socket, server_udp, angulo_balon);
            }
        }
    }
    
}
