#include <player_test.hpp>
#include <server.hpp>
#include <field.hpp>

void PlayerTest::play(){

    Server& s = Server::getInstance();
    Field& f = Field::getInstance();

    int i = 0;
    if (s.getState() == Server::GameState::before_kick_off){
        //cout << i << endl;
        cout << "i" << endl;
        turn(20);
        //i++;
    }

    //cout << "klk loko PlayerTest: " << side << endl;

};