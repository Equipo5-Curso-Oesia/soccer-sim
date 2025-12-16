#include <goalkeeper.hpp>
#include <server.hpp>
#include <field.hpp>
#include <utils.hpp>
#include <iostream>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

constexpr double kFieldLength = 105.0;

// Posición inicial en coordenadas de Field (0..105). Para el equipo izquierdo, la portería propia está en x=0.
// Para el equipo derecho, Field refleja la X automáticamente para que la portería propia esté en x=105.
constexpr double kHomeX = 0.5;
constexpr double kHomeY = 0.0;

// Mantener al portero en/cerca de la boca de la portería.
constexpr double kGoalMouthHalfY = 7.0;

// Área de penalti en coordenadas de Field (ver flags en `field.hpp`).
constexpr double kPenaltyAreaMaxX = 16.5;
constexpr double kPenaltyAreaHalfY = 20.0;

// Heurísticas (metros / grados).
constexpr double kKickableDist = 0.7; // coincide con `Defender`
constexpr double kCatchDist = 1.3;
constexpr double kChaseDist = 12.0;
constexpr double kTurnEpsDeg = 12.0;

double clamp(double v, double lo, double hi) {
    return std::max(lo, std::min(v, hi));
}

double normalizeDeg(double a) {
    while (a > 180.0) a -= 360.0;
    while (a < -180.0) a += 360.0;
    return a;
}

bool isInOurPenaltyArea(double x, double y) {
    return (x >= 0.0 && x <= kPenaltyAreaMaxX && std::abs(y) <= kPenaltyAreaHalfY);
}

} // namespace

void Goalkeeper::findBall(int i, optional<double> ballDir)
{
    if (i % 3 != 0) return;
    if (ballDir.has_value())
        turn(ballDir.value());
    else
        turn(20);
}

void Goalkeeper::play(){
    Server& s = Server::getInstance();
    Field& f = Field::getInstance();

    static int i = 0;
    static Server::GameState last_game_state = Server::GameState::unknown;
    if (last_game_state != s.getState()) i = 0;
    last_game_state = s.getState();

    // 1) Comenzar el partido en nuestra portería.
    if (s.getState() == Server::GameState::before_kick_off) {
        if (i == 0) {            
            move(kHomeX, kHomeY);
        } else {
            findBall(i, std::get<1>(f.getBall()));
        }
        i++;
        return;
    }

    // 2) Comportamiento principal del portero durante el juego.
    if (s.getState() == Server::GameState::play_on) {
        const auto ball = f.getBall();

        const auto ballDist = std::get<0>(ball);
        const auto ballDir = std::get<1>(ball);

        // Si no podemos ver el balón, escanear como el Defensor.
        if (!ballDir.has_value()) {
            findBall(i, ballDir);
            i++;
            return;
        }

        // Estimar la posición absoluta del balón (en coordenadas de Field) si es posible.
        const auto [px, py, pdirOpt] = f.getPlayerPos();
        std::optional<std::pair<double, double>> ballAbs;
        if (ballDist.has_value() && pdirOpt.has_value()) {
            const double theta = (pdirOpt.value() + ballDir.value()) * (PI / 180.0);
            ballAbs = {px + ballDist.value() * std::cos(theta), py + ballDist.value() * std::sin(theta)};
        }

        // Si el balón es pateabl, despejar fuerte.
        if (ballDist.has_value() && ballDist.value() <= kKickableDist) {
            if (std::abs(ballDir.value()) > 10.0) {
                turn(ballDir.value());
            } else {
                kick(100, 0);
            }
            i++;
            return;
        }

        // Atrapar cuando el balón está en nuestra área de penalti y suficientemente cerca.
        if (ballDist.has_value() && ballAbs.has_value()) {
            const double bx = ballAbs->first;
            const double by = ballAbs->second;

            if (ballDist.value() <= kCatchDist && isInOurPenaltyArea(bx, by)) {
                if (std::abs(ballDir.value()) > 5.0) {
                    turn(ballDir.value());
                    i++;
                    return;
                }

                // Comando `catch` (solo para portero). Misma convención de signo que `turn`.
                std::stringstream ss;
                ss << std::fixed << std::setprecision(3) << (ballDir.value() * -1);
                x("(catch " + ss.str() + ")");
                i++;
                return;
            }
        }

        // Si el balón está en nuestra área de penalti y aún no es atrapable, perseguirlo (pero quedarse dentro del área).
        if (ballDist.has_value() && ballAbs.has_value()) {
            const double bx = ballAbs->first;
            const double by = ballAbs->second;
            if (isInOurPenaltyArea(bx, by) && ballDist.value() <= kChaseDist) {
                if (std::abs(ballDir.value()) > 15.0) {
                    turn(ballDir.value());
                } else {
                    dash(100, ballDir.value());
                }
                i++;
                return;
            }
        }

        // De lo contrario: quedarse en el área de la línea de gol, centrado con la Y del balón si se conoce, y seguir rastreando.
        if (ballAbs.has_value() && pdirOpt.has_value()) {
            const double targetX = kHomeX;
            const double targetY = clamp(ballAbs->second, -kGoalMouthHalfY, kGoalMouthHalfY);
            const double dx = targetX - px;
            const double dy = targetY - py;
            const double distToTarget = std::hypot(dx, dy);
            if (distToTarget > 0.8) {
                const double targetAbsDeg = std::atan2(dy, dx) * (180.0 / PI);
                const double relDeg = normalizeDeg(targetAbsDeg - pdirOpt.value());
                if (std::abs(relDeg) > kTurnEpsDeg) turn(relDeg);
                else dash(60, relDeg);
                i++;
                return;
            }
        }

        turn(ballDir.value());
        i++;
        return;
    }

    // Otros modos de juego: simplemente seguir rastreando/escaneando.
    findBall(i, std::get<1>(f.getBall()));
    i++;
};