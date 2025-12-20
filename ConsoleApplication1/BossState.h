#pragma once
#include <vector>
#include<functional>
#include<iostream>

class Boss;

// 二阶段 Boss 可以瞬移的锚点 (X, Y)
struct BossAnchor {
    float x, y;
};

// 定义了Boss的状态接口
class BossState {
public:
    virtual ~BossState() {};

    virtual void Enter(Boss& boss){} //进入状态调用，做初始化，比如瞬移回中间，重置计时器等
    virtual void Execute(Boss& boss) = 0; //每帧更新时调用，相当于AI逻辑
	virtual void Exit(Boss& boss) {} // 退出状态时调用，做清理工作
};

// --- 一阶段状态 ---
class PhaseOneState : public BossState {
private:
    std::vector<std::function<void(Boss&)>> attacks; // 攻击模式列表
    int lastAttack = -1; //记录上一次的攻击
	DWORD lastAttackTime = 0; //记录上一次攻击的时间
	int attackCycle = 0; //攻击计数循环器，用于每3次强制位移一次

public:
    PhaseOneState(); //构造函数里去注册招式
    //void Enter(Boss& boss) override;
    void Execute(Boss& boss) override;
};

// --- 过渡状态  ---
class TransitionState : public BossState {
public:
    void Enter(Boss& boss) override;
    void Execute(Boss& boss) override;
};

// --- 二阶段状态 ---
class PhaseTwoState : public BossState {
private:
    std::vector<std::function<void(Boss&)>> attacks;
    int lastAttack = -1; //记录上一次的攻击
    DWORD lastAttackTime = 0; //记录上一次攻击的时间
    int attackCycle = 0;
	int attackCount = 2; //每次瞬移后攻击次数，1或2
    int TargetIndex = -1;  // 当前瞬移的目标是第几个锚点

    std::vector<BossAnchor> Anchors = {
    {500 + 100, 200 - 350},
    {120 + 100, 50 - 350},
    {800 + 100, 0 - 350},
    {480 + 100, -300 - 350},
    {0 + 100, -200 - 350},
    {980 + 100, -280 - 350}
    };

public:
    PhaseTwoState();
    void Enter(Boss& boss) override;
    void Execute(Boss& boss) override;
};

// --- 爬梯阶段 ---
class ClimbingState : public BossState {
public:
    void Enter(Boss& boss) override;
    void Execute(Boss& boss) override;
};

// --- 三阶段 --- 
class PhaseThreeState : public BossState {
private:
    DWORD lastAttackTime = 0;
    int attackCounter = 0;
public:
	void Enter(Boss& boss) override;
	void Execute(Boss& boss) override;
};