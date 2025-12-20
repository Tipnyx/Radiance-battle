# include "boss.h"
#include "BossState.h"
#include"player.h"
#include"world.h"
#include"projectile.h"

extern std::vector<Projectile*> projectiles;

PhaseOneState::PhaseOneState() {
    // 注册攻击模式
    // 0: 光球
    attacks.push_back([](Boss& b) {
        b.orbAttackActive = true; b.orbAttackCount = 0; b.orbAttackLastTime = GetTickCount();
        });
    // 1: 横剑
    attacks.push_back([](Boss& b) {
        b.swordAttackActive = true; b.swordAttackType = 1; b.swordAttackCount = 0; b.swordAttackFromLeft = rand() % 2;
        });
    // 2: 纵剑
    attacks.push_back([](Boss& b) {
        b.swordAttackActive = true; b.swordAttackType = 2; b.swordAttackCount = 0;
        });
    // 3: 光柱
    attacks.push_back([](Boss& b) {
        b.SpawnBeam();
        });
    // 4: 脸刺
    attacks.push_back([](Boss& b) {
        b.burstAttackActive = true; b.burstWaveCount = 0; b.lastBurstTime = GetTickCount();
        });
    // 5: 三连激光
    attacks.push_back([](Boss& b) {
        b.laserAttackActive = true;; b.laserWaveCount = 0; b.lastLaserTime = GetTickCount() - 1000;
        });
    // 6: 瞬移
    attacks.push_back([](Boss& b) {
        b.targetX = WINDOW_W * (rand() % 3 + 1) / 4; b.isTeleporting = true;
        });
}



void PhaseOneState::Execute(Boss& boss) {

    DWORD currentTime = GetTickCount();

    if (boss.alpha < 1.0f) {
        lastAttackTime = GetTickCount();
        return;
    }

    if (boss.orbAttackActive ||
        boss.swordAttackActive ||
        boss.burstAttackActive ||
        boss.laserAttackActive) return;

    if (boss.hp <= 0 && boss.active) {
        boss.ChangeState(new ClimbingState());
        return;
    }

    if (boss.hp <= 200 && !boss.PhaseOneLast) {
        boss.PhaseOneLast = true;

        // 1. 清空所有正在进行的旧攻击状态
        boss.orbAttackActive = false;
        boss.swordAttackActive = false;
        boss.burstAttackActive = false;
        boss.laserAttackActive = false;

        // 2. 强行回归中心点
        boss.targetX = WINDOW_W / 2.0f;
        boss.isTeleporting = true;

        // 3. 强行修改地刺逻辑 (我们可以直接修改全局变量)
        currentSpikeState = SPIKE_ACTIVE;
        // 这里我们可以稍微改一下 SpikeManager，让它支持“全屏两边刺”
        return;
    }

    if (boss.PhaseOneLast) {
        // 锁定位置：防止位移结束后再次触发随机位移
        if (!boss.isTeleporting) boss.x = WINDOW_W / 2.0f;

        // 持续使用垂直剑雨，缩短间隔
        static DWORD lastFinalSwordTime = 0;
        if (currentTime - lastFinalSwordTime > 900) { // 0.8秒一波，非常密集
			attacks[2](boss); // 使用纵向剑雨
            lastFinalSwordTime = currentTime;
        }
        return;
    }

    if (currentTime - lastAttackTime > 1500) {
        lastAttackTime = currentTime;

        // 简单的随机出招
        static int lastAttack = -1;
        int attackType;
        attackCycle++;
        if (attackCycle >= 3) { attackType = 6; attackCycle = 0; }
        else { do { attackType = rand() % 6; } while (attackType == lastAttack); }

        attacks[attackType](boss); // 执行之前注册招式表里写的lambda
    }
}

void ClimbingState::Enter(Boss& boss) {
	boss.active = false; // Boss 消失
    boss.PhaseClimb = true;
    boss.hp = 0;

    boss.PhaseOneLast = false;
    boss.orbAttackActive = false;
    boss.swordAttackActive = false;
    boss.burstAttackActive = false;
    boss.laserAttackActive = false;

    currentSpikeState = SPIKE_HIDDEN;

    GenerateStairs();
    projectiles.clear();
}

void ClimbingState::Execute(Boss& boss) {
	if (!boss.active && boss.PhaseClimb && !boss.isPhaseTwoActive && player.y < -200) {
		boss.ChangeState(new PhaseTwoState());
	}
}

PhaseTwoState::PhaseTwoState(){
	// 注册攻击模式
	// 0: 光球
	attacks.push_back([](Boss& b) {
		b.orbAttackActive = true; b.orbAttackCount = 0; b.orbAttackLastTime = GetTickCount();
		});
	// 1: 横剑
	attacks.push_back([](Boss& b) {
		b.swordAttackActive = true; b.swordAttackType = 1; b.swordAttackCount = 0; b.swordAttackLastTime = GetTickCount(); b.swordAttackFromLeft = rand() % 2;
		});
    // 2: 光柱
    attacks.push_back([](Boss& b) {
        b.SpawnBeam();
        });
	// 3: 脸刺
	attacks.push_back([](Boss& b) {
		b.burstAttackActive = true; b.burstWaveCount = 0; b.lastBurstTime = GetTickCount();
		});
	// 4: 三连激光
	attacks.push_back([](Boss& b) {
		b.laserAttackActive = true;; b.laserWaveCount = 0; b.lastLaserTime = GetTickCount() - 1000;
		});
}

void PhaseTwoState::Enter(Boss& boss) {
    boss.active = true;
    boss.hp = 500;
    boss.isPhaseTwoActive = true; // 二阶段标记
    boss.x = WINDOW_W / 2;
    boss.y = -600;
    boss.PhaseClimb = false;
    boss.isTeleporting = false;
    boss.phaseTwoTargetIndex = -1;
    boss.trails.clear();

}

void PhaseTwoState::Execute(Boss& boss) {

    DWORD currentTime = GetTickCount();

    if (boss.isTeleporting ||
        boss.orbAttackActive ||
        boss.swordAttackActive ||
        boss.burstAttackActive ||
        boss.laserAttackActive) return;

    if (boss.hp <= 0 && boss.isPhaseTwoActive) {
        boss.isDefeated = true;
		boss.active = false;
		return;
    }

    // 移动逻辑
    if (boss.phaseTwoTargetIndex == -1) {
        // 随机选一个新位置
        int newIndex = rand() % boss.phaseTwoAnchors.size();
        // 简单的防重复：如果跟上次一样，就取下一个
        if (newIndex == boss.phaseTwoTargetIndex) {
            newIndex = (newIndex + 1) % boss.phaseTwoAnchors.size();
        }
        boss.phaseTwoTargetIndex = newIndex;

        // 启动瞬移
        boss.isTeleporting = true;
        boss.teleportStartTime = currentTime;

        // 设置目标 (更新 targetX 和 targetY)
        boss.targetX = boss.phaseTwoAnchors[newIndex].x;
        boss.targetY = boss.phaseTwoAnchors[newIndex].y;

        // 决定到了之后打几次 (1次或2次)
        boss.phaseTwoAttackCount = 1 + rand() % 2;
        return;
    }

    if (boss.phaseTwoAttackCount > 0) {
        // 瞬移到位后，发呆 1 秒再攻击，给玩家反应时间
        static DWORD waitTimer = 0;
        if (currentTime - waitTimer < 1000) return; // 等待中...

        // 简单的随机出招
        int attackType = rand() % 5;

        attacks[attackType](boss);

        boss.phaseTwoAttackCount--; // 消耗一次攻击次数
        waitTimer = currentTime;       // 重置等待计时
    }
    else {
        // 3. 次数用尽 -> 准备下一次瞬移
        boss.phaseTwoTargetIndex = -1; // 设置为 -1 会在下一帧触发上面的瞬移逻辑
    }
}

    
    
    