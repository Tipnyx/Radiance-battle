# include"boss.h"
# include"projectile.h"
# include"player.h"

// Force recompile
Boss::Boss() {
        x = WINDOW_W / 2;
        y = 200; // 悬浮在空中
    }

void Boss::update() {
    // 简单的悬浮律动 (正弦波)
    y = 200 + sin(GetTickCount() / 500.0f) * 20;
}

void Boss::draw() {
    if (!active) return;

    int cx = (int)x;
    int cy = (int)y;

    // --- 绘制辐光本体 (简化版：发光大飞蛾) ---
    // 1. 底层金光 (放射状)
    for (int i = 0; i < 8; i++) {
        float angle = i * 3.1415f / 4.0f + (GetTickCount() / 1000.0f);
        setlinecolor(RGB(255, 255, 150));
        setlinestyle(PS_SOLID, 2);
        line(cx, cy, cx + (int)(cos(angle) * 120), cy + (int)(sin(angle) * 120));
    }

    // 2. 身体核心 (白色)
    setfillcolor(WHITE);
    solidellipse(cx - 25, cy - 40, cx + 25, cy + 40);

    // 3. 巨大的羽翼 (淡黄色)
    setfillcolor(RGB(255, 255, 230));
    // 左翼
    POINT leftWing[] = { {cx - 20, cy}, {cx - 100, cy - 60}, {cx - 80, cy + 40} };
    solidpolygon(leftWing, 3);
    // 右翼
    POINT rightWing[] = { {cx + 20, cy}, {cx + 100, cy - 60}, {cx + 80, cy + 40} };
    solidpolygon(rightWing, 3);

    // 4. 发光的眼睛
    setfillcolor(RGB(255, 100, 0)); // 经典的感染橙
    solidcircle(cx - 8, cy - 15, 5);
    solidcircle(cx + 8, cy - 15, 5);
}

extern std::vector<Projectile*> projectiles;
extern Player player;
extern Boss boss;

void Boss::SpawnSwordWallHorizontal(bool fromLeft) {
    int totalSwords = 12; // 剑的总数
    int spacing = 60;     // 剑之间的间隔

    int gapA = rand() % totalSwords;
    int gapB = rand() % totalSwords;
    int gapC = rand() % totalSwords;

    // 确保缺口不会生成在太高或太低的位置，保留上下边界
    int gapStart = 1;
    int randomOffset = (rand() % 50) - 25; // 在 Y 轴方向整体上下浮动 -25 到 +25 像素

    float startX = fromLeft ? -100.0f : WINDOW_W + 100.0f; // 从左还是从右，初始坐标
    float vx = fromLeft ? 10.0f : -10.0f; // 对应的速度方向
    float angle = fromLeft ? 0.0f : 3.14159f;

    for (int i = 0; i < 12; i++) {
        if (i >= gapStart && (i == gapA || i == gapB || i == gapC)) continue;
        // 分布在Y轴上，覆盖玩家可能跳跃的高度
        float py = PLATFORM_Y - 50 - (i * spacing) + randomOffset;

        if (py > PLATFORM_Y - 10) continue;

        projectiles.push_back(new Sword(startX, py, vx, 0, angle, false));
    }
}

void Boss::SpawnSwordWallVertical() {
    int totalSwords = 20;
    float spacing = 60.0f;
    int gapStart = 2;

    int gapA = rand() % totalSwords;
    int gapB = rand() % totalSwords;
    int gapC = rand() % totalSwords;

    int randomOffset = (rand() % 60) - 30;

    float startXBase = PLATFORM_X;

    for (int i = 0; i < 15; i++) {
        if (i >= gapStart && (i == gapA || i == gapB || i == gapC)) continue; // gap跳过
        float px = startXBase + randomOffset + (i * spacing);
        projectiles.push_back(new Sword(px, -150.0f, 0, 15.0f, 1.5708f, false));
    }
}

void Boss::SpawnOrbs() {
    const float minDist = 200.0f; //最小距离
    float ox, oy;
    int tryCount = 0;
    do {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float dist = 300.0f;
        ox = (WINDOW_W / 2) + cos(angle) * dist;
        oy = (PLATFORM_Y - 200) - sin(angle) * 100;
        float dx = ox - (player.x + player.w / 2);
        float dy = oy - (player.y + player.h / 2);
        if (sqrt(dx * dx + dy * dy) >= minDist) break;
        tryCount++;
    } while (tryCount < 20);

    Orb* orb = new Orb(ox, oy, player.x + player.w / 2, player.y + player.h / 2);
    projectiles.push_back(orb);
}

void Boss::SpawnBeam() {
    bool fromLeft = rand() % 2 == 0;
    float startX = fromLeft ? -100.0f : WINDOW_W + 100.0f;
    float spd = fromLeft ? 10.0f : -10.0f;
    projectiles.push_back(new Beam(startX, spd));
}

void Boss::SpawnSwordBurst() {
    int count = 12; // 保持 12 根
    float startRadius = 60.0f;

    // --- 关键：计算交错角度 ---
    // 第一波偏移为 0，第二波偏移半个步长，使其正好射向第一波的缝隙
    float step = (2 * 3.14159f) / count;
    float offsetAngle = (burstWaveCount == 1) ? (step / 2.0f) : 0.0f;

    for (int i = 0; i < count; i++) {
        // 修复：直接计算角度，不再跳过或重复 i=0
        float angle = (i * step) + offsetAngle;

        float sx = boss.x + cos(angle) * startRadius;
        float sy = boss.y + sin(angle) * startRadius;
        float speed = 15.0f;
        float svx = cos(angle) * speed;
        float svy = sin(angle) * speed;

        projectiles.push_back(new Sword(sx, sy, svx, svy, angle, true));
    }
}

// Boss AI逻辑
void Boss::BossAI() {
    DWORD currentTime = GetTickCount();

    // 处理环形剑连发逻辑
    if (burstAttackActive) {
        // 计算当前大招已经进行了多久
        DWORD elapsed = currentTime - lastBurstTime;

        // 第一波：0ms 触发
        if (burstWaveCount == 0) {
            SpawnSwordBurst(); // 内部 burstWaveCount 为 0，发射第一波
            burstWaveCount = 1;
        }
        // 第二波：500ms 触发 (此时第一波刚好 500ms 开始发射)
        else if (burstWaveCount == 1 && elapsed >= 500) {
            SpawnSwordBurst(); // 内部 burstWaveCount 为 1，发射交错角度的第二波
            burstWaveCount = 2;
        }
        // 结束判定：1.5s 后（给第二波留 1s 飞行时间）重置状态
        else if (burstWaveCount == 2 && elapsed >= 1000) {
            burstAttackActive = false;
            lastAttackTime = currentTime;
        }
        return;
    }

    //光球攻击正在进行
    if (orbAttackActive) {
        if (orbAttackCount < ORB_ATTACK_TOTAL && currentTime - orbAttackLastTime > ORB_ATTACK_INTERVAL) {
            orbAttackLastTime = currentTime;
            orbAttackCount++;
            SpawnOrbs();
        }
        //攻击结束
        if (orbAttackCount >= ORB_ATTACK_TOTAL) {
            orbAttackActive = false;
            lastAttackTime = currentTime;
        }
        return; //光球攻击期间其他攻击不能进行
    }

    //剑雨攻击正在进行
    if (swordAttackActive) {
        if (swordAttackCount < SWORD_ATTACK_TOTAL && currentTime - swordAttackLastTime > SWORD_ATTACK_INTERVAL) {
            swordAttackLastTime = currentTime;
            swordAttackCount++;
            if (swordAttackType == 1) {
                SpawnSwordWallHorizontal(swordAttackFromLeft);
            }
            else {
                SpawnSwordWallVertical();
            }
        }
        if (swordAttackCount >= SWORD_ATTACK_TOTAL) {
            swordAttackActive = false;
            lastAttackTime = currentTime;
        }
        return;
    }

    // 基础攻击间隔
    if (currentTime - lastAttackTime > 2000) {
        lastAttackTime = currentTime;
        int attackType = rand() % 5; // 增加到 5 种随机情况

        // 光球攻击
        if (attackType == 0) {
            orbAttackActive = true;
            orbAttackCount = 0;
            orbAttackLastTime = currentTime;
        }
        // 横剑雨
        else if (attackType == 1) { 
            swordAttackActive = true;
            swordAttackType = 1;
            swordAttackCount = 0;
            swordAttackFromLeft = rand() % 2 == 0;
        }
        // 纵向剑雨
        else if (attackType == 2) {
            swordAttackActive = true;
            swordAttackType = 2;
            swordAttackCount = 0;
        }
        // 光柱攻击
        else if (attackType == 3) {
            SpawnBeam();
        }
        // 环形剑攻击
        else if (attackType == 4) {
            burstAttackActive = true;
            burstWaveCount = 0;
            lastBurstTime = GetTickCount();
        }
    }

}