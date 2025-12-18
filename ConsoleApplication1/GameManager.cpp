#include"GameManager.h"
#include"player.h"
#include"boss.h"
#include"projectile.h"

extern Player player;
extern Boss boss;
extern std::vector<Projectile*> projectiles;

/* 重置游戏:
玩家重置
删除所有弹幕
重置计时器
重置地刺状态
*/
void GameReset(DWORD gameStartTime) {
    // 死亡重开
    player.reset();
    for (auto p : projectiles) delete p;
    projectiles.clear();
    gameStartTime = GetTickCount();
    // 重置地刺
    currentSpikeState = SPIKE_HIDDEN;
    spikeTimer = 0;
}

/* 此函数实现玩家攻击BOSS的判定,及攻击到后的行为*/
void AttackBoss() {
    if (player.isAttacking) {
        Rect bRect = { boss.x - 50, boss.y - 60, 100, 120 }; // BOSS 的碰撞盒
        if (player.attackBox.checkCollision(bRect)) {
            boss.hp -= 2; // 掉血
            // 以后可以在这里加“回魂”逻辑

            // 攻击瞬间的顿帧效果
            if (player.atkTimer == player.atkDuration - 1) { // 只在第一帧停顿
                // Sleep(10); 
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
void GameLogic(DWORD gameStartTime) {
    if (player.hp > 0) {
        // 1. 更新逻辑
		player.update(); // 玩家动画和状态更新
        boss.update(); // BOSS动画
		boss.BossAI(); // BOSS 出招判定

        AttackBoss(); // 玩家攻击 BOSS 的判定
        ProjectileManager(); //管理弹幕
    }
    else {
        GameReset(gameStartTime);
    }
}