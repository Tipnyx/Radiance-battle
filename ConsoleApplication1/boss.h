#pragma once
# include"common.h"

struct TrailPoint {
    float x, y;
    float alpha;
};

class BossState;

class Boss {

private:
	BossState* currentState = nullptr; // 当前状态指针
    
    const int ORB_ATTACK_TOTAL = 3;    // 一次攻击发射3个
    const int ORB_ATTACK_INTERVAL = 1500; // 每1.5秒发射一个

    const int SWORD_ATTACK_TOTAL = 3; //一次三波
    const int SWORD_ATTACK_INTERVAL_VERTICAL = 1000; //每1.5秒发射一波
    const int SWORD_ATTACK_INTERVAL_HORIZON = 1250; //每1.5秒发射一波

    const int TOTAL_BURST_WAVES = 2; // 总共两波

public:
    float x, y;
    int hp = 1000;
    bool active = true;
    float alpha = 0; // 用于进场时的渐显效果

	// 状态切换变量
    bool isPhaseOneLast = false; // 是否进入剑雨阶段
	bool isPhaseTransition = false; // 是否正在进行阶段转换
    bool isPhaseTwoActive = false; // 二阶段是否激活
	bool isPhaseClimbing = false; // 是否进入攀爬阶段
    bool isPhaseThree = false; // 三阶段

    // 光球多波攻击状态
    bool orbAttackActive = false;      // 是否正在进行光球攻击
    int orbAttackCount = 0;            // 已发射的光球数
    DWORD orbAttackLastTime = 0;       // 上一个光球发射时间

    // 剑雨多波攻击状态
    bool swordAttackActive = false; //是否正在进行剑雨
    int swordAttackCount = 0; // 已发射波数
    DWORD swordAttackLastTime = 0;// 上一次发射时间
    
    int swordAttackType = 0; //1：横向；2：纵向
    bool swordAttackFromLeft = true; // true: 从左, false: 从右

	// 脸刺多波攻击状态
	bool burstAttackActive = false;  // 是否正在进行脸刺攻击
    int burstWaveCount = 0;         // 已生成的波数
    DWORD lastBurstTime = 0;        // 上一波生成的起始时间

    // 三连激光状态变量
	bool laserAttackActive = false; // 是否正在进行三连激光攻击
	int laserWaveCount = 0; // 已发射波数
	DWORD lastLaserTime = 0; // 上一次发射时间

    // 攀爬阶段的追踪激光变量
    bool climbingLaserActive = false;
    DWORD lastClimbingLaserTime = 0;  // 上次发射时间

    // 拖尾变量
    float targetX;             // 目标位置
    float targetY;
    bool isTeleporting = false; // 是否正在位移
    std::vector<TrailPoint> trails; // 存储拖尾坐标
    const int MAX_TRAILS = 10;      // 拖尾长度

    // 受击相关
	bool boss_is_invincible = false; //无敌状态
	DWORD boss_invincible_start_time = 0; // 记录无敌开始时间
	const int BOSS_INVINCIBLE_DURATION = 300; // BOSS受击无敌时间300ms

    // 图片缓存
    IMAGE sunCache; // 增加一个图片缓存对象
    void InitSunCache(); // 用于初始化绘制这张图

    bool isDefeated = false; //boss是否被击败

    IMAGE hitCache;
	void InitHitCache();

    Boss();
	
	// --- 核心更新与绘制函数 ---
    
    void update(); 
    void draw();

	// --- 攻击生成函数 ---

    void SpawnSwordWallHorizontal(bool fromLeft);
    void SpawnSwordWallVertical();
    void SpawnOrbs();
    void SpawnBeam();
    void SpawnSwordBurst();
    void SpawnLaserBurst();

    // --- 碰撞箱 ---

	virtual Rect getRect(); // 获取BOSS碰撞盒
	void drawDebug(); // 调试模式下绘制碰撞盒
	
    void reset(); // 重置BOSS状态，用于重新开始游戏等场景

    // --- Boss状态(联合BossState进行使用) ---

    void ChangeState(BossState* newState); // 切换状态
    void UpdateStateMachine();             // 更新状态机
	void UpdateAttacks();		  // 根据状态执行攻击逻辑
};






