#include <utils.hpp>

ostream& operator<<(ostream& os, const tuple<string, MinimalSocket::Port, bool>& t) {
    os << "Team Name: " << get<0>(t) << ", Port: " << get<1>(t) << ", Is Goalie: " << (get<2>(t) ? "true" : "false");
    return os;
};

tuple<string, MinimalSocket::Port, bool> parseArgs (int argc, char* argv[]) {
    enum class ArgTypes {
        teamName,
        port,
        goalkeeper,
        unknown
    };

    function<ArgTypes(string)> hashString{[](string const& str) {
        if (str == "--team-name") return ArgTypes::teamName;
        if (str == "--port") return ArgTypes::port;
        if (str == "--is-goalie") return ArgTypes::goalkeeper;
        return ArgTypes::unknown;
    }};

    string team_name{""};
    MinimalSocket::Port port;
    bool is_goalie = false;
    
    for (int i = 0; i < argc; i++) {
        
        vector<string> arg = split(string(argv[i]), '=');
        if (arg.size() != 2) {
            throw invalid_argument("Unknown argument: " + (string)argv[i]);
        }

        switch(hashString(arg.at(0))) {
            case ArgTypes::teamName:
                team_name = arg.at(1);
                break;
            case ArgTypes::port:
                try{
                    port = stoi(arg.at(1));
                    if (port <= 0 || port > 65535) {
                        throw invalid_argument("Port number out of range: " + arg.at(1));
                    }
                } catch (const exception &e) {
                    throw invalid_argument("Invalid port number: " + arg.at(1));
                }
                break;
            case ArgTypes::goalkeeper:
                is_goalie = (arg.at(1) == "true");
                if (arg.at(1) != "true" && arg.at(1) != "false") {
                    throw invalid_argument("Invalid value for is-goalie: " + arg.at(1));
                }
                break;
            default:
                throw invalid_argument("Unknown argument: " + arg.at(0));
        }
    }

    string err_msg{""};
    if (team_name == "")
        err_msg += "--team-name={{TEAMNAME}} is required.\n";
    if (port == 0)
        err_msg += "--port={{PORT}} is required.\n";
    if (err_msg != "")
        throw invalid_argument(err_msg);    
    
    return make_tuple(team_name, port, is_goalie);
};

vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
};