#pragma once
#include"common.h"

class Player;

struct Projectile {
    float x = 0, y = 0;
    float w = 0, h = 0;
    int type = 0; // 0:Orb, 1:Sword, 2:Beam
    bool active = true;

    virtual void update(Player& p) = 0;
    virtual void draw() = 0;
    virtual Rect getRect();

    virtual void drawDebug();
};

enum OrbState { SPAWNING, CHARGING, RECOVERING };

struct Orb : Projectile {
	float vx, vy;
	float currentSpeed;
	float orbScale = 0.2f; // 初始大小
	// --- 调节参数 ---
	const float MAX_SPEED = 14.0f;     // 冲刺速度（辐光的球很快）
	const float TURN_FORCE = 0.12f;    // 转向力度：越小弧线越大，越大越粘人
	const int   MAX_ATTACKS = 2;       // 攻击轮次
	OrbState state = SPAWNING;
	DWORD stateStartTime;
	DWORD spawnTime;
	int attackCount = 0;
	Orb(float sx, float sy, float px, float py);
	void update(Player& p) override;
	void draw() override;
	Rect getRect() override;
};

enum SwordState { SWORD_PREVIEW, SWORD_LAUNCH };

struct Sword :  Projectile {
	float vx, vy;
	float angle;
	SwordState state = SWORD_PREVIEW;
	DWORD spawnTime;
	float speed;
	// --- 新增弧线参数 ---
	float curveRate = 0; // 每一帧旋转的弧度
	Sword(float startX, float startY, float _vx, float _vy, float _angle, bool isCurve=false);
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