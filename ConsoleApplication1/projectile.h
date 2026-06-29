#pragma once
#include"common.h"

class Player;

struct Projectile {
    float x = 0, y = 0;
    float w = 0, h = 0;
    int type = 0; // 0:Orb, 1:Sword, 2:Beam 3:Laser
    bool active = true;

    virtual void update(Player& p) = 0;
    virtual void draw() = 0;
    virtual Rect getRect();
    virtual void drawDebug();
};

enum OrbState { SPAWNING, CHARGING, RECOVERING };

struct Orb : Projectile {
    OrbState state = SPAWNING;
    float vx, vy;
    float currentSpeed;
    float orbScale = 0.2f;

    const float MAX_SPEED = 14.0f;
    const float TURN_FORCE = 0.12f;
    const int   MAX_ATTACKS = 2;

    DWORD stateStartTime;
    DWORD spawnTime;
    int attackCount = 0;

    Orb(float sx, float sy, float px, float py);
    void update(Player& p) override;
    void draw() override;
    Rect getRect() override;
};

enum SwordState { SWORD_PREVIEW, SWORD_LAUNCH };

struct Sword : Projectile {
    SwordState state = SWORD_PREVIEW;
    float vx, vy;
    float angle;
    DWORD spawnTime;
    float speed;
    float curveRate = 0;

    Sword(float startX, float startY, float _vx, float _vy, float _angle, bool isCurve = false);
    std::vector<POINT> getHitPoints();
    void update(Player& p) override;
    void draw() override;
    Rect getRect() override;
    void drawDebug() override;
};

struct Beam : Projectile {
    float speed;
    Beam(float startX, float _speed);
    void update(Player& p) override;
    void draw() override;
};

enum LaserState { LASER_PREPARE, LASER_FIRE, LASER_FADE };

struct Laser : Projectile {
    LaserState state = LASER_PREPARE;
    float cx, cy;
    float angle;
    float currentWidth;
    float length = 6000.0f;
    DWORD stateStartTime;

    Laser(float _cx, float _cy, float _angle);
    void update(Player& p) override;
    void draw() override;
    void drawDebug() override;
    std::vector<POINT> getHitPoints();
};
