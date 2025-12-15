#include <player.hpp>

using namespace std;

class PlayerTest : public Player
{
public:

// poner despues de los () override para sobreescribir la funcion de player
// se pueden escribir metodos explusivos

    void play() override;


protected:
private:
    
    // Singleton vars
    PlayerTest() : Player(){};
    PlayerTest(string team_name, int player_number, char side, bool is_goalie) : Player(team_name, player_number, side, is_goalie){};

    friend class Player;
    

    
};

//void catchGoalie(double dir, bool override = false); // Only for goalie player