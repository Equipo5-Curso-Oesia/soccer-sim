#pragma once
#include <memory>
class Server;
class Field;
class Player;

class Role {
public:
    virtual ~Role() = default;
    virtual void setInitialPosition(Player& p) = 0;
    virtual void playCycle(Player& p, Server& s, Field& f) = 0;
};

class Goalkeeper : public Role {
public:
    void setInitialPosition(Player& p) override;
    void playCycle(Player& p, Server& s, Field& f) override;
};

class Defense : public Role {
public:
    void setInitialPosition(Player& p) override;
    void playCycle(Player& p, Server& s, Field& f) override;
};

class Forward : public Role {
public:
    void setInitialPosition(Player& p) override;
    void playCycle(Player& p, Server& s, Field& f) override;
};
