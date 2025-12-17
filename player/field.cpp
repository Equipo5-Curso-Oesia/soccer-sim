// TODO: Colision model
// TODO: 
#include <field.hpp>
#include <utils.hpp>
#include <server.hpp>
#include <player.hpp>
#include <optional>

#include <cmath>
#include <limits>



Field::Field(){
    Player& p = Player::getInstance<Player>();

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
    if (!std::isfinite(dir)) return;
    if (get<2>(me).has_value()){
        get<2>(me) = sum_angles(dir, get<2>(me).value());
    } else
        get<2>(me) = dir;
}

// string without '(see (' and ))eof '(flag) dist dir) ((flag) dir) ... ((flag) ...'
void Field::calculatePositions(int time, bool see_refresh) {
    if(see_refresh) {
        minPowErr();
        //triangulationAverage();
        
        std::cout << "------------------------------------------\n";
        std::cout << "📍 Mi Posición (me) actualizada:\n";
        std::cout << "   - X: " << std::get<0>(me) << "\n";
        std::cout << "   - Y: " << std::get<1>(me) << "\n";
        std::cout << "   - Dir: " << std::get<2>(me).value() << "\n";
        std::cout << "==========================================\n";
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
    int used_pos_solutions{0};

    const posX prev_x = get<0>(me);
    const posY prev_y = get<1>(me);
    const std::optional<double> prev_dir = get<2>(me);

    for (int i{0}; i < marks_to_this_distance_and_dir.size(); i ++) {
        auto mark_i = marks_to_this_distance_and_dir.at(i);
        if (!get<0>(mark_i.second).has_value() || !std::isfinite(get<0>(mark_i.second).value())) continue;
        for (int j{i+1}; j < marks_to_this_distance_and_dir.size(); j++) {
            auto mark_j = marks_to_this_distance_and_dir.at(j);
            if (!get<0>(mark_j.second).has_value() || !std::isfinite(get<0>(mark_j.second).value())) continue;
            for (int k{j+1}; k < marks_to_this_distance_and_dir.size(); k++) {
                auto mark_k = marks_to_this_distance_and_dir.at(k);
                if (!get<0>(mark_k.second).has_value() || !std::isfinite(get<0>(mark_k.second).value())) continue;
                const double di = get<0>(mark_i.second).value();
                const double dj = get<0>(mark_j.second).value();
                const double dk = get<0>(mark_k.second).value();
                const double denom = di + dj + dk;
                if (!(denom > 0.0) || !std::isfinite(denom)) continue;

                double avg_weight_pos = 1.0 / denom;
                acum_avg_weight_pos += avg_weight_pos;
                optional<pair<posX, posY>> calculated_pos = triangulation(di, flags_positions.at(mark_i.first),
                                                                          dj, flags_positions.at(mark_j.first),
                                                                          dk, flags_positions.at(mark_k.first));
                if (calculated_pos && std::isfinite(calculated_pos->first) && std::isfinite(calculated_pos->second)) {
                    avg_pos = {(calculated_pos->first * avg_weight_pos) + avg_pos.first, (calculated_pos->second * avg_weight_pos) + avg_pos.second};
                    used_pos_solutions++;
                }
            }
        }
    }

    // Not enough valid data: keep previous estimate.
    if (used_pos_solutions <= 0 || !(acum_avg_weight_pos > 0.0) || !std::isfinite(acum_avg_weight_pos)) {
        get<0>(me) = prev_x;
        get<1>(me) = prev_y;
        get<2>(me) = prev_dir;
        return;
    }

    const double new_x = avg_pos.first / acum_avg_weight_pos;
    const double new_y = avg_pos.second / acum_avg_weight_pos;
    if (!std::isfinite(new_x) || !std::isfinite(new_y)) {
        get<0>(me) = prev_x;
        get<1>(me) = prev_y;
        get<2>(me) = prev_dir;
        return;
    }

    get<0>(me) = new_x;
    get<1>(me) = new_y;

    double acum_avg_weight_dir{0};
    double avg_dir_x{0};
    double avg_dir_y{0};

    int used_dir_marks{0};

    for (int i{0}; i < marks_to_this_distance_and_dir.size(); i ++) {
        auto mark_i = marks_to_this_distance_and_dir.at(i);
        if (!get<0>(mark_i.second).has_value() || !get<1>(mark_i.second).has_value()) continue;
        if (!std::isfinite(get<0>(mark_i.second).value()) || !std::isfinite(get<1>(mark_i.second).value())) continue;

        double avg_weight_dir = get<0>(mark_i.second).value();
        if (!(avg_weight_dir > 0.0) || !std::isfinite(avg_weight_dir)) continue;
        acum_avg_weight_dir += avg_weight_dir;

        double calculated_dir = point_2_point_abs_angle({get<0>(me), get<1>(me)}, flags_positions.at(mark_i.first)) - get<1>(mark_i.second).value();
        if (!std::isfinite(calculated_dir)) continue;

        avg_dir_x += (cos(calculated_dir*PI/180) * avg_weight_dir);
        avg_dir_y += (sin(calculated_dir*PI/180) * avg_weight_dir);
        used_dir_marks++;
    }

    if (used_dir_marks <= 0 || !(acum_avg_weight_dir > 0.0) ||
        (!std::isfinite(avg_dir_x) || !std::isfinite(avg_dir_y))) {
        get<2>(me) = prev_dir;
        return;
    }

    const double new_dir = atan2(avg_dir_y, avg_dir_x) * (180 / PI);
    if (!std::isfinite(new_dir)) {
        get<2>(me) = prev_dir;
        return;
    }
    get<2>(me) = new_dir;
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
        return atan2(
            p2.second 
            - 
            p1.second, p2.first - p1.first
        );
    }};

    const int MAX_ITERATIONS{200};
    const double pos_err_convr_min {1e-4};
    const double dir_err_convr_min{1e-6};
    
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
            auto const& flag_pos = flags_positions.at(mark.first);
            double my_x = get<0>(me);
            double my_y = get<1>(me);

            double dist_calc = point_2_point_dist(flag_pos, {my_x, my_y});
            double dist_meas = get<0>(mark.second).value();
            double dist_err = dist_meas - dist_calc; // Positivo si estoy más cerca de lo que debería

            double ux = (my_x - flag_pos.first) / (dist_calc + 1e-6);
            double uy = (my_y - flag_pos.second) / (dist_calc + 1e-6);

            x_delta += dist_err * ux; 
            y_delta += dist_err * uy;
            sum_pow_err_pos += dist_err * dist_err;



            double current_dir = get<2>(me).value(); 
            double relative_meas = get<1>(mark.second).value();

            double angle_to_mark_deg = point_2_point_abs_angle({my_x, my_y}, flag_pos) * (180.0 / M_PI);

            double dir_err = angle_to_mark_deg - (current_dir + relative_meas);
            
            while (dir_err > 180.0)  dir_err -= 360.0;
            while (dir_err < -180.0) dir_err += 360.0;

            dir_delta += dir_err; 
            sum_pow_err_dir += dir_err * dir_err;


        }
        get<0>(me) += correction_rate_pos * x_delta;
        get<1>(me) += correction_rate_pos * y_delta;

        double avg_dir_delta = dir_delta / marks_to_this_distance_and_dir.size();
        double new_dir = get<2>(me).value() + (correction_rate_dir * avg_dir_delta);

        while (new_dir > 180.0)  new_dir -= 360.0;
        while (new_dir < -180.0) new_dir += 360.0;
        get<2>(me) = new_dir;


        err_convr_pos = abs(err_ant_pos - sum_pow_err_pos);
        err_convr_dir = abs(err_ant_dir - sum_pow_err_dir);

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
            (data.size()< 2)? optional<double>{}    :    ((data.at(0) != "k" && data.at(0) != "t") ? stod(data.at(0))      : optional<double>{}),
            (data.size()< 2)? stod(data.at(0)) * -1 :    ((data.at(1) != "k" && data.at(1) != "t") ? stod(data.at(1)) * -1 : optional<double>{}),
            (data.size()< 3)? optional<double>{}    :    ((data.at(2) != "k" && data.at(2) != "t") ? stod(data.at(2))      : optional<double>{}),
            (data.size()< 4)? optional<double>{}    :    ((data.at(3) != "k" && data.at(3) != "t") ? stod(data.at(3)) * -1 : optional<double>{})
        );
    }};

    parse_time = time;
    // now players_position is errased each cycle, must update older data if it´s possible
    players_position = {};

    marks_to_this_distance_and_dir = {}; 
    ball_position = {nullopt, nullopt, nullopt, nullopt};

    auto marks = split(s.substr(1, s.size()-2), ") (");
    for (auto mark: marks) {        
        vector<string> aux = split(mark, ") ");
        string flag = aux.at(0) + ')';
        auto data = split(aux.at(1), ' ');

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
