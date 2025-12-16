#include <player_test.hpp>
#include <server.hpp>
#include <field.hpp>
#include <iostream>
#include <functional>
#include <queue>

using Task = function<void()>;

void PlayerTest::play(){

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



    }

    #include <queue>
    #include <iostream>
    #include <functional>

    using Task = function<void()>;

    static queue<Task> task_queue;
    
    task_queue.push(bind(turnNeck, this, 10, false)); // hay que añadir todos los argumentos en bind
    task_queue.push(bind(Player::turn, this, 10, false));
    task_queue.push(bind(&Player::turn, this, 10, false));
    task_queue.push(
        {[this, f]() {
            cout << "CICLO 3: Ejecutando Dash con poder " << i << endl;
            this->dash(10); 
        }}
    );

    Task next_task = task_queue.front();
    // Ejecutar la tarea (llama a la función enlazada)
    //next_task(this); 
    next_task(); 
    // Eliminar la tarea ejecutada
    task_queue.pop();

};