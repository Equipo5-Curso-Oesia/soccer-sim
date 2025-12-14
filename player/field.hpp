//Tambien que defina las areas importantes y si se está en un area o no.
// TODO: implementar otro metodo de localizacion mejor.

//Crear una clase que gestione todo lo relacionado con la comunicacion
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <chrono>
#include <stdexcept>
#include <map>
#include <variant>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <optional>
#include <tuple>

// This class gather all info respect the elements in the field.
// It is a singleton class, only a single element is possible
// This class parse see server info and process positions
// To access positions getters and setters

using namespace std;

typedef optional<double> dist;
typedef optional<double> dist_change;
// Dir is relative to player sight, 0º means in front of it, looking at him, +90º means in his left, and -90º means in his right 
typedef optional<double> dir;
typedef optional<double> dir_chage;
typedef double posX;
typedef double posY;
typedef tuple<dist, dir, dist_change, dir_chage> PosData;

class Field
{
public:
    // Singleton constructor
    static Field& getInstance() 
    {
        if (!instance)
            instance = new Field();
        return *instance;
    };
    ~Field() = default;

    // Player positioning functions
    void resetPos() {
        me = {0, 0, 0};
    }
    void setMove(posX x, posY y);
    void setTurn(double dir);
    
    // Positioning functions
    void calculatePositions(int time, bool see_refresh = false);
    void parseSee(int time, string const &s);

protected:
private:

    // Singleton vars
    Field();
    inline static Field* instance = nullptr;

    // Player pos vars
    int parse_time = 0;
    tuple<posX, posY, dir> me{0, 0, 0}; // Dir is the sum of body dir and head dir

    // Parse vars
    vector<pair<string, PosData>> marks_to_this_distance_and_dir; 
    map<string, PosData> players_position;
    PosData ball_position = {};

    // Positioning algorithms functions
    void minPowErr();
    void triangulationAverage();

    // Harcoded flag pos map
    map<string, pair<posX, posY>> flags_positions = {

        // Own goal is the reference (x, y)=(0, 0)
        // Field length is x, width is y
        // Oponents goal is in x= 105 (105, 0)
        // Left is positive y, right is negative y

        // for players on server left side
        //{"(name)", {{105 - xPos, -yPos}}}
        // for players on server right side
        //{"(name)", {{xPos, yPos}}}

        // Left side behind goal line top to bottom
        {"(f l t 30)",   {-5, 30}},
        {"(f l t 20)",   {-5, 20}},
        {"(f l t 10)",   {-5, 10}},
        {"(f l 0)",      {-5, 0}},
        {"(f l b 10)",   {-5, -10}},
        {"(f l b 20)",   {-5, -20}},
        {"(f l b 30)",   {-5, -30}},

        // Left side goal line top to bottom
        // server_param::goal_width
        {"(f l t)",      {0, 34}},
        {"(f g l t)",    {0, 7}},
        {"(g l)",        {0, 0}},
        {"(f g l b)",    {0, -7}},
        {"(f l b)",      {0, -34}},

        // Left side penalti box top to bottom        
        {"(f p l t)",         {16.5, 20}},
        {"(f p l c)",         {16.5, 0}},
        {"(f p l b)",         {16.5, -20}},

        // Center top to bottom
        {"(f c t)",         {52.5, 34}},
        {"(f c)",         {52.5, 0}},
        {"(f c b)",         {52.5, -34}},

        // Right side penalti box top to bottom        
        {"(f p r t)",         {88.5, 20}},
        {"(f p r c)",         {88.5, 0}},
        {"(f p r b)",         {88.5, -20}},

        // Right side goal line top to bottom
        // server_param::goal_width
        {"(f r t)",      {105, 34}},
        {"(f g r t)",    {105, 7}},
        {"(g r)",        {105, 0}},
        {"(f g r b)",    {105, -7}},
        {"(f r b)",      {105, -34}},

        // Right side behind goal line top to bottom
        {"(f r t 30)",   {110, 30}},
        {"(f r t 20)",   {110, 20}},
        {"(f r t 10)",   {110, 10}},
        {"(f r 0)",      {110, 0}},
        {"(f r b 10)",   {110, -10}},
        {"(f r b 20)",   {110, -20}},
        {"(f r b 30)",   {110, -30}},

        // Top side band left to right
        {"(f t l 50)",    {2.5, 39}},
        {"(f t l 40)",    {12.5, 39}},
        {"(f t l 30)",    {22.5, 39}},
        {"(f t l 20)",    {32.5, 39}},
        {"(f t l 10)",    {42.5, 39}},
        {"(f t 0)",       {52.5, 39}},
        {"(f t r 10)",    {62.5, 39}},
        {"(f t r 20)",    {72.5, 39}},
        {"(f t r 30)",    {82.5, 39}},
        {"(f t r 40)",    {92.5, 39}},
        {"(f t r 50)",    {102.5, 39}},

        // Bottom side band left to right
        {"(f b l 50)",    {2.5, -39}},
        {"(f b l 40)",    {12.5, -39}},
        {"(f b l 30)",    {22.5, -39}},
        {"(f b l 20)",    {32.5, -39}},
        {"(f b l 10)",    {42.5, -39}},
        {"(f b 0)",       {52.5, -39}},
        {"(f b r 10)",    {62.5, -39}},
        {"(f b r 20)",    {72.5, -39}},
        {"(f b r 30)",    {82.5, -39}},
        {"(f b r 40)",    {92.5, -39}},
        {"(f b r 50)",    {102.5, -39}}

    };
    
    // Private methods
};

