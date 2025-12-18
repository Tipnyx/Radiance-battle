#pragma once
# include"common.h"

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

    Boss();
    void update();
    void draw();

    void SpawnSwordWallHorizontal(bool fromLeft);
    void SpawnSwordWallVertical();
    void SpawnOrbs();
    void SpawnBeam();
    void SpawnSwordBurst();
    void BossAI();
};

