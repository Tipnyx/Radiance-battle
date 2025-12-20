#include"GameManager.h"
#include"player.h"
#include"boss.h"
#include"projectile.h"
#include"BossState.h"

extern Player player;
extern Boss boss;
extern std::vector<Projectile*> projectiles;

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#include <cstdlib>

/* 重置游戏:
玩家重置
删除所有弹幕
重置计时器
重置地刺状态
*/
void GameReset(DWORD& gameStartTime) {
    
    // 死亡重开
	//printf("玩家在 %lu ms 死亡，重置游戏状态\n", GetTickCount() - gameStartTime);
    player.reset();
    boss.reset();
    for (auto p : projectiles) delete p;
    projectiles.clear();
    gameStartTime = GetTickCount();
    boss.isPhaseOneLast = false;

    // 2. 重置音乐
    // 停止当前播放
    mciSendString(L"stop bgm", NULL, 0, NULL);
    // 将进度条拖回 0 (开头)
    mciSendString(L"seek bgm to start", NULL, 0, NULL);
    // 重新开始循环播放
    mciSendString(L"play bgm repeat", NULL, 0, NULL);

	//printf("游戏重置完成，目前游戏时间点处于：%lu ms\n", gameStartTime);
    // 重置地刺
    currentSpikeState = SPIKE_HIDDEN;
    spikeTimer = 0;

    boss.active = true;
    currentLevelBottom = WINDOW_H;
    boss.isPhaseTwoActive = false;
    boss.isPhaseTransition = false;
    boss.isPhaseClimbing = false;

    platforms.clear();
    platforms.push_back({ (float)PLATFORM_X, (float)PLATFORM_Y, (float)PLATFORM_W, 500 });

    boss.isDefeated = false;
	boss.ChangeState(new PhaseOneState());
}

/* 此函数实现玩家攻击BOSS的判定,及攻击到后的行为*/
void AttackBoss() {
    int hittime = GetTickCount(); //获取当前时间点

    if (hittime - boss.boss_invincible_start_time >= boss.BOSS_INVINCIBLE_DURATION) { // 如果距离上次无敌时间点已经过了0.45秒
        boss.boss_is_invincible = false; // 将boss设置为非无敌
    }

    if (player.isAttacking) {
        Rect bRect = boss.getRect(); // BOSS 的碰撞盒

        if (player.attackBox.checkCollision(bRect)) { //打中boss了

            if (!boss.boss_is_invincible) { // 如果此时boss不无敌
                boss.hp -= 20; // 掉血 
                //printf("bossHP:%d", boss.hp);
				boss.boss_is_invincible = true; //将boss设置为无敌
                boss.boss_invincible_start_time = GetTickCount(); // 获取打中boss后，且boss不无敌的时间点
            }

            // 下劈判定
			if (player.attackDir == 2) {
				player.vy = -7.0f; // 反弹效果
				player.jumpCount = 1;  // 刷新二段跳
                player.hasDashedInAir = false;
			}

            // 以后可以在这里加“回魂”逻辑
            
            // 攻击瞬间的顿帧效果
            if (player.atkTimer == player.atkDuration - 1) { // 只在第一帧停顿
                 Sleep(50); 
            }
        }
    }
}

/* 此函数定义不同弹幕的碰撞箱(光剑采用多点判定,光球/光柱是矩形判定)
及玩家下劈到弹幕后,被弹幕碰到后的行为*/
void ProjectileManager() {
    // 检查弹幕列表里的每一个弹幕
    for (size_t i = 0; i < projectiles.size(); ) {
		projectiles[i]->update(player); // 更新弹幕位置和状态

        // 碰撞检测
		if (projectiles[i]->active) { //如果弹幕还活着
			Rect pRect = player.getHitbox(); //获取玩家碰撞盒
            bool playerHit = false; // 是否撞到玩家
            bool pogoHit = false;   // 是否被玩家下劈

            // --- 根据弹幕类型执行不同的判定算法 ---
            if (projectiles[i]->type == 1) { // 如果是长剑,我是不去理会碰撞箱的!
				Sword* sw = (Sword*)projectiles[i]; // 转换为剑指针
				if (sw->state == SWORD_LAUNCH) { // 只有发射状态才有伤害判定
					auto hitPts = sw->getHitPoints(); // 获取剑的多个判定点
                    
                    // 判定点“进入”玩家体内，则受伤（啊，不要进来~）
                    for (auto& p : hitPts) {
                        if (p.x >= pRect.x && p.x <= pRect.x + pRect.w &&
                            p.y >= pRect.y && p.y <= pRect.y + pRect.h) {
                            playerHit = true; break;
                        }
                    }

                    // 玩家在攻击，玩家在下劈！
                    if (player.isAttacking && player.attackDir == 2) {
                        // 判定点进入了攻击箱，玩家被反弹
                        for (auto& p : hitPts) {
                            if (p.x >= player.attackBox.x && p.x <= player.attackBox.x + player.attackBox.w &&
                                p.y >= player.attackBox.y && p.y <= player.attackBox.y + player.attackBox.h) {
                                pogoHit = true; break;
                            }
                        }
                    }
                }
            }
            else if (projectiles[i]->type == 3) { // Laser
                // 强转为 Laser 指针
                Laser* laser = (Laser*)projectiles[i];
                // 获取采样点
                std::vector<POINT> pts = laser->getHitPoints();

                // 遍历所有点进行圆形碰撞检测
                for (auto p : pts) {
                    if (p.x >= pRect.x && p.x <= pRect.x + pRect.w &&
                        p.y >= pRect.y && p.y <= pRect.y + pRect.h) {
                        playerHit = true; break;
                    }
                }
            }

            else { // 光球或光束：标准矩形判定
                Rect bRect = projectiles[i]->getRect();
                if (pRect.checkCollision(bRect)) playerHit = true;
                if (player.isAttacking && player.attackDir == 2 && player.attackBox.checkCollision(bRect)) {
                    pogoHit = true;
                }
            }

            // --- B. 统一处理碰撞后果 ---
            // 1. 处理下劈弹起
            if (pogoHit) {
                player.vy = -7.0f;
                player.jumpCount = 1;  // 刷新二段跳
                player.hasDashedInAir = false;
                //这里可以加个音效提示 Pogo 成功
            }

            // 2. 处理玩家受伤或冲刺穿透
            if (playerHit) {
                if (!player.isInvincible) { //玩家此时没无敌帧的话
                    // 真正受伤
                    player.hp--;
                    player.hurtTimer = player.HURT_DURATION;
                    player.isInvincible = true;

                    // 只有球体会撞人销毁，剑和激光是穿透的
                    if (projectiles[i]->type == 0) projectiles[i]->active = false;
                }
                else {
                    // 正在冲刺/无敌：实现“穿透”效果
                    // 如果是光球，冲刺撞到也会碎掉（符合空洞骑士手感）
                    if (projectiles[i]->type == 0) projectiles[i]->active = false;
                }
            }
        }

        // 清理非激活的弹幕
        if (!projectiles[i]->active) {
            delete projectiles[i];
            projectiles.erase(projectiles.begin() + i);
        }
        else {
            i++;
        }
    }
}


/* 游戏主逻辑函数,实现了
更新玩家和boss的逻辑
BOSS出招判定
玩家攻击BOSS的判定
管理弹幕等功能
以及玩家HP小于0后的重置逻辑
*/
void GameLogic(DWORD& gameStartTime) {
	UpdateCamera(player); // 更新镜头位置
    if (player.hp > 0) {
        // 1. 更新逻辑
		player.update(); // 玩家动画和状态更新
        boss.update(); // BOSS动画和状态更新

        AttackBoss(); // 玩家攻击 BOSS 的判定
        ProjectileManager(); //管理弹幕
    }
    else {
        if (GetAsyncKeyState('R') & 0x8000){
            GameReset(gameStartTime);
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            // 优雅退出：先关闭绘图窗口，再结束程序
            closegraph();
            exit(0);
        }
        
    }
}

void DrawEntities() {
    if (debug_mode) {
        setlinecolor(GREEN); // 玩家本体用绿色框
        Rect pRect = player.getHitbox();
        rectangle((int)pRect.x-cameraX, (int)pRect.y-cameraY, (int)(pRect.x-cameraX + pRect.w), (int)(pRect.y-cameraY + pRect.h));

        if (player.isAttacking) {
            setlinecolor(YELLOW); // 玩家攻击范围用黄色框
            rectangle((int)player.attackBox.x - cameraX, (int)player.attackBox.y - cameraY,
                (int)(player.attackBox.x - cameraX + player.attackBox.w),
                (int)(player.attackBox.y - cameraY + player.attackBox.h));
        }
    }
    // 如果开启了调试模式则绘制红框
    for (auto p : projectiles) { p->draw(); if (debug_mode) p->drawDebug(); } // 画弹幕
    if (debug_mode) { boss.drawDebug(); }
    boss.draw(); //画Boss
    if (player.hp > 0) player.draw(); // 画玩家
};


void UpdateCamera(Player& p) {

    float targetX = p.x - (WINDOW_W / 2.0f);

	// 玩家往上走且处于二阶段转换时，抬高底线
    if (p.y < 200 && currentLevelBottom > 400 && boss.isPhaseTransition) {currentLevelBottom = 400;}
    if (!(boss.isPhaseClimbing || boss.isPhaseThree) || ((boss.isPhaseClimbing || boss.isPhaseThree) && p.y >= -4500)) { targetX = p.x - (WINDOW_W / 2.0f); }
	if (p.y < -4550 && currentLevelBottom >= -4400 && (boss.isPhaseClimbing || boss.isPhaseThree)) {
		currentLevelBottom = -4400;
        targetX = 0;
	}

    // 目标：让玩家处于屏幕中心
	// 被减数：想要以它为中心的那个实体的坐标；
	// 减数：那个中心物体在屏幕上的位置
    
    cameraX += (targetX - cameraX) * 0.1f;
    if (cameraX < -200) cameraX = -200;
    if (cameraX > 200) cameraX = 200;

    // boss存活且不是二阶段，爬梯阶段时会保持镜头Y轴在默认位置
    if (boss.active && !boss.isPhaseTwoActive && !(boss.isPhaseClimbing || boss.isPhaseThree)) {
        float targetY = 0;
        cameraY += (targetY - cameraY) * 0.1f; //镜头跟随
    }
    else {
        float targetY;
        // Boss 消失了 (进入攀爬阶段) -> 开启 Y 轴自由跟随
        // 目标是让玩家保持在屏幕垂直中心偏下一点的位置
        if (p.y < -4400 && (boss.isPhaseClimbing || boss.isPhaseThree) ) {
            // 锁定镜头高度，不再随玩家跳跃起伏
            // 这个值需要根据你最终平台的实际高度微调，目标是让平台处于屏幕中下部
            targetY = -4550 - (WINDOW_H * 0.75f);
        }
        else {
            targetY = p.y - WINDOW_H * 0.55f;
            float maxCamY = currentLevelBottom - WINDOW_H;
            if (targetY > maxCamY) targetY = maxCamY;
        }

        cameraY += (targetY - cameraY) * 0.05f;
    }
}