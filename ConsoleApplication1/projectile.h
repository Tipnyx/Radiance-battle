#pragma once
#include"common.h"

class Player;

struct Projectile {
    float x = 0, y = 0;
    float w = 0, h = 0;
    int type = 0; // 0:Orb, 1:Sword, 2:Beam 3:Laser
    bool active = true; // 默认激活

    virtual void update(Player& p) = 0;
    virtual void draw() = 0;
    virtual Rect getRect();

    virtual void drawDebug();
};

// 对于每一种弹幕，我们都设计了状态机来控制其行为

enum OrbState { SPAWNING, CHARGING, RECOVERING };

struct Orb : Projectile {
	
	OrbState state = SPAWNING; // 初始状态为生成中

	float vx, vy; // 水平和竖直的速度分量
	float currentSpeed; // 当前速度
	float orbScale = 0.2f; // 初始大小

	const float MAX_SPEED = 14.0f;     // 光球冲刺速度
	const float TURN_FORCE = 0.12f;    // 转向力度：越小弧线越大，越大越粘人
	const int   MAX_ATTACKS = 2;       // 光球拐弯后，继续进行下一次攻击的循环轮数
	
	DWORD stateStartTime; // 状态开始时间
	DWORD spawnTime; // 生成时间
	int attackCount = 0; // 已完成的攻击次数，这个是和MAX_ATTACKS结合使用的

	static IMAGE imgOrb;

	Orb(float sx, float sy, float px, float py); // 传入生成位置和玩家位置，传入玩家位置主要是为了追踪？
	void update(Player& p) override; 
	void draw() override;
	Rect getRect() override;

};


enum SwordState { SWORD_PREVIEW, SWORD_LAUNCH }; // 预览、发射

struct Sword :  Projectile {
	
	SwordState state = SWORD_PREVIEW; // 初始为预览状态，此时没有伤害判定，颜色也较浅

	float vx, vy;
	float angle; // 光剑的角度
	DWORD spawnTime; // 生成时间
	float speed; // 预设速度
	
	// --- 弧线参数 ---
	float curveRate = 0; // 每一帧旋转的弧度

	static IMAGE imgSwordH_Mask; // 横向掩码
	static IMAGE imgSwordH_Src;  // 横向前景
	static IMAGE imgSwordV_Mask; // 纵向掩码
	static IMAGE imgSwordV_Src;  // 纵向前景

	Sword(float startX, float startY, float _vx, float _vy, float _angle, bool isCurve=false);
	std::vector<POINT> getHitPoints();
	void update(Player& p) override;
	void draw() override;
	Rect getRect() override;
	void drawDebug() override;

	static void InitSwordSprites(); 

};

// 这个是光柱，只有左右移动，没有多余的状态，所以这里没有enum

struct Beam : Projectile {
	float speed;
	
	Beam(float startX, float _speed);
	void update(Player& p) override;
	void draw() override;
};



enum LaserState { LASER_PREPARE, LASER_FIRE, LASER_FADE }; // 预警、发射、消失

struct Laser : Projectile {

	LaserState state = LASER_PREPARE; // 先准备

	float cx, cy;       // 激光源头（Boss中心）
	float angle;        // 发射角度
	float currentWidth; // 当前宽度（用于动画）
	float length = 1500.0f; // 激光长度，足够穿透屏幕

	DWORD stateStartTime; // 状态开始时间

	// 构造函数
	Laser(float _cx, float _cy, float _angle); // 需要传入位置和角度

	void update(Player& p) override;
	void draw() override;
	void drawDebug() override;

	// 激光特有的碰撞点获取（因为是斜着的长条，不能只用 getRect）
	std::vector<POINT> getHitPoints(); 

};