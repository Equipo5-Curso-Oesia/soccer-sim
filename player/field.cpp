// TODO: Colision model
// TODO: 
#include <field.hpp>
#include <utils.hpp>
#include <server.hpp>
#include <player.hpp>
#include <optional>

const int MAX_ITERATIONS{200};
const double pos_err_convr_min {1e-4};
const double dir_err_convr_min{1e-6};

Field::Field(){
    Player& p = Player::getInstance();
    if (p.getSide() == 'r') {
        for (auto flag:flags_positions)
            flags_positions[flag.first] = {105 - flag.second.first, flag.second.second * -1};
    }
};

void Field::setMove(posX x, posY y) {
    get<0>(me) = x;
    get<1>(me) = y;
}
void Field::setTurn(double dir) {
    if (get<2>(me).has_value()){
        get<2>(me) = sum_angles(dir, get<2>(me).value());
    } else
        get<2>(me) = dir;
}

// string without '(see (' and ))eof '(flag) dist dir) ((flag) dir) ... ((flag) ...'
void Field::calculatePositions(int time, bool see_refresh) {
    if(see_refresh) {
        //minPowErr();
        triangulationAverage();
/*         std::cout << "------------------------------------------\n";
        std::cout << "📍 Mi Posición (me) actualizada:\n";
        std::cout << "   - X: " << std::get<0>(me) << "\n";
        std::cout << "   - Y: " << std::get<1>(me) << "\n";
        std::cout << "   - Dir: " << std::get<2>(me).value() << "\n";
        std::cout << "==========================================\n"; */
    } else {
        int diff_time = time - parse_time;
        // estimate from data received actual 
            //me position
            //other players position
            //ball position
    }
}

void Field::triangulationAverage() {
    function<optional<pair<posX, posY>>(double, pair<posX, posY>, double, pair<posX, posY>, double, pair<posX, posY>)> 
        triangulation {[](double d1, pair<posX, posY> p1, double d2, pair<posX, posY> p2, double d3, pair<posX, posY> p3)-> optional<pair<posX, posY>> {
        double A = 2*(p2.first - p1.first);
        double B = 2*(p2.second - p1.second);
        double C = d1*d1 - d2*d2 - p1.first*p1.first + p2.first*p2.first - p1.second*p1.second + p2.second*p2.second;

        double D = 2*(p3.first - p1.first);
        double E = 2*(p3.second - p1.second);
        double F = d1*d1 - d3*d3 - p1.first*p1.first + p3.first*p3.first - p1.second*p1.second + p3.second*p3.second;

        double denominator = A*E - B*D;
        
        // No solution: dots are aligned or it's imposible
        if (fabs(denominator) < 1e-12) 
            return nullopt;
        
        optional<pair<posX, posY>> x;
        x = {(C*E - B*F) / denominator, (A*F - C*D) / denominator};
        return make_pair((C*E - B*F) / denominator, (A*F - C*D) / denominator);
    }};
    function<double(pair<posX, posY>, pair<posX, posY>)> point_2_point_abs_angle {[](pair<posX, posY> p1, pair<posX, posY> p2) {
        return atan2(p2.second - p1.second, p2.first - p1.first)*(180/PI);
    }};

    double acum_avg_weight_pos{0};
    pair<posX, posY> avg_pos{0, 0};

    for (int i{0}; i < (int)marks_to_this_distance_and_dir.size(); i ++) {
        auto mark_i = marks_to_this_distance_and_dir.at(i);
        if (!get<0>(mark_i.second).has_value()) continue;
        for (int j{i+1}; j < (int)marks_to_this_distance_and_dir.size(); j++) {
            auto mark_j = marks_to_this_distance_and_dir.at(j);
            if (!get<0>(mark_j.second).has_value()) continue;
            for (int k{j+1}; k < (int)marks_to_this_distance_and_dir.size(); k++) {
                auto mark_k = marks_to_this_distance_and_dir.at(k);
                if (!get<0>(mark_k.second).has_value()) continue;
                double avg_weight_pos = 1/(get<0>(mark_i.second).value() + get<0>(mark_j.second).value() + get<0>(mark_k.second).value());
                optional<pair<posX, posY>> calculated_pos = triangulation(get<0>(mark_i.second).value(), flags_positions.at(mark_i.first), 
                                                                          get<0>(mark_j.second).value(), flags_positions.at(mark_j.first), 
                                                                          get<0>(mark_k.second).value(), flags_positions.at(mark_k.first));
                if (calculated_pos) {
                    acum_avg_weight_pos += avg_weight_pos;
                    avg_pos = {(calculated_pos->first * avg_weight_pos) + avg_pos.first, (calculated_pos->second * avg_weight_pos) + avg_pos.second};
                }
            }
        }
    }

    if (acum_avg_weight_pos <= 0) {
        // Not enough valid information to triangulate.
        return;
    }

    get<0>(me) = avg_pos.first/acum_avg_weight_pos;
    get<1>(me) = avg_pos.second/acum_avg_weight_pos;

    double acum_avg_weight_dir{0};
    double avg_dir_x{0};
    double avg_dir_y{0};

    for (int i{0}; i < (int)marks_to_this_distance_and_dir.size(); i ++) {
        auto mark_i = marks_to_this_distance_and_dir.at(i);
        if (!get<0>(mark_i.second).has_value() || !get<1>(mark_i.second).has_value()) continue;
        double avg_weight_dir = get<0>(mark_i.second).value();
        acum_avg_weight_dir += avg_weight_dir;

        double calculated_dir = point_2_point_abs_angle({get<0>(me), get<1>(me)}, flags_positions.at(mark_i.first)) - get<1>(mark_i.second).value();

        avg_dir_x += (cos(calculated_dir*PI/180) * avg_weight_dir);
        avg_dir_y += (sin(calculated_dir*PI/180) * avg_weight_dir);
    }

    if (acum_avg_weight_dir <= 0) {
        return;
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
                auto const dist_measured {get<0>(mark.second)};
                auto const dist_calculated {point_2_point_dist(flags_positions.at(mark.first), {get<0>(me), get<1>(me)})};//markers_positions.at(mark.first), {get<0>(me), get<1>(me)})};
                double error {dist_measured.value() - dist_calculated};
                sum_pow_err_pos += pow(error, 2);

                auto const x_diff {get<0>(me) - flags_positions.at(mark.first).first};
                x_delta += (error / dist_calculated) * x_diff;
                auto const y_diff {get<1>(me) - flags_positions.at(mark.first).second};
                y_delta += (error / dist_calculated) * y_diff;
            } else {x_delta = 0; y_delta = 0;}                 
            // Direction aproximation
            if (err_convr_dir > dir_err_convr_min) {                   
                auto const dir_absolute_estimate {get<2>(me)};
                auto const dir_partial_measured {get<1>(mark.second).value()};
                auto const dir_partial_calculated {point_2_point_abs_angle(flags_positions.at(mark.first), {get<0>(me), get<1>(me)})};

                double error = dir_absolute_estimate.value() - dir_partial_measured - dir_partial_calculated;
                // Ver como hacerlo correctamente para que de el valor correcto en -180,180
                sum_pow_err_dir += pow(error, 2);

                auto const dir_diff {error};
                dir_delta -= (error / (dir_partial_measured + dir_partial_calculated)) * dir_diff;
            } else {dir_delta = 0;}
        }
        get<0>(me) += correction_rate_pos * x_delta;
        get<1>(me) += correction_rate_pos * y_delta;
        //cout << "correction_rate_pos: " << correction_rate_pos << " x: " << get<0>(me) << " y: " << get<1>(me) << " delta_x: " << x_delta << "  delta_y: " << y_delta << "  sum_pow_err_pos: " << sum_pow_err_pos << endl;
        //get<2>(me) += correction_rate_dir * dir_delta;
        //get<2>(me) = fmod(get<2>(me), 180);
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

    function<PosData(vector<string>&)> posMakeTuple{[](vector<string> &data)
    {
        return make_tuple(
            (data.size()< 2)? optional<double>{} : stod(data.at(0)),
            (data.size()< 2)? stod(data.at(0)) * -1 : stod(data.at(1)) * -1,
            (data.size()< 3)? optional<double>{} : stod(data.at(2)),
            (data.size()< 4)? optional<double>{} : (stod(data.at(3)) * -1));
    }};

    parse_time = time;
    // now players_position is errased each cycle, must update older data if it´s possible
    players_position = {};

    marks_to_this_distance_and_dir = {};

    if (s.size() < 2) {
        Field::calculatePositions(parse_time, true);
        return;
    }

    string inner = s;
    if (!inner.empty() && inner.front() == '(' && inner.back() == ')') {
        inner = inner.substr(1, inner.size() - 2);
    }
    if (inner.empty()) {
        Field::calculatePositions(parse_time, true);
        return;
    }

    auto marks = split(inner, ") (");
    for (auto mark: marks) {
        // Each entry is typically like: "(b) 10 20" or "(f t l 50) 30 -10"
        // but we must handle cases without data gracefully.
        auto close = mark.find(") ");
        if (close == string::npos) {
            // No data part.
            continue;
        }
        string flag = mark.substr(0, close + 1);
        string rest = mark.substr(close + 2);
        if (rest.empty()) continue;
        auto data = split(rest, ' ');
        if (data.empty()) continue;

        if(flags_positions.find(flag) != flags_positions.end()){
            marks_to_this_distance_and_dir.push_back(
                make_pair(
                    flag,
                    posMakeTuple(data)
                ));
        } else if (flag == "(b)"){ //ball
            ball_position = posMakeTuple(data);       
        } else {
            // now players_position is errased each cycle, must update older data if it´s possible
            players_position[flag]= posMakeTuple(data);
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

std::optional<double> Field::getBallDist() const {
    if (get<0>(ball_position).has_value())
        return get<0>(ball_position).value();
    return std::nullopt;
}

std::optional<double> Field::getBallDir() const {
    if (get<1>(ball_position).has_value())
        return get<1>(ball_position).value();
    return std::nullopt;
}
