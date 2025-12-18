#pragma once
#include "common.h"

// 玩家类
class Player {
public:
    float x, y;
    float vx, vy;
    int w = 30, h = 50;
    bool onGround = false;
    int facing = 1; // 1 Right, -1 Left

    // 无敌帧
    int hurtTimer = 0;            // 受伤无敌倒计时
    const int HURT_DURATION = 90; // 受伤后无敌 1秒 (约60帧)

    // 二段跳
    int jumpCount = 0; //当前跳跃次数
    const int MAX_JUMP = 2; // 最大跳跃次数（二段跳）
    bool lastJumpKey = false; //上一帧跳跃键状态

    // 冲刺相关
    bool isDashing = false;
    bool isShadowDash = false; // 区分当前是否为暗影冲刺
    bool canShadowDash = true; // 暗影冲刺就绪状态
    bool canDash = true;
    DWORD dashStartTime = 0;
    DWORD lastShadowDashTime = 0;

    const int DASH_DURATION = 300; // ms
    const int SHADOW_DASH_COOLDOWN = 1500; // 暗影冲刺冷却 (1.5s)
    const int NORMAL_DASH_COOLDOWN = 400;  // 普通冲刺冷却 (极短)
    DWORD lastNormalDashTime = 0;

    // 攻击相关
    bool isAttacking = false;
    DWORD attackStartTime = 0;
    int attackDir = 0; // 0:Side, 1:Up, 2:Down
    Rect attackBox;
    int atkTimer = 0;         // 当前攻击计时器（帧数）
    const int atkDuration = 10; // 攻击持续的总帧数 (约0.16秒)


    // 状态
    int hp = 10;
    bool isInvincible = false; // 无敌帧

    // --- 暗影冲刺残影相关 ---
    struct ShadowGhost {
        float x, y;
        int life; // 寿命，从 255 减到 0
    };
    std::vector<ShadowGhost> ghosts;
    DWORD lastGhostTime = 0; // 控制残影生成频率

    // 函数声明（只写名字，不写大括号里的逻辑）
    Player();
    
    void reset();
    void update();
    void draw();
    Rect getHitbox();
};

extern Player player;