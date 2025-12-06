#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <MinimalSocket/udp/UdpSocket.h>

using namespace std;

ostream& operator<<(ostream& os, const tuple<string, MinimalSocket::Port, bool>& t);

tuple<string, MinimalSocket::Port, bool> parseArgs (int argc, char* argv[]);
vector<string> split(const string &s, char delimiter);
