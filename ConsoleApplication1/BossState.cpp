# include "boss.h"
#include "BossState.h"
#include"player.h"
#include"world.h"
#include"projectile.h"

extern std::vector<Projectile*> projectiles;

PhaseOneState::PhaseOneState() {

    attacks.push_back([](Boss& b) {
        b.orbAttackActive = true; b.orbAttackCount = 0; b.orbAttackLastTime = GetTickCount();
        });
    attacks.push_back([](Boss& b) {
        b.swordAttackActive = true; b.swordAttackType = 1; b.swordAttackCount = 0; b.swordAttackFromLeft = rand() % 2;
        });
    attacks.push_back([](Boss& b) {
        b.swordAttackActive = true; b.swordAttackType = 2; b.swordAttackCount = 0;
        });
    attacks.push_back([](Boss& b) {
        b.SpawnBeam();
        });
    attacks.push_back([](Boss& b) {
        b.burstAttackActive = true; b.burstWaveCount = 0; b.lastBurstTime = GetTickCount();
        });
    attacks.push_back([](Boss& b) {
        b.laserAttackActive = true;; b.laserWaveCount = 0; b.lastLaserTime = GetTickCount() - 1000;
        });
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

  
        boss.orbAttackActive = false;
        boss.swordAttackActive = false;
        boss.burstAttackActive = false;
        boss.laserAttackActive = false;


        boss.targetX = WINDOW_W / 2.0f;
        boss.isTeleporting = true;


        currentSpikeState = SPIKE_ACTIVE;
   
        return;
    }

    if (boss.isPhaseOneLast) {
   
        if (!boss.isTeleporting) boss.x = WINDOW_W / 2.0f;


        static DWORD lastFinalSwordTime = 0;
        if (currentTime - lastFinalSwordTime > 1000) { 
			attacks[2](boss); 
            lastFinalSwordTime = currentTime;
        }
        return;
    }

    if (currentTime - lastAttackTime > 2500) {
        lastAttackTime = currentTime;

        lastAttack = -1;
        int attackType;
        attackCycle++;
        if (attackCycle >= 3) { attackType = 6; attackCycle = 0; }
        else { do { attackType = rand() % 6; } while (attackType == lastAttack); }
		lastAttack = attackType;

        attacks[attackType](boss);
    }
}

void TransitionState::Enter(Boss& boss) {
    boss.active = false; 
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

	attacks.push_back([](Boss& b) {
		b.orbAttackActive = true; b.orbAttackCount = 0; b.orbAttackLastTime = GetTickCount();
		});

	attacks.push_back([](Boss& b) {
		b.swordAttackActive = true; b.swordAttackType = 1; b.swordAttackCount = 0; b.swordAttackLastTime = GetTickCount(); b.swordAttackFromLeft = rand() % 2;
		});

    attacks.push_back([](Boss& b) {
        b.SpawnBeam();
        });

	attacks.push_back([](Boss& b) {
		b.burstAttackActive = true; b.burstWaveCount = 0; b.lastBurstTime = GetTickCount();
		});

	attacks.push_back([](Boss& b) {
		b.laserAttackActive = true;; b.laserWaveCount = 0; b.lastLaserTime = GetTickCount() - 1000;
		});
}

void PhaseTwoState::Enter(Boss& boss) {
    boss.maxHp = boss.maxHp;
    TargetIndex = -1;
    
    boss.active = true;
    boss.hp = 500;
    boss.isPhaseTwoActive = true; 
    boss.x = WINDOW_W / 2;
    boss.y = -600;
    boss.isPhaseTransition = false;
    boss.isTeleporting = false;
    
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
        boss.ChangeState(new ClimbingState());
        return;
    }


    if (TargetIndex == -1) {

        int newIndex = rand() % Anchors.size();
        

        if (newIndex == TargetIndex) {
            newIndex = (newIndex + 1) % Anchors.size();
        }
        TargetIndex = newIndex;


        boss.isTeleporting = true;

        boss.targetX = Anchors[newIndex].x;
        boss.targetY = Anchors[newIndex].y;

        attackCount = 1 + rand() % 2;
        lastAttackTime = currentTime;

        return;
    }

    if (attackCycle <= attackCount) {

        if (currentTime - lastAttackTime > 2500) {
            lastAttackTime = currentTime;
     
            lastAttack = -1;
            int attackType;
            do { attackType = rand() % 5; } while (attackType == lastAttack);
            lastAttack = attackType;


            attacks[attackType](boss);
            attackCycle++;
        }
    }
    else {
        TargetIndex = -1;
		attackCycle = 0;
    }
}

void ClimbingState::Enter(Boss& boss) {
    
    boss.isPhaseTwoActive = false;

    boss.isTeleporting = true;
    boss.maxHp = boss.hp;
    boss.isPhaseClimbing = true;
    boss.climbingLaserActive = true;

    boss.hp = 500;
    boss.targetX = WINDOW_W / 2;
    boss.targetY = -4900;

    boss.lastClimbingLaserTime = GetTickCount();

	GenerateUPStairs(); 

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

    boss.maxHp = boss.hp;
    boss.isPhaseThree = true;
    boss.isTeleporting = true;

    boss.hp = 300;
	boss.targetX = WINDOW_W / 2;
	boss.targetY = -4900;

    attackCounter = 0; 

	boss.trails.clear();
	projectiles.clear();
}

void PhaseThreeState::Execute(Boss& boss) {
	
    DWORD currentTime = GetTickCount();


    if (boss.hp <= 0) {
        boss.isDefeated = true;
        boss.active = false;
        boss.orbAttackActive = false;
        projectiles.clear();
        return;

    }

    if (currentTime - lastAttackTime > 2000) {
        lastAttackTime = currentTime;
        boss.orbAttackActive = true;
        //boss.orbAttackLastTime = currentTime;
        attackCounter++; 
    }
    else { boss.orbAttackActive = false; }
    
    if (boss.isTeleporting) return;

    if (attackCounter >= 2) {

        boss.targetX = WINDOW_W * (rand() % 3 + 1) / 4.0f;
        boss.isTeleporting = true;

        attackCounter = 0; 
        return;
    }
    
}


    
    
    