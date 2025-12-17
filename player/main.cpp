#include <server.hpp>
#include <field.hpp>
#include <utils.hpp>
#include <player.hpp>
#include <player_test.hpp>
#include <goalkeeper.hpp>
#include <defender.hpp>

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
    Field& field = Field::getInstance();
    Player* player {nullptr};
    switch(Player::getInstance<Player>().getPlayerNumber()) {
        case 1: //player = &Player::getInstance<PlayerTest>(); break;
        case 2:
        case 3:
        case 4:
        case 5: //player = &Player::getInstance<Forward>(); break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10: player = &Player::getInstance<Defender>(); break;
        default:
            if (is_goalie) player = &Player::getInstance<Goalkeeper>(); break;
    }
    

    // Ver como cambiar de tipo de jugador

    cout << "Player created successfully" << endl;
    
    while(true){

        server.getServer();///* true); */
        player->play(); // Los argumentos los hace creando objetos de field y server y haciendo getter y setters correspondientes

        // Aquí va la logica del jugador
        // La logica devuelve la accion/acciones a enviar al servidor

    }    
}
