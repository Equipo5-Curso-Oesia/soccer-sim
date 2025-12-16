#include <player.hpp>

using namespace std;

class Forward : public Player
{
public:

// poner despues de los () override para sobreescribir la funcion de player
// se pueden escribir metodos explusivos

    void play() override;

protected:
private:
    
    // Singleton vars
    Forward() : Player(){};
    Forward(string team_name, int player_number, char side, bool is_goalie) : Player(team_name, player_number, side, is_goalie){};

    friend class Player;

    void findBall(int i, optional<double> ballDir);
    void goHome();
    
    
};

//void catchGoalie(double dir, bool override = false); // Only for goalie player
