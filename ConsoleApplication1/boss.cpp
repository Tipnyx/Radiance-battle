# include"boss.h"
# include"projectile.h"
# include"player.h"


extern std::vector<Projectile*> projectiles;
extern Player player;
extern Boss boss;

// 将颜色根据 alpha 进行亮度缩放，模拟渐显
COLORREF Fade(COLORREF c, float a) {
    return RGB(GetRValue(c) * a, GetGValue(c) * a, GetBValue(c) * a);
}

Boss::Boss() {
        x = WINDOW_W / 2;
        y = -50; // 悬浮在空中
    }

void Boss::update() {
    //入场效果
    if (alpha < 1.0f) {
        alpha += 0.0033f; //调整这个值控制进场速度，0.005约等于3-4秒
        y += 1.0f;
        
    }
    // 终止位置
    else{
        y = 250 + sin(GetTickCount() / 500.0f) * 20;
    }
    // 简单的悬浮律动 (正弦波)

    // 2. 新增：平滑水平位移逻辑
    if (isTeleporting) {
        // 使用 Lerp 公式：当前位置 += (目标 - 当前) * 速度因子
        // 0.15f 这个值越大，移动越快；越小，拖尾越长越慢
        x += (targetX - x) * 0.15f;

        // 记录当前位置到拖尾数组
        TrailPoint p = { x, y, alpha };
        trails.push_back(p);
        if (trails.size() > MAX_TRAILS) {
            trails.erase(trails.begin()); // 保持长度
        }

        // 如果距离目标足够近，结束位移并清空拖尾（可选渐隐）
        if (fabs(x - targetX) < 0.5f) {
            x = targetX;
            isTeleporting = false;
        }
    }
    else {
        // 如果没在位移，慢慢清空拖尾，让拖尾自然消失
        if (!trails.empty()) {
            trails.erase(trails.begin());
        }
    }
    
}

void Boss::draw() {
    if (!active) return;

    // --- 新增：绘制拖尾残影 ---
    for (size_t i = 0; i < trails.size(); i++) {
        // 越靠前的残影越透明
        float trailAlpha = (i / (float)trails.size()) * alpha * 0.4f;
        int tx = (int)trails[i].x;
        int ty = (int)trails[i].y;

        // 这里只需要画一个简化的太阳主体作为残影，否则性能消耗太大
        for (int k = 0; k < 60; k += 10) {
                setfillcolor(Fade(RGB(255, 180 - k, 100 - k), trailAlpha));
                solidcircle(tx, ty, 80 - k);
        }
    }

	// --- 正常绘制 Boss 本体 ---
    int cx = (int)x;
    int cy = (int)y;
    int r = 150;
    // 1. 旋转背景

    for (int j = 0; j < 3; j++) {
        setlinecolor(Fade(RGB(240, 240, 240),alpha));
        setlinestyle(PS_SOLID, 5);
        setfillcolor(NULL);
        circle(cx, cy, 90 + 30 * j);
    }

    for (int i = 0; i < 8; i++) {
           
        float angle = i * 3.1415f / 4.0f + (GetTickCount() / 1000.0f); //45度的间隔,随时间旋转(通过时间偏移实现)
        int ex = cx + (int)(cos(angle) * r);
        int ey = cy + (int)(sin(angle) * r);
        
        setlinecolor(Fade(RGB(255, 255, 220),alpha));
        setlinestyle(PS_SOLID, 11+ (rand() % 3));
        line(cx, cy, ex, ey);
        
        setlinecolor(Fade(RGB(204, 218, 221),alpha));
        setlinestyle(PS_SOLID, 9);
        line(cx, cy, ex, ey);
        
        // --- 末端尖刺逻辑 ---
        float spikeLen = 45.0f;    // 尖刺从末端往外延伸的长度
        float spikeWidth = 18.0f;  // 尖刺底部的宽度（一半）
        float baseR = r - 10.0f;   // 尖刺底座的位置（稍微缩进光束一点，衔接更自然）

        POINT pts[3];
        // 尖刺顶点：在 angle 方向延伸
        pts[0].x = cx + (int)((baseR + spikeLen) * cos(angle));
        pts[0].y = cy + (int)((baseR + spikeLen) * sin(angle));

        // 尖刺底边左角：利用垂直向量 (-sin, cos) 进行偏移
        pts[1].x = cx + (int)(baseR * cos(angle) - spikeWidth * sin(angle));
        pts[1].y = cy + (int)(baseR * sin(angle) + spikeWidth * cos(angle));

        // 尖刺底边右角：利用垂直向量 (sin, -cos) 进行偏移
        pts[2].x = cx + (int)(baseR * cos(angle) + spikeWidth * sin(angle));
        pts[2].y = cy + (int)(baseR * sin(angle) - spikeWidth * cos(angle));

        // 填充颜色：使用稍微亮一点的米白色，制造立体感
        setfillcolor(Fade(RGB(255, 255, 240),alpha));
        solidpolygon(pts, 3);

    }

    // 2. 主体像个太阳
    for (int i = 0; i < 90; i++) {
        if (boss.boss_is_invincible)
        {
            setfillcolor(Fade(RGB(255, 255 - i, 255 - i), alpha));
            solidellipse(cx - 90 + i, cy - 90 + i, cx + 90 - i, cy + 90 - i);
        }
        else{
            setfillcolor(Fade(RGB(255, 192 - i, 107 - i), alpha));
            solidellipse(cx - 90 + i, cy - 90 + i, cx + 90 - i, cy + 90 - i);
        }
    }
    setfillcolor(NULL);
	setlinecolor(Fade(RGB(255, 112, 17),alpha));
	setlinestyle(PS_SOLID, 3);
	circle(cx, cy, 90 + (rand() % 3)); // 微微抖动的边框

}

Rect Boss::getRect() {
    return { x - 90,y - 90, 180,180 };
}

void Boss::reset() {
	hp = 1000;
	x = WINDOW_W / 2;
	y = -50;
	alpha = 0.0f;
	isTeleporting = false;
	trails.clear();
}

void Boss::drawDebug() {
    if (!debug_mode) return;
    setlinecolor(RED);
    //setfillstyle(BS_NULL); // 无填充
    Rect r = getRect();
    rectangle((int)r.x, (int)r.y, (int)(r.x + r.w), (int)(r.y + r.h));
    
    settextcolor(WHITE);
    settextstyle(20, 0, _T("Consolas"));
    TCHAR s[50];
    _stprintf_s(s, _T("BOSS HP: %d"), boss.hp);
    outtextxy(10, 100, s);
}

void Boss::SpawnSwordWallHorizontal(bool fromLeft) {
    //printf("横剑\n");
	// 先定义一下画横剑的参数,startX,vx, angle,这都是好直接确定的
    float startX = fromLeft ? -100.0f : WINDOW_W + 100.0f; // 从左还是从右，初始坐标
    float vx = fromLeft ? 11.0f : -11.0f; // 对应的速度方向
    float angle = fromLeft ? 0.0f : 3.14159f;

    // 现在我们要算y的分布了
    int totalSwords = 12; // 定义多少把剑
    int spacing = 70;     // 剑之间的间隔多大,注意我们的主角是40格宽,65格高的

    int gapA = rand() % totalSwords;
    int gapB = rand() % totalSwords;
    int gapC = rand() % totalSwords;

    // 确保缺口不会生成在太高或太低的位置，保留上下边界
    int gapStart = 1;
    int randomOffset = (rand() % 50) - 40; // 在 Y 轴方向整体上下浮动 -25 到 +25 像素

    for (int i = 0; i < totalSwords; i++) {
        if (i >= gapStart && (i == gapA || i == gapB || i == gapC)) continue;
        // 分布在Y轴上，覆盖玩家可能跳跃的高度
        float py = PLATFORM_Y  - (i * spacing) + randomOffset;
        projectiles.push_back(new Sword(startX, py, vx, 0, angle, false));
    }
}

void Boss::SpawnSwordWallVertical() {
    //printf("竖剑\n");
	int totalSwords = 20; // 定义多少把剑
	float spacing = 75.0f; // 剑之间的间隔多大
    int gapStart = 2;

    int gapA = rand() % totalSwords;
    int gapB = rand() % totalSwords;
    int gapC = rand() % totalSwords;

    int randomOffset = (rand() % 60) - 30;

    float startXBase = PLATFORM_X;

    for (int i = 0; i < 15; i++) {
        if (i >= gapStart && (i == gapA || i == gapB || i == gapC)) continue; // gap跳过
        float px = startXBase + randomOffset + (i * spacing);
        projectiles.push_back(new Sword(px, -120.0f, 0, 15.0f, 1.5708f, false));
    }
}

void Boss::SpawnOrbs() {
    //printf("光球\n");
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
    //printf("光柱\n");
    bool fromLeft = rand() % 2 == 0;
    float startX = fromLeft ? -100.0f : WINDOW_W + 100.0f;
    float spd = fromLeft ? 10.0f : -10.0f;
    projectiles.push_back(new Beam(startX, spd));
}

void Boss::SpawnSwordBurst() {
    //printf("脸刺\n");
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

// 三连激光

void Boss::SpawnLaserBurst() {
    //printf("辐射激光 - 第 %d 波\n", laserWaveCount + 1);

    int count = 8; // 一圈 8 条激光，构成全方位封锁
    float step = (2 * 3.14159f) / count; // 每条激光间隔 45 度 (360 / 8)

    // --- 核心逻辑：波次交错 ---
    // 每一波的基础角度都偏移 step 的 1/3
    // 这样三波打完，刚好覆盖了之前的空隙，逼迫玩家必须走位
    float waveOffset = (laserWaveCount % 3) * (step / 3.0f);

    // 加上一点随机扰动 (-5度 到 +5度)，让每次 Boss 战都不一样
    float randomJitter = ((rand() % 100) / 100.0f * 0.17f) - 0.08f;

    // 基础旋转：为了避免激光总是死板地呈"十字"或"X型"，给一个初始旋转
    float baseRotation = 0.0f;

     //如果想稍微"针对"一下玩家，可以让基础角度大致指向玩家，但依然是全屏散射
    /*float dx = (player.x + player.w / 2) - x;
    float dy = (player.y + player.h / 2) - y;
    baseRotation = atan2(dy, dx); */

    for (int i = 0; i < count; i++) {
        // 最终角度 = 基础旋转 + 波次偏移 + 随机扰动 + 循环增量
        float angle = baseRotation + waveOffset + randomJitter + (i * step);

        projectiles.push_back(new Laser(x, y, angle));
    }
}

// Boss AI逻辑
void Boss::BossAI() {

	// 入场未完成，暂停攻击逻辑
    if (alpha < 1.0f) {
        lastAttackTime = GetTickCount();
        return;
    }
    
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

    // --- 激光三连射逻辑 ---
    if (laserAttackActive) {
        DWORD elapsed = currentTime - lastLaserTime;

        // 节奏：每隔 800ms 射一波 (预警0.7s + 爆发0.5s，所以波次间要有重叠才刺激)
        if (elapsed > 800 && laserWaveCount < 3) {
            SpawnLaserBurst();
            lastLaserTime = currentTime;
            laserWaveCount++;
        }

        // 三波射完后，多留点时间给最后的一波消失
        if (laserWaveCount >= 3 && elapsed > 1500) {
            laserAttackActive = false;
            lastAttackTime = currentTime; // 结束攻击
        }
        return; // 激光期间独占 AI
    }

    // 基础攻击间隔
    if (currentTime - lastAttackTime > 2000) {
        lastAttackTime = currentTime;
        
		// 避免连续出现同一种攻击
        static int lastAttack = -1;
        int attackType;

        // --- 核心逻辑修改：每 3 次攻击固定位移一次 ---
        attackCycle++; // 每次进入攻击选择，计数加 1

        if (attackCycle >= 3) {
            // 达到第 3 次，强制执行位移 (attackType 5)
            attackType = 6;
            attackCycle = 0; // 重置计数器
        }
        else {
            // 前 3 次，在 0-5 招式之间随机（不含 6）
            do {
                attackType = rand() % 6;
            } while (attackType == lastAttack);
        }

        //printf("[AI] pick attackType = %d\n", attackType);

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
        else if (attackType == 5) { // 新增：三连激光
            laserAttackActive = true;
            laserWaveCount = 0;
            lastLaserTime = currentTime - 1000; // 这里的减法是为了让第一波立即发射
        }
        else if (attackType == 6) {
            targetX = WINDOW_W * (rand() % 3 + 1) / 4; // 计算目标四分点
            isTeleporting = true; // 开启位移状态
            //printf("[AI] Teleport to targetX = %f\n", targetX);
        }
        
    }

}