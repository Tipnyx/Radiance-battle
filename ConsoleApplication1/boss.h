#pragma once
# include"common.h"

struct TrailPoint {
    float x, y;
    float alpha;
};

class BossState;

class Boss {

private:
    BossState* currentState = nullptr;

    const int ORB_ATTACK_TOTAL = 3;
    const int ORB_ATTACK_INTERVAL = 1500;

    const int SWORD_ATTACK_TOTAL = 3;
    const int SWORD_ATTACK_INTERVAL_VERTICAL = 1000;
    const int SWORD_ATTACK_INTERVAL_HORIZON = 1250;

    const int TOTAL_BURST_WAVES = 2;

public:
    float x, y;
    int hp = 200;
    int maxHp = 1000;
    bool active = true;
    float alpha = 0;

    bool isPhaseOneLast = false;
    bool isPhaseTransition = false;
    bool isPhaseTwoActive = false;
    bool isPhaseClimbing = false;
    bool isPhaseThree = false;

    bool orbAttackActive = false;
    int orbAttackCount = 0;
    DWORD orbAttackLastTime = 0;

    bool swordAttackActive = false;
    int swordAttackCount = 0;
    DWORD swordAttackLastTime = 0;

    int swordAttackType = 0;
    bool swordAttackFromLeft = true;

    bool burstAttackActive = false;
    int burstWaveCount = 0;
    DWORD lastBurstTime = 0;

    bool laserAttackActive = false;
    int laserWaveCount = 0;
    DWORD lastLaserTime = 0;

    bool climbingLaserActive = false;
    DWORD lastClimbingLaserTime = 0;

    float targetX;
    float targetY;
    bool isTeleporting = false;
    std::vector<TrailPoint> trails;
    const int MAX_TRAILS = 10;

    bool boss_is_invincible = false;
    DWORD boss_invincible_start_time = 0;
    const int BOSS_INVINCIBLE_DURATION = 300;

    Texture sunTex;
    Texture hitTex;
    void InitSunCache();
    void InitHitCache();

    bool isDefeated = false;

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

    void ChangeState(BossState* newState);
    void UpdateStateMachine();
    void UpdateAttacks();
};
