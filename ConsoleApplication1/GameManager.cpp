#include"GameManager.h"
#include"player.h"
#include"boss.h"
#include"projectile.h"
#include"BossState.h"

extern Player player;
extern Boss boss;
extern std::vector<Projectile*> projectiles;
extern GLFWwindow* window;

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#include <cstdlib>

void GameReset(DWORD& gameStartTime) {
    player.reset();
    boss.reset();
    for (auto p : projectiles) delete p;
    projectiles.clear();
    gameStartTime = GetTickCount();
    boss.isPhaseOneLast = false;

    mciSendString(L"stop bgm", NULL, 0, NULL);
    mciSendString(L"seek bgm to start", NULL, 0, NULL);
    mciSendString(L"play bgm repeat", NULL, 0, NULL);

    currentSpikeState = SPIKE_HIDDEN;
    spikeTimer = 0;

    boss.active = true;
    currentLevelBottom = WINDOW_H;
    boss.isPhaseTwoActive = false;
    boss.isPhaseTransition = false;
    boss.isPhaseClimbing = false;
    boss.isPhaseThree = false;

    platforms.clear();
    platforms.push_back({ (float)PLATFORM_X, (float)PLATFORM_Y, (float)PLATFORM_W, 500 });

    boss.isDefeated = false;
    boss.ChangeState(new PhaseOneState());
}

void AttackBoss() {
    int hittime = GetTickCount();
    if (hittime - boss.boss_invincible_start_time >= boss.BOSS_INVINCIBLE_DURATION)
        boss.boss_is_invincible = false;

    if (player.isAttacking) {
        Rect bRect = boss.getRect();
        if (player.attackBox.checkCollision(bRect)) {
            if (!boss.boss_is_invincible) {
                boss.hp -= 20;
                boss.boss_is_invincible = true;
                boss.boss_invincible_start_time = GetTickCount();
            }
            if (player.attackDir == 2) {
                player.vy = -7.0f;
                player.jumpCount = 1;
                player.hasDashedInAir = false;
            }
            if (player.atkTimer == player.atkDuration - 1) Sleep(50);
        }
    }
}

void ProjectileManager() {
    for (size_t i = 0; i < projectiles.size(); ) {
        projectiles[i]->update(player);
        if (projectiles[i]->active) {
            Rect pRect = player.getHitbox();
            bool playerHit = false;
            bool pogoHit = false;

            if (projectiles[i]->type == 1) {
                Sword* sw = (Sword*)projectiles[i];
                if (sw->state == SWORD_LAUNCH) {
                    auto hitPts = sw->getHitPoints();
                    for (auto& p : hitPts) {
                        if (p.x >= pRect.x && p.x <= pRect.x + pRect.w &&
                            p.y >= pRect.y && p.y <= pRect.y + pRect.h) {
                            playerHit = true; break;
                        }
                    }
                    if (player.isAttacking && player.attackDir == 2) {
                        for (auto& p : hitPts) {
                            if (p.x >= player.attackBox.x && p.x <= player.attackBox.x + player.attackBox.w &&
                                p.y >= player.attackBox.y && p.y <= player.attackBox.y + player.attackBox.h) {
                                pogoHit = true; break;
                            }
                        }
                    }
                }
            }
            else if (projectiles[i]->type == 3) {
                Laser* laser = (Laser*)projectiles[i];
                std::vector<POINT> pts = laser->getHitPoints();
                for (auto p : pts) {
                    if (p.x >= pRect.x && p.x <= pRect.x + pRect.w &&
                        p.y >= pRect.y && p.y <= pRect.y + pRect.h) {
                        playerHit = true; break;
                    }
                }
            }
            else {
                Rect bRect = projectiles[i]->getRect();
                if (pRect.checkCollision(bRect)) playerHit = true;
                if (player.isAttacking && player.attackDir == 2 && player.attackBox.checkCollision(bRect))
                    pogoHit = true;
            }

            if (pogoHit) {
                player.vy = -7.0f;
                player.jumpCount = 1;
                player.hasDashedInAir = false;
            }

            if (playerHit) {
                if (!player.isInvincible) {
                    TriggerScreenShake(8.0f);
                    player.hp--;
                    player.hurtTimer = player.HURT_DURATION;
                    player.isInvincible = true;
                    if (projectiles[i]->type == 0) projectiles[i]->active = false;
                }
                else {
                    if (projectiles[i]->type == 0) projectiles[i]->active = false;
                }
            }
        }

        if (!projectiles[i]->active) {
            delete projectiles[i];
            projectiles.erase(projectiles.begin() + i);
        }
        else i++;
    }
}

void GameLogic(DWORD& gameStartTime) {
    UpdateCamera(player);
    if (player.hp > 0) {
        player.update();
        boss.update();
        AttackBoss();
        ProjectileManager();
    }
    else {
        GameReset(gameStartTime);
    }
}

void DrawEntities() {
    if (debug_mode) {
        glSetColor(COLOR_GREEN);
        Rect pRect = player.getHitbox();
        drawLineRect(pRect.x - cameraX, pRect.y - cameraY, pRect.w, pRect.h);

        if (player.isAttacking) {
            glSetColor(COLOR_YELLOW);
            drawLineRect(player.attackBox.x - cameraX, player.attackBox.y - cameraY,
                         player.attackBox.w, player.attackBox.h);
        }
    }
    for (auto p : projectiles) { p->draw(); if (debug_mode) p->drawDebug(); }
    if (debug_mode) boss.drawDebug();
    boss.draw();
    if (player.hp > 0) player.draw();
}

void UpdateCamera(Player& p) {
    float targetX = p.x - (WINDOW_W / 2.0f);

    if (p.y < 200 && currentLevelBottom > 400 && boss.isPhaseTransition) currentLevelBottom = 400;
    if (!(boss.isPhaseClimbing || boss.isPhaseThree) || ((boss.isPhaseClimbing || boss.isPhaseThree) && p.y >= -4500))
        targetX = p.x - (WINDOW_W / 2.0f);
    if (p.y < -4550 && currentLevelBottom >= -4400 && (boss.isPhaseClimbing || boss.isPhaseThree)) {
        currentLevelBottom = -4400;
        targetX = 0;
    }

    cameraX += (targetX - cameraX) * 0.1f;
    if (cameraX < -200) cameraX = -200;
    if (cameraX > 200) cameraX = 200;

    if (boss.active && !boss.isPhaseTwoActive && !(boss.isPhaseClimbing || boss.isPhaseThree)) {
        float targetY = 0;
        cameraY += (targetY - cameraY) * 0.1f;
    }
    else {
        float targetY;
        if (p.y < -4400 && (boss.isPhaseClimbing || boss.isPhaseThree))
            targetY = -4550 - (WINDOW_H * 0.75f);
        else {
            targetY = p.y - WINDOW_H * 0.55f;
            float maxCamY = currentLevelBottom - WINDOW_H;
            if (targetY > maxCamY) targetY = maxCamY;
        }
        cameraY += (targetY - cameraY) * 0.05f;
    }
}
