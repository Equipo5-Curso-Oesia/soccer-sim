#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <MinimalSocket/udp/UdpSocket.h>
#include <cmath>

using namespace std;

ostream& operator<<(ostream& os, const tuple<string, MinimalSocket::Port, bool>& t);

tuple<string, MinimalSocket::Port, bool> parseArgs (int argc, char* argv[]);

vector<string> split(string const &s, char delimiter, function<string(string)> const& modFunct = {[](string s) {return s;}});
vector<string> split(string const& s, string delimiter);

const double PI{3.14159265358979323846};

// This class gather all info respect the elements in the field.
// It is a singleton class, only a single element is possible
// This class parse see server info and process positions
// To access positions getters and setters

typedef optional<double> dist;
typedef optional<double> dist_change;
// Dir is relative to player sight, 0º means in front of it, looking at him, +90º means in his left, and -90º means in his right 
typedef optional<double> dir;
typedef optional<double> dir_chage;
typedef double posX;
typedef double posY;
typedef tuple<dist, dir, dist_change, dir_chage> PosData;
double sum_angles (double ang1, double ang2);
double res_angles (double ang1, double ang2);