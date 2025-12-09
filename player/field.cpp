#include <field.hpp>
#include <utils.hpp>

// string without '(see (' and ))eof '(flag) dist dir) ((flag) dir) ... ((flag) ...'
void Field::calculatePositions(bool see_refresh) {

    function<dist(pair<posX, posY>, pair<posX, posY>)> point_2_point_dist {[](pair<posX, posY> p1, pair<posX, posY> p2){
        return sqrt(
            pow(p1.first - p2.first, 2)
             + 
            pow(p1.second - p2.second, 2)
        );
    }};

    if(see_refresh) {
        // min square error multilateration position calculation
        double err_ant = numeric_limits<double>::max();
        for (int iter = 0; iter < 100; iter++) {
            double sum_pow_err = 0;
            double x_delta = 0;
            double y_delta = 0;
            for(auto const& mark : marks_to_this_distance_and_dir) {
                double error = mark.second.first - point_2_point_dist(markers_positions[mark.first], {get<0>(me), get<1>(me)});

                sum_pow_err += pow(error, 2);

                x_delta += (error / (mark.second.first - error)) * (get<0>(me) - markers_positions[mark.first].first);
                y_delta += (error / (mark.second.first - error)) * (get<1>(me) - markers_positions[mark.first].second);
                //magia negra
            }

            if(abs(err_ant - sum_pow_err) < 1e-6){
                break;
            }
            err_ant = sum_pow_err;
            get<0>(me) += 0.1 * x_delta;
            get<1>(me) += 0.1 * y_delta;
        }

        std::cout << "------------------------------------------\n";
        std::cout << "📍 Mi Posición (me) actualizada:\n";
        std::cout << "   - X: " << std::get<0>(me) << "\n";
        std::cout << "   - Y: " << std::get<1>(me) << "\n";
        std::cout << "   - Dir: " << std::get<2>(me) << "\n";
        std::cout << "==========================================\n";

    } else {

        // estimate from data received actual 
            //me position
            //other players position
            //ball position
    }
}

void Field::parseSee(string const& s) {

    // (name) Direction
    // (name) Distance Direction
    // (name) Distance Direction DistChange 
    // (name) Distance Direction DistChange DirChange

    // now players_position is errased each cycle, must update older data if it´s possible
    players_position = {};

    marks_to_this_distance_and_dir = {}; 

    auto marks = split(s.substr(1, s.size()-2), ") (");

    for (auto mark: marks) {
        
        vector<string> aux = split(mark, ") ");
        string flag = aux.at(0) + ')';
        auto data = split(aux.at(1), ' ');

        if(markers_positions.find(flag) != markers_positions.end()){
            if (data.size() >= 2)
                marks_to_this_distance_and_dir.push_back({flag, {stod(data.at(0)), stod(data.at(1))}});
            else
                marks_to_this_distance_and_dir.push_back({flag, {-1, stod(data.at(1))}});
        
        } else if (flag == "(b)"){ //ball
            if (data.size() >= 2)
                ball_position = {stod(data.at(0)), stod(data.at(1))};
            else
                ball_position = {-1, stod(data.at(1))};
        } else {
            // now players_position is errased each cycle, must update older data if it´s possible
            if (data.size() >= 2)
                players_position.push_back({flag, stod(data.at(0)), stod(data.at(1))});
            else
                players_position.push_back({flag, -1, stod(data.at(1))});
        }
    }

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
            std::cout << "   - ID: " << std::get<0>(player) << ", Distancia: " << std::get<1>(player) << ", Dirección: " << std::get<2>(player) << "\n";
        }
    }
    

    Field::calculatePositions(true);
}