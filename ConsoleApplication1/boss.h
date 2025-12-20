#pragma once
# include"common.h"

struct TrailPoint {
    float x, y;
    float alpha;
};

// 二阶段 Boss 可以瞬移的锚点 (X, Y)
struct BossAnchor {
    float x, y;
};



class BossState;

class Boss {
	BossState* currentState = nullptr; // 当前状态指针

public:
    float x, y;
    int hp = 10;
    bool active = true;
    float alpha = 0; // 用于进场时的渐显效果

    DWORD phaseTwoWaitTimer = 0;

	// 状态切换变量
    bool isPhaseOneLast = false; // 是否进入剑雨阶段
	bool isPhaseTransition = false; // 是否正在进行阶段转换
    bool isPhaseTwoActive = false; // 二阶段是否激活
	bool isPhaseClimbing = false; // 是否进入攀爬阶段
    bool isPhaseThree = false; // 三阶段

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
    float targetY;
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
	const int BOSS_INVINCIBLE_DURATION = 300; // BOSS受击无敌时间200ms

    // 图片缓存
    IMAGE sunCache; // 增加一个图片缓存对象
    void InitSunCache(); // 用于初始化绘制这张图

    // 二阶段战斗逻辑变量
    
    int phaseTwoTargetIndex = -1;  // 当前瞬移的目标是第几个锚点
    int phaseTwoAttackCount = 0;   // 在当前位置还需要攻击几次
    DWORD teleportStartTime = 0;   // 记录瞬移开始的时间

    bool isDefeated = false; //boss是否被击败

    bool climbingLaserActive = false; // 是否开启攀爬阶段的追踪激光
    DWORD lastClimbingLaserTime = 0;  // 上次发射时间

    std::vector<BossAnchor> phaseTwoAnchors = {
    {500 + 100, 200 - 350},
    {120 + 100, 50 - 350},
    {800 + 100, 0 - 350},
    {480 + 100, -300 - 350},
    {0 + 100, -200 - 350},
    {980 + 100, -280 - 350}
    };

    IMAGE hitCache;
	void InitHitCache();

    Boss();
	void update(); // 这里面要写if(currentState) currentState->update(*this)
    // 这里的update是BossState的update，传入Boss的引用，调用对应的状态
    
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

    void ChangeState(BossState* newState); // 切换状态
    void UpdateStateMachine();             // 更新状态机
    void UpdateAttacks();
};






