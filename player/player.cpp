#include <player.hpp>


struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2() = default;
    Vec2(double _x, double _y) : x(_x), y(_y) {}

    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }

    double length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const {
        double L = length();
        return (L > 1e-9) ? Vec2(x / L, y / L) : Vec2(0, 0);
    }
};

class Player {
public:
    Player(int id, const std::string& name, const Vec2& startPos = Vec2{})
        : id_(id), name_(name), pos_(startPos) {}

    // Basic tick update, dt in seconds
    void update(double dt) {
        if (moving_) {
            Vec2 toTarget = target_ - pos_;
            double dist = toTarget.length();
            if (dist < 1e-6) {
                vel_ = Vec2{};
                moving_ = false;
            } else {
                Vec2 dir = toTarget.normalized();
                // simple speed interpolation
                double desiredSpeed = maxSpeed_;
                Vec2 desiredVel = dir * desiredSpeed;
                // simple smoothing
                vel_ = vel_ * 0.8 + desiredVel * 0.2;
                pos_ = pos_ + vel_ * dt;
                // stop when close
                if ((target_ - pos_).length() < 0.05) {
                    pos_ = target_;
                    vel_ = Vec2{};
                    moving_ = false;
                }
            }
            // stamina drains while moving
            stamina_ = std::max(0.0, stamina_ - staminaDrainPerSec_ * dt);
        }
    }

    void moveTo(const Vec2& target) {
        target_ = target;
        moving_ = true;
    }

    // Kick the ball with a given power in range [0,1]
    void kick(double power) {
        power = std::min(1.0, std::max(0.0, power));
        double actualPower = power * kickPower_ * (0.5 + 0.5 * (stamina_)); // stamina affects kick
        // In a real sim you'd apply this to the ball; here we just print it
        std::cout << "Player " << id_ << " (" << name_ << ") kicks with power " << actualPower << "\n";
        stamina_ = std::max(0.0, stamina_ - 0.1 * power);
    }

    // Accessors
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    Vec2 position() const { return pos_; }
    double stamina() const { return stamina_; }

    // Simple status print
    void debugPrint() const {
        std::cout << "Player " << id_ << " '" << name_ << "' pos(" << pos_.x << "," << pos_.y
                  << ") vel(" << vel_.x << "," << vel_.y << ") stamina=" << stamina_
                  << (moving_ ? " moving\n" : " idle\n");
    }

private:
    int id_;
    std::string name_;
    Vec2 pos_{}, vel_{}, target_{};
    bool moving_ = false;

    // Tunable parameters
    double maxSpeed_ = 3.0;             // m/s
    double kickPower_ = 20.0;           // arbitrary units
    double stamina_ = 1.0;              // 0..1
    double staminaDrainPerSec_ = 0.05;  // per second while moving
};

// Example usage (comment out or remove in production):
/*
int main() {
    Player p(7, "Striker", Vec2{0,0});
    p.moveTo(Vec2{10, 0});
    for (int i = 0; i < 100; ++i) {
        p.update(0.1);
        p.debugPrint();
    }
    p.kick(0.8);
    return 0;
}
*/