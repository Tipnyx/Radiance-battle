#pragma once
# include"common.h"

struct TrailPoint {
    float x, y;
    float alpha;
};

class Boss {
public:
    float x, y;
    int hp = 1000;
    bool active = true;
    float alpha = 0; // 用于进场时的渐显效果

    DWORD lastAttackTime = 0;
    DWORD lastOrbTime = 0;
    int orbCount = 0;
    bool orbAttackActive = false;      // 是否正在进行光球攻击
    int orbAttackCount = 0;            // 已发射的光球数
    DWORD orbAttackLastTime = 0;       // 上一个光球发射时间

    //剑雨多波攻击状态
    bool swordAttackActive = false; //是否正在进行剑雨
    int swordAttackType = 0; //1：横向；2：纵向
    int swordAttackCount = 0; // 已发射波数
    DWORD swordAttackLastTime = 0;// 上一次发射时间

    bool swordAttackFromLeft = true; // true: 从左, false: 从右

    bool burstAttackActive = false;
    int burstWaveCount = 0;         // 已生成的波数
    DWORD lastBurstTime = 0;        // 上一波生成的起始时间

    int attackCycle = 0; // 新增：记录当前是第几次攻击

    // 拖尾变量
    float targetX;             // 目标位置
    bool isTeleporting = false; // 是否正在位移
    std::vector<TrailPoint> trails; // 存储拖尾坐标
    const int MAX_TRAILS = 10;      // 拖尾长度

    // 三连激光状态变量
    bool laserAttackActive = false;
    int laserWaveCount = 0;
    DWORD lastLaserTime = 0;

    // 受击相关
	bool boss_is_invincible = false;
	DWORD boss_invincible_start_time = 0;
	const int BOSS_INVINCIBLE_DURATION = 450; // BOSS受击无敌时间450ms

    Boss();
    void update();
    void draw();

    void SpawnSwordWallHorizontal(bool fromLeft);
    void SpawnSwordWallVertical();
    void SpawnOrbs();
    void SpawnBeam();
    void SpawnSwordBurst();
    void SpawnLaserBurst(); 

    virtual Rect getRect();
	void drawDebug();
    void reset();

    void BossAI();
};

