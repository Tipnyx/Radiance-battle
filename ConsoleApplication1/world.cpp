#include"world.h"
#include"player.h"
#include"boss.h"

std::vector<Rect> platforms;

void make_one_stair(int X, int Y, int W, int H) {
    Rect platform;
    platform.x = X;
    platform.y = Y;
    platform.w = W;
    platform.h = H;
    platforms.push_back(platform);
}

void GenerateStairs() {
    float startX = WINDOW_W / 2;
    float currentY = PLATFORM_Y - 250;
    make_one_stair(681, 450, 150, 50);
    make_one_stair(500, 200, 175, 50);
    make_one_stair(120, 50, 175, 50);
    make_one_stair(800, 0, 100, 50);
    make_one_stair(480, -300, 175, 50);
    make_one_stair(0, -200, 100, 50);
    make_one_stair(980, -280, 100, 50);
}

void GenerateUPStairs() {
    make_one_stair(655, -550, 175, 50);
    make_one_stair(480, -800, 100, 50);
    make_one_stair(700, -1050, 100, 100);
    make_one_stair(745, -1300, 100, 50);
    make_one_stair(470, -1550, 175, 50);
    make_one_stair(390, -1800, 100, 100);
    make_one_stair(515, -2050, 100, 50);
    make_one_stair(635, -2300, 175, 50);
    make_one_stair(510, -2550, 100, 50);
    make_one_stair(760, -2800, 100, 100);
    make_one_stair(820, -3050, 100, 50);
    make_one_stair(500, -3300, 175, 50);
    make_one_stair(410, -3550, 100, 100);
    make_one_stair(550, -3800, 100, 50);
    make_one_stair(650, -4050, 175, 50);
    make_one_stair(601, -4300, 100, 50);
    make_one_stair(401, -4550, 100, 50);
    make_one_stair(801, -4550, 100, 50);
}

void InitPlatform() {
    platforms.clear();
    platforms.push_back({ (float)PLATFORM_X, (float)PLATFORM_Y, (float)PLATFORM_W, 500 });
}

void DrawSpikes(float x, float y, float w, float h) {
    Color colorGlow = MakeColor(255, 120, 20);
    Color colorHalo = MakeColor(255, 230, 150);
    Color colorCore = COLOR_WHITE;

    int numSpikes = (int)(w / 42) + 1;
    float margin = 15.0f;
    float step = (w - 2 * margin) / (numSpikes - 1);

    for (int i = 0; i < numSpikes; i++) {
        float cx = x + margin + i * step;
        float bot = y + h;
        float top = bot - 40;

        glSetColor(colorCore);
        glSetLineWidth(1);
        POINT pts[] = {
            {(int)cx - 7 - cameraX, (int)bot - cameraY},
            {(int)cx + 7 - cameraX, (int)bot - cameraY},
            {(int)cx - cameraX, (int)top - cameraY}
        };
        drawFilledPolygon(pts, 3);

        POINT guard[] = {
            {(int)cx - cameraX, (int)bot - 12 - cameraY},
            {(int)cx - 16 - cameraX, (int)bot - 2 - cameraY},
            {(int)cx + 16 - cameraX, (int)bot - 2 - cameraY}
        };
        drawFilledPolygon(guard, 3);

        glSetColor(COLOR_WHITE);
        glSetLineWidth(2);
        drawLine(cx - cameraX, bot - 2 - cameraY, cx - cameraX, top + 8 - cameraY);
    }
    glSetLineWidth(1);
}

extern Player player;
extern Boss boss;

void DrawUI() {
    int maxHp = 10;
    int startX = 60;
    int startY = 60;
    int spacing = 45;

    for (int i = 0; i < maxHp; i++) {
        int cx = startX + i * spacing;
        int cy = startY;

        if (i < player.hp) {
            glSetColor(COLOR_WHITE);
            drawFilledEllipse(cx, cy, 14, 18);
            glSetColor(COLOR_BLACK);
            drawFilledEllipse(cx - 5, cy, 3, 5);
            drawFilledEllipse(cx + 5, cy, 3, 5);
        }
        else {
            glSetColor(MakeColor(40, 40, 60));
            glSetLineWidth(2);
            drawLineEllipse(cx, cy, 12, 15);
            glSetLineWidth(1);
        }
    }

    // YOU DIED 文字暂时跳过（OpenGL 原生不直接支持文字渲染）
}

void DrawSinglePlatform(const Rect& r) {
    int radius = 15;

    // 底板
    glSetColor(MakeColor(60, 70, 90));
    drawFilledRoundRect(r.x - cameraX, r.y - cameraY, r.w, r.h, (float)radius);

    // 面层
    glSetColor(MakeColor(20, 24, 35));
    drawFilledRoundRect(r.x - cameraX, r.y + 4 - cameraY, r.w, r.h, (float)radius);

    // 装饰点
    if (r.w * r.h > 200) {
        int dotCount = (int)(r.w * r.h / 400);
        if (dotCount < 3) dotCount = 3;
        if (dotCount > 50) dotCount = 50;
        int padding = 5;
        int rangeX = (int)r.w - 2 * padding;
        int rangeY = (int)r.h - 8 - padding;
        if (rangeX > 0 && rangeY > 0) {
            for (int i = 0; i < dotCount; i++) {
                int rx = (int)r.x + padding + rand() % rangeX;
                int ry = (int)r.y + 6 + rand() % rangeY;
                int type = rand() % 3;
                if (type == 0) glSetColor(MakeColor(70, 80, 100));
                else if (type == 1) glSetColor(MakeColor(10, 12, 18));
                else continue;
                drawFilledCircle(rx - cameraX, ry - cameraY, 1);
            }
        }
    }

    // 裂纹
    if (r.w > 100 && r.h > 40) {
        glSetColor(MakeColor(45, 50, 70));
        glSetLineWidth(1);
        for (int i = 0; i < 3; i++) {
            int sx = (int)r.x + 5 + rand() % ((int)r.w - 10);
            int sy = (int)r.y + 10 + rand() % ((int)r.h - 20);
            int len = 10 + rand() % 20;
            drawLine(sx - cameraX, sy - cameraY, sx - cameraX + len, sy - cameraY + (rand() % 3 - 1));
        }
    }

    // 边框
    glSetColor(MakeColor(40, 50, 70));
    glSetLineWidth(1);
    drawLineRoundRect(r.x - cameraX, r.y + 4 - cameraY, r.w, r.h, (float)radius);
}

void DrawPlatform() {
    for (const auto& p : platforms) {
        DrawSinglePlatform(p);
    }
}

void LastSpike() {
    float sideWidth = PLATFORM_W / 4.0f;
    Rect leftSpikeRect = { (float)PLATFORM_X, (float)PLATFORM_Y - 5, sideWidth, 10 };
    Rect rightSpikeRect = { (float)PLATFORM_X + PLATFORM_W * 0.75f, (float)PLATFORM_Y - 5, sideWidth, 10 };

    DrawSpikes(leftSpikeRect.x, leftSpikeRect.y, leftSpikeRect.w, leftSpikeRect.h);
    DrawSpikes(rightSpikeRect.x, rightSpikeRect.y, rightSpikeRect.w, rightSpikeRect.h);

    if (!player.isInvincible) {
        Rect pRect = player.getHitbox();
        pRect.x += 10; pRect.w -= 20;
        if (pRect.checkCollision(leftSpikeRect) || pRect.checkCollision(rightSpikeRect)) {
            if (pRect.y + pRect.h > PLATFORM_Y - 25) {
                player.hp--;
                player.isInvincible = true;
                player.vy = -8.0f;
            }
        }
    }

    if (player.isAttacking && player.attackDir == 2) {
        if (player.attackBox.checkCollision(leftSpikeRect) || player.attackBox.checkCollision(rightSpikeRect)) {
            if (player.y + player.h <= PLATFORM_Y + 10) {
                player.vy = -9.0f;
                player.jumpCount = 1;
                player.hasDashedInAir = false;
            }
        }
    }
}

void SpikeManager(DWORD gameStartTime) {
    DWORD now = GetTickCount();

    if (boss.isPhaseOneLast) {
        LastSpike();
        return;
    }

    if (currentSpikeState == SPIKE_HIDDEN) {
        if (boss.hp < 650 && !boss.isPhaseTransition) {
            currentSpikeState = SPIKE_WARNING;
            spikeTimer = now;
            spikeOnLeft = true;
        }
    }
    else if (currentSpikeState == SPIKE_WARNING) {
        if (now - spikeTimer > DURATION_WARNING) {
            currentSpikeState = SPIKE_ACTIVE;
            spikeTimer = now;
        }
    }
    else if (currentSpikeState == SPIKE_ACTIVE) {
        if (now - spikeTimer > DURATION_ACTIVE) {
            currentSpikeState = SPIKE_WARNING;
            spikeTimer = now;
            spikeOnLeft = !spikeOnLeft;
        }
    }

    Rect currentSpikeRect = { 0, 0, 0, 0 };
    int spikeY = PLATFORM_Y - 5;
    if (spikeOnLeft)
        currentSpikeRect = { (float)PLATFORM_X, (float)spikeY, (float)PLATFORM_W / 2.0f, 10 };
    else
        currentSpikeRect = { (float)PLATFORM_X + PLATFORM_W / 2.0f, (float)spikeY, (float)PLATFORM_W / 2.0f, 10 };

    if (currentSpikeState == SPIKE_WARNING) {
        if ((now / 100) % 2 == 0) glSetColor(MakeColor(255, 255, 200));
        else glSetColor(MakeColor(100, 100, 0));
        drawFilledRect(currentSpikeRect.x - cameraX, PLATFORM_Y - 5 - cameraY,
                       currentSpikeRect.w, 5);
    }
    else if (currentSpikeState == SPIKE_ACTIVE) {
        DrawSpikes(currentSpikeRect.x, currentSpikeRect.y, currentSpikeRect.w, currentSpikeRect.h);

        if (player.isAttacking && player.attackDir == 2) {
            if (player.attackBox.checkCollision(currentSpikeRect)) {
                if (player.y + player.h <= PLATFORM_Y + 10) {
                    player.vy = -9.0f;
                    player.jumpCount = 1;
                    player.hasDashedInAir = false;
                }
            }
        }
        if (!player.isInvincible) {
            Rect pRect = player.getHitbox();
            pRect.x += 10; pRect.w -= 20;
            if (pRect.checkCollision(currentSpikeRect)) {
                if (pRect.y + pRect.h > PLATFORM_Y - 25) {
                    player.hp--;
                    player.isInvincible = true;
                    player.vy = -8.0f;
                }
            }
        }
    }
}
