#include <server.hpp>
#include <field.hpp>
#include <utils.hpp>
#include <player.hpp>

using namespace std;
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

int main(int argc, char *argv[])
{
    string team_name;
    MinimalSocket::Port send_port;
    bool is_goalie;
    try {
        tie(team_name, send_port, is_goalie) = parseArgs(argc-1, &argv[1]);
    } catch (const invalid_argument &e) {
        cerr << e.what() << endl;
        return -1;
    }
    
    cout << "Arguments parsed successfully" << endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

    Server& server = Server::getInstance(team_name, send_port, is_goalie);
    cout << "Player created successfully" << endl;

    Field& field = Field::getInstance();

    Player& player = Player::getInstance();


    //sleep(1);

    server.x("(move -34 0)");
    server.x("(turn 50)"); // el angulo es negativo respecto a nosotros
    //server.x("(turn 180)");
    server.x("(turn_neck -25)");


    //auto before = chrono::high_resolution_clock::now().time_since_epoch().count();
    //cout << before << endl;
    while(true){

        server.getServer();///* true); */
        player.play(); // Los argumentos los hace creando objetos de field y server y haciendo getter y setters correspondientes

        // Aquí va la logica del jugador
        // La logica devuelve la accion/acciones a enviar al servidor

    }

/*     Players player;
    player.parseInit(init_message_content);
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
    init_message_content = received_message->received_message;
    
    cout << "Listo para jugar" << endl; */

/*     while(true){
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
    } */
    
}
