#include <forward.hpp>
#include <server.hpp>
#include <field.hpp>
#include <iostream>
#include <functional>
#include <queue>

using Task = std::function<void()>;

void Forward::play(){

    cout << "Forward playing" << endl;

    Server& s = Server::getInstance();
    Field& f = Field::getInstance();

    static int i = 0;
    if (s.getState() == Server::GameState::before_kick_off){ // Antes del saque
        cout << i << endl;
        turn(20);
        i++;
    } else if ((s.getState() == Server::GameState::kick_off_l && side == 'l')
                    ||
                s.getState() == Server::GameState::kick_off_r && side == 'r') { // Saco mi equipo

    } else if ((s.getState() == Server::GameState::kick_off_l && side == 'r')
                    ||
                s.getState() == Server::GameState::kick_off_r && side == 'l') { // Saca contrario


    } else if (s.getState() == Server::GameState::drop_ball) { // EL balon está en juego si en 100 ciclos el equipo que le toca sacar no la ha tocado o cualquiera la ha tocado posteriormente

    } else if (s.getState() == Server::GameState::play_on) { // El partido está en marcha

        // Si ves la pelota a menos de x distancia
            // Si la pelota está muy cerca
                // Patear --> kick(50, dirPorteriaRival)
            // Si la pelota no está muy cerca
                // Mirar hacia ella --> turn(dirBall)
                // Correr hacia ella --> dash(50)
        // Si no ves la pelota
            // Si no estás en la posición inicial
                // Recuperar posición
                    // turn(dirPosInicial)
                    // dash(50)
            // Una vez recuperada la posición
                // Girar sobre si mismo --> turn(20)

    }

    static std::queue<Task> task_queue;

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

};