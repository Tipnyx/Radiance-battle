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
        boss.ChangeState(new TransitionState());
        return;
    }

    if (boss.hp <= 200 && !boss.isPhaseOneLast) {
        boss.isPhaseOneLast = true;

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

    if (boss.isPhaseOneLast) {
        // 锁定位置：防止位移结束后再次触发随机位移
        if (!boss.isTeleporting) boss.x = WINDOW_W / 2.0f;

        // 持续使用垂直剑雨，缩短间隔
        static DWORD lastFinalSwordTime = 0;
        if (currentTime - lastFinalSwordTime > 1000) { // 0.8秒一波，非常密集
			attacks[2](boss); // 使用纵向剑雨
            lastFinalSwordTime = currentTime;
        }
        return;
    }

    if (currentTime - lastAttackTime > 2500) {
        lastAttackTime = currentTime;

        // 简单的随机出招
        lastAttack = -1;
        int attackType;
        attackCycle++;
        if (attackCycle >= 3) { attackType = 6; attackCycle = 0; }
        else { do { attackType = rand() % 6; } while (attackType == lastAttack); }
		lastAttack = attackType;

        attacks[attackType](boss); // 执行之前注册招式表里写的lambda
    }
}

void TransitionState::Enter(Boss& boss) {
	boss.active = false; // Boss 消失
    boss.isPhaseTransition = true;
    boss.hp = 0;

    boss.isPhaseOneLast = false;
    boss.orbAttackActive = false;
    boss.swordAttackActive = false;
    boss.burstAttackActive = false;
    boss.laserAttackActive = false;

    currentSpikeState = SPIKE_HIDDEN;

    GenerateStairs();
    projectiles.clear();
}

void TransitionState::Execute(Boss& boss) {
	if (!boss.active && boss.isPhaseTransition && !boss.isPhaseTwoActive && player.y < -200) {
		boss.ChangeState(new PhaseTwoState());
        return;
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
    TargetIndex = -1;
    
    boss.active = true;
    boss.hp = 500;
    boss.isPhaseTwoActive = true; // 二阶段标记
    boss.x = WINDOW_W / 2;
    boss.y = -600;
    boss.isPhaseTransition = false;
    boss.isTeleporting = false;
    
    boss.trails.clear();
    
}

void PhaseTwoState::Execute(Boss& boss) {

    DWORD currentTime = GetTickCount();

	// 位移或者攻击中不处理其他逻辑
    if (boss.isTeleporting ||
        boss.orbAttackActive ||
        boss.swordAttackActive ||
        boss.burstAttackActive ||
        boss.laserAttackActive) return;

	// 转阶段判定
    if (boss.hp <= 0 && boss.isPhaseTwoActive) {
        boss.ChangeState(new ClimbingState());
        return;
    }

    // 移动逻辑
    if (TargetIndex == -1) {
        // 随机选一个新位置
        int newIndex = rand() % Anchors.size();
        
        // 简单的防重复：如果跟上次一样，就取下一个
        if (newIndex == TargetIndex) {
            newIndex = (newIndex + 1) % Anchors.size();
        }
        TargetIndex = newIndex;

        // 启动瞬移
        boss.isTeleporting = true;

        // 设置目标 (更新 targetX 和 targetY)
        boss.targetX = Anchors[newIndex].x;
        boss.targetY = Anchors[newIndex].y;

        // 决定到了之后打几次 (1次或2次)
        attackCount = 1 + rand() % 2;
        lastAttackTime = currentTime;

        return;
    }

    if (attackCycle <= attackCount) {
        // 攻击间隔大于2.5秒
        if (currentTime - lastAttackTime > 2500) {
            lastAttackTime = currentTime;
            // 随机选招，防连续同一招
            lastAttack = -1;
            int attackType;
            do { attackType = rand() % 5; } while (attackType == lastAttack);
            lastAttack = attackType;

            // 出招
            attacks[attackType](boss);
            attackCycle++;
        }
    }
    else {
        TargetIndex = -1; // 设置为 -1 会在下一帧触发上面的瞬移逻辑
		attackCycle = 0;
    }
}

void ClimbingState::Enter(Boss& boss) {
    
    boss.isPhaseTwoActive = false;

    boss.isTeleporting = true;
    boss.isPhaseClimbing = true;
    boss.climbingLaserActive = true;

    boss.hp = 500;
    boss.targetX = WINDOW_W / 2;
    boss.targetY = -4900;

    boss.lastClimbingLaserTime = GetTickCount();

	GenerateUPStairs(); // 生成向上的台阶

    boss.trails.clear();
    projectiles.clear();

}

void ClimbingState::Execute(Boss& boss) {
    if (player.y < -4400) {
        boss.climbingLaserActive = false;
        boss.ChangeState(new PhaseThreeState());
    }
}

void PhaseThreeState::Enter(Boss& boss) {
	
	boss.isPhaseClimbing = false;

    boss.isPhaseThree = true;
    boss.isTeleporting = true;

    boss.hp = 300;
	boss.targetX = WINDOW_W / 2;
	boss.targetY = -4900;

    attackCounter = 0; // 重置私有计数器

	boss.trails.clear();
	projectiles.clear();
}

void PhaseThreeState::Execute(Boss& boss) {
	
    DWORD currentTime = GetTickCount();

    // 胜利判定
    if (boss.hp <= 0) {
        boss.isDefeated = true;
        boss.active = false;
        boss.orbAttackActive = false;
        projectiles.clear();
        return;
        // 可以在这里触发全局胜利逻辑
    }

    if (currentTime - lastAttackTime > 2000) {
        lastAttackTime = currentTime;
        boss.orbAttackActive = true;
        //boss.orbAttackLastTime = currentTime;
        attackCounter++; // 增加发招计数
    }
    else { boss.orbAttackActive = false; }
    
    if (boss.isTeleporting) return;

    if (attackCounter >= 2) {
        // 四分位点随机：1/4, 2/4, 3/4 屏幕宽度
        boss.targetX = WINDOW_W * (rand() % 3 + 1) / 4.0f;
        boss.isTeleporting = true;

        attackCounter = 0; // 重置计数器，开始下一轮循环
        return;
    }
    
}


    
    
    