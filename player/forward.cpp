#include <forward.hpp>
#include <server.hpp>
#include <field.hpp>
#include <iostream>
#include <functional>
#include <queue>
#include <list>

using Task = std::function<void()>;

void Forward::play(){

    cout << endl << endl;

    Server& s = Server::getInstance();
    Field& f = Field::getInstance();

    static int i = 0;
    static Server::GameState last_game_state = Server::GameState::unknown;
    if (last_game_state != s.getState())
        i = 0;
    last_game_state = s.getState();
    if (s.getState() == Server::GameState::before_kick_off){ // Antes del saque

        switch(i) {
            case 0:
                move(posForNumber(player_number).first, posForNumber(player_number).second); 
                break;
            default:
                findBall(i, get<1>(f.getBall()));
                break;                 
        }

    } else if ((s.getState() == Server::GameState::kick_off_l && side == 'l')
                    ||
                s.getState() == Server::GameState::kick_off_r && side == 'r') { // Saco mi equipo
        PosData ball = f.getBall();

        if (!get<1>(ball).has_value())
            findBall(i, get<1>(f.getBall()));

        else if(get<0>(ball).has_value()){
            if(get<0>(ball).value() < 0.7) {

                if (abs(get<1>(ball).value()) > 10.0) {
                    turn(get<1>(ball).value());
                    return;
                }
                kick(80, get<1>(ball).value()* -1);
                return;
            }
            if (get<0>(ball).value() < 20.0) {
                dash(70, get<1>(ball).value());
                return;
            }
            // Track ball by turning toward it
            turn(get<1>(ball).value());
            return;
        }
    } else if ((s.getState() == Server::GameState::kick_off_l && side == 'r')
                    ||
                s.getState() == Server::GameState::kick_off_r && side == 'l') { // Saca contrario
        
        findBall(i, get<1>(f.getBall()));
    } else if (s.getState() == Server::GameState::drop_ball) { // EL balon está en juego si en 100 ciclos el equipo que le toca sacar no la ha tocado o cualquiera la ha tocado posteriormente

    } else if (s.getState() == Server::GameState::play_on) { // El partido está en marcha

        PosData ball = f.getBall();

        if (!get<1>(ball).has_value())
            findBall(i, get<1>(f.getBall()));

        else if(get<0>(ball).has_value()){
            if(get<0>(ball).value() < 0.7) {

                if (abs(get<1>(ball).value()) > 10.0) {
                    turn(get<1>(ball).value());
                    return;
                }
                kick(80, get<1>(ball).value()* -1);
                return;
            }
            if (get<0>(ball).value() < 20.0) {
                dash(70, get<1>(ball).value());
                return;
            }
            // Track ball by turning toward it
            turn(get<1>(ball).value());
            return;
        }
    }


    i++;

/*  static std::queue<Task> task_queue;

    if (task_queue.empty()) {
        task_queue.push(std::bind(&Player::turnNeck, this, 10, false));
        task_queue.push(std::bind(&Player::turn, this, 10, false));
        task_queue.push(
            [this, &f]() {
                cout << "CICLO 3: Ejecutando Dash con poder " << i << endl;
                this->dash(10); 
            }
        );
    }

    Task next_task = task_queue.front();
    next_task(); 
    task_queue.pop();
*/
}

void Forward::findBall(int i, optional<double> ballDir)
{
    if (i % 3 == 0)
    {
        if (ballDir.has_value())
        {
            turn(ballDir.value());
        }
        else
        {
            turn(20);
        }
    }
};
