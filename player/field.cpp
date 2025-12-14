#include <field.hpp>
#include <utils.hpp>
#include <server.hpp>
#include <player.hpp>
#include <optional>

const double PI{3.14159265358979323846};
const int MAX_ITERATIONS{200};
const double pos_err_convr_min {1e-4};
const double dir_err_convr_min{1e-6};

Field::Field(){
    Player& p = Player::getInstance();
    if (p.getSide() == 'r') {
        //get<2>(me) =  180;
        for (auto flag:flags_positions)
            flags_positions[flag.first] = {105 - flag.second.first, flag.second.second * -1};
    }
/*     for (auto flag:flags_positions){
        cout << flag.first << endl;
        cout << "flag position: " << flags_positions[flag.first].first << ", " << flags_positions[flag.first].second << endl;
        cout << "new flag position: " << flags_positions[flag.first].first << ", " << flags_positions[flag.first].second << endl << endl;} */
};

// string without '(see (' and ))eof '(flag) dist dir) ((flag) dir) ... ((flag) ...'
void Field::calculatePositions(int time, bool see_refresh) {
    if(see_refresh) {

        //minPowErr();
        triangulationAverage();

        std::cout << "------------------------------------------\n";
        std::cout << "📍 Mi Posición (me) actualizada:\n";
        std::cout << "   - X: " << std::get<0>(me) << "\n";
        std::cout << "   - Y: " << std::get<1>(me) << "\n";
        std::cout << "   - Dir: " << std::get<2>(me) << "\n";
        std::cout << "==========================================\n";

    } else {

        int diff_time = time - parse_time;
        cout << "Time sinse last position parse: " << diff_time << endl;

        // estimate from data received actual 
            //me position
            //other players position
            //ball position
    }
}

void Field::triangulationAverage() {
    function<optional<pair<posX, posY>>(double, pair<posX, posY>, double, pair<posX, posY>, double, pair<posX, posY>)> 
        triangulation {[](double d1, pair<posX, posY> p1, double d2, pair<posX, posY> p2, double d3, pair<posX, posY> p3)-> optional<pair<posX, posY>> {
        // Transformación para resolver el sistema
        double A = 2*(p2.first - p1.first);
        double B = 2*(p2.second - p1.second);
        double C = d1*d1 - d2*d2 - p1.first*p1.first + p2.first*p2.first - p1.second*p1.second + p2.second*p2.second;

        double D = 2*(p3.first - p1.first);
        double E = 2*(p3.second - p1.second);
        double F = d1*d1 - d3*d3 - p1.first*p1.first + p3.first*p3.first - p1.second*p1.second + p3.second*p3.second;

        double denominator = A*E - B*D;
        
        // No hay solución: los puntos están alineados o es imposible
        if (fabs(denominator) < 1e-12) 
            return nullopt;
        
        optional<pair<posX, posY>> x;
        x = {(C*E - B*F) / denominator, (A*F - C*D) / denominator};
        return make_pair((C*E - B*F) / denominator, (A*F - C*D) / denominator);//(optional<pair<posX, posY>>);
    }};
    function<double(pair<posX, posY>, pair<posX, posY>)> point_2_point_abs_angle {[](pair<posX, posY> p1, pair<posX, posY> p2) {
        return atan2(p2.second - p1.second, p2.first - p1.first)*(180/PI);
    }};

    double acum_avg_weight_pos{0};
    pair<posX, posY> avg_pos{0, 0};

    for (int i{0}; i < marks_to_this_distance_and_dir.size(); i ++) {
        auto mark_i = marks_to_this_distance_and_dir.at(i);
        for (int j{i+1}; j < marks_to_this_distance_and_dir.size(); j++) {
            auto mark_j = marks_to_this_distance_and_dir.at(j);
            for (int k{j+1}; k < marks_to_this_distance_and_dir.size(); k++) {
                auto mark_k = marks_to_this_distance_and_dir.at(k);
                double avg_weight_pos = 1/(mark_i.second.first + mark_j.second.first + mark_k.second.first);
                acum_avg_weight_pos += avg_weight_pos;
                optional<pair<posX, posY>> calculated_pos = triangulation(mark_i.second.first, flags_positions.at(mark_i.first), 
                                                                          mark_j.second.first, flags_positions.at(mark_j.first), 
                                                                          mark_k.second.first, flags_positions.at(mark_k.first));
                if (calculated_pos)
                    avg_pos = {(calculated_pos->first * avg_weight_pos) + avg_pos.first, (calculated_pos->second * avg_weight_pos) + avg_pos.second};
            }
        }
    }
    get<0>(me) = avg_pos.first/acum_avg_weight_pos;
    get<1>(me) = avg_pos.second/acum_avg_weight_pos;

    double acum_avg_weight_dir{0};
    double avg_dir_x{0};
    double avg_dir_y{0};

    for (int i{0}; i < marks_to_this_distance_and_dir.size(); i ++) {
        auto mark_i = marks_to_this_distance_and_dir.at(i);
        double avg_weight_dir = mark_i.second.first;
        acum_avg_weight_dir += avg_weight_dir;
/*         cout << "me: " << get<0>(me) << ", " << get<1>(me) << " flag" << mark_i.first << ": " << flags_positions.at(mark_i.first).first << ", " << flags_positions.at(mark_i.first).second << endl;
        cout << "absolute dir flag to player: " << point_2_point_abs_angle({get<0>(me), get<1>(me)}, flags_positions.at(mark_i.first)) 
             << " relative angle of player and flag: " << mark_i.second.second << endl;
 */
        double calculated_dir = point_2_point_abs_angle({get<0>(me), get<1>(me)}, flags_positions.at(mark_i.first)) - mark_i.second.second;

        avg_dir_x += (cos(calculated_dir*PI/180) * avg_weight_dir);
        avg_dir_y += (sin(calculated_dir*PI/180) * avg_weight_dir);
    }

    get<2>(me) = atan2(avg_dir_y, avg_dir_x)*(180/PI);
}

void Field::minPowErr() { // No funciona

    function<double(pair<posX, posY>, pair<posX, posY>)> point_2_point_dist {[](pair<posX, posY> p1, pair<posX, posY> p2){
        return sqrt(
            pow(p1.first - p2.first, 2)
            + 
            pow(p1.second - p2.second, 2)
        );
    }};
    function<double(pair<posX, posY>, pair<posX, posY>)> point_2_point_abs_angle {[](pair<posX, posY> p1, pair<posX, posY> p2) {
        return atan2(p2.second - p1.second, p2.first - p1.first);
    }};
    double correction_rate_pos {0.1};
    double correction_rate_dir {0.1};
    
    // min square error multilateration player position calculation
    double err_ant_pos {numeric_limits<double>::max()};        
    double err_ant_dir {numeric_limits<double>::max()};
    double err_convr_pos {numeric_limits<double>::max()};
    double err_convr_dir {numeric_limits<double>::max()};

    int iterations {0};
    while ((err_convr_pos > pos_err_convr_min 
            || 
            err_convr_dir > dir_err_convr_min)
            &&
            iterations < MAX_ITERATIONS){

        double sum_pow_err_pos {0};
        double x_delta {0};
        double y_delta {0};
        double sum_pow_err_dir {0};
        double dir_delta {0};
        for(auto const& mark : marks_to_this_distance_and_dir) {
            //magia negra
            // Position aproximation
            if (err_convr_pos > pos_err_convr_min){      
                auto const dist_measured {mark.second.first};
                auto const dist_calculated {point_2_point_dist(flags_positions.at(mark.first), {get<0>(me), get<1>(me)})};//markers_positions.at(mark.first), {get<0>(me), get<1>(me)})};
                double error {dist_measured - dist_calculated};
                sum_pow_err_pos += pow(error, 2);

                auto const x_diff {get<0>(me) - flags_positions.at(mark.first).first};
                x_delta += (error / dist_calculated) * x_diff;
                auto const y_diff {get<1>(me) - flags_positions.at(mark.first).second};
                y_delta += (error / dist_calculated) * y_diff;
            } else {x_delta = 0; y_delta = 0;}                 
            // Direction aproximation
            if (err_convr_dir > dir_err_convr_min) {                   
                auto const dir_absolute_estimate {get<2>(me)};
                auto const dir_partial_measured {mark.second.second};
                auto const dir_partial_calculated {point_2_point_abs_angle(flags_positions.at(mark.first), {get<0>(me), get<1>(me)})};

                double error = dir_absolute_estimate - dir_partial_measured - dir_partial_calculated;
                // Ver como hacerlo correctamente para que de el valor correcto en -180,180
                sum_pow_err_dir += pow(error, 2);

                auto const dir_diff {error};
                dir_delta -= (error / (dir_partial_measured + dir_partial_calculated)) * dir_diff;
            } else {dir_delta = 0;}
        }
        get<0>(me) += correction_rate_pos * x_delta;
        get<1>(me) += correction_rate_pos * y_delta;
        //cout << "correction_rate_pos: " << correction_rate_pos << " x: " << get<0>(me) << " y: " << get<1>(me) << " delta_x: " << x_delta << "  delta_y: " << y_delta << "  sum_pow_err_pos: " << sum_pow_err_pos << endl;
        get<2>(me) += correction_rate_dir * dir_delta;
        get<2>(me) = fmod(get<2>(me), 180);
        //cout << "correction_rate_dir: " << correction_rate_dir << "dir: " << get<2>(me) << " delta_dir: " << dir_delta << " sum_pow_err_dir: " << sum_pow_err_dir << endl << endl;

        err_convr_pos = abs(err_ant_pos - sum_pow_err_pos);
        err_convr_dir = abs(err_ant_dir - sum_pow_err_dir);

        // Dynamic correction rate adjust, if error rate is larger than last iteration, oscilation is ocurring and correction rate reduced
        /* if(sum_pow_err_pos > err_ant_pos)
            correction_rate_pos *= (err_ant_pos/sum_pow_err_pos > 0.0001) ?(err_ant_pos/sum_pow_err_pos) : 0.0001;
        if(sum_pow_err_dir > err_ant_dir)
            correction_rate_dir *= (err_ant_dir/sum_pow_err_dir > 0.0001) ? (err_ant_dir/sum_pow_err_dir) : 0.0001; */

        err_ant_pos = sum_pow_err_pos;
        err_ant_dir = sum_pow_err_dir;

        iterations++; 
    }

}

void Field::parseSee(int time, string const& s){
    // (name) Direction
    // (name) Distance Direction
    // (name) Distance Direction DistChange 
    // (name) Distance Direction DistChange DirChange

    parse_time = time;
    // now players_position is errased each cycle, must update older data if it´s possible
    players_position = {};

    marks_to_this_distance_and_dir = {}; 

    auto marks = split(s.substr(1, s.size()-2), ") (");
    for (auto mark: marks) {        
        vector<string> aux = split(mark, ") ");
        string flag = aux.at(0) + ')';
        auto data = split(aux.at(1), ' ');

        if(flags_positions.find(flag) != flags_positions.end()){
            if (data.size() >= 2)
                marks_to_this_distance_and_dir.push_back({flag, {stod(data.at(0)), stod(data.at(1)) * -1}});
            else
                marks_to_this_distance_and_dir.push_back({flag, {-1, stod(data.at(1)) * -1}});
        
        } else if (flag == "(b)"){ //ball
            if (data.size() >= 2)
                ball_position = {stod(data.at(0)), stod(data.at(1)) * -1};
            else
                ball_position = {-1, stod(data.at(1)) * -1};
        } else {
            // now players_position is errased each cycle, must update older data if it´s possible
            if (data.size() >= 2)
                players_position[flag]= {stod(data.at(0)), stod(data.at(1)) * -1};
            else
                players_position[flag] = {-1, stod(data.at(1)) * -1};
        }
    }
/* 
    std::cout << "==========================================\n";
    std::cout << "           ESTADO DEL JUEGO               \n";
    std::cout << "==========================================\n";

    // 1. Imprimir la tupla 'me' (Mi Posición)
    std::cout << "📍 Mi Posición (me):\n";
    std::cout << "   - X: " << std::get<0>(me) << "\n";
    std::cout << "   - Y: " << std::get<1>(me) << "\n";
    std::cout << "   - Dir: " << std::get<2>(me) << "\n";
    std::cout << "------------------------------------------\n";

    // 2. Imprimir 'ball_position' (Posición del Balón)
    std::cout << "⚽ Posición del Balón (ball_position):\n";
    std::cout << "   - Distancia: " << ball_position.first << "\n";
    std::cout << "   - Dirección: " << ball_position.second << "\n";
    std::cout << "------------------------------------------\n";

    // 3. Imprimir 'marks_to_this_distance_and_dir' (Marcas de Referencia)
    std::cout << "🚩 Marcas de Referencia (marks_to_this_distance_and_dir):\n";
    if (marks_to_this_distance_and_dir.empty()) {
        std::cout << "   (No hay marcas)\n";
    } else {
        for (const auto& mark : marks_to_this_distance_and_dir) {
            // mark.first es el string (nombre de la marca)
            // mark.second es el pair<dist, dir>
            std::cout << "   - Marca: " << mark.first << ", Distancia: " << mark.second.first << ", Dirección: " << mark.second.second << "\n";
        }
    }
    std::cout << "------------------------------------------\n";

    // 4. Imprimir 'players_position' (Posiciones de Jugadores)
    std::cout << "🏃 Posiciones de Jugadores (players_position):\n";
    if (players_position.empty()) {
        std::cout << "   (No hay jugadores)\n";
    } else {
        for (const auto& player : players_position) {
            // Los elementos de la tupla se acceden por índice (0, 1, 2)
            std::cout << "   - ID: " << player.first << ", Distancia: " << player.second.first << ", Dirección: " << player.second.second << "\n";
        }
    }
*/
    Field::calculatePositions(parse_time, true);
}