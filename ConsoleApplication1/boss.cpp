# include"boss.h"
# include"projectile.h"
# include"player.h"
# include"world.h"

extern std::vector<Projectile*> projectiles;
extern Player player;
extern Boss boss;

// 将颜色根据 alpha 进行亮度缩放，模拟渐显
COLORREF Fade(COLORREF c, float a) {
    return RGB(GetRValue(c) * a, GetGValue(c) * a, GetBValue(c) * a);
}

// 二阶段 Boss 可以瞬移的锚点 (X, Y)
// 这些坐标参考了你在 world.cpp 里生成的台阶位置
// 建议 Y 坐标比台阶高 100-150 像素，让 Boss 悬浮在台阶上方
struct BossAnchor {
    float x, y;
};

// 预设的瞬移点列表 (根据你的 GenerateStairs 数据推算的)
std::vector<BossAnchor> phaseTwoAnchors = {

    {500, 200 - 150},
    {120, 50 - 150},
    {800, 0 - 150},
    {480, -300 - 150},
    {0, -200 - 150},
    {980, -280 - 150}
};

Boss::Boss() {
        x = WINDOW_W / 2;
        y = -100; // 悬浮在空中
        InitSunCache();
        InitHitCache();
}

void Boss::update() {
    //入场效果
    if (alpha < 1.0f) {
        alpha += 0.0033f; //调整这个值控制进场速度，0.005约等于3-4秒
        
        if (!isPhaseTwoActive) {
            y += 1.0f;
        }
        return;
        
    }

    // 简单的悬浮律动 (正弦波)

    // 2. 悬浮律动逻辑 (修改！)
    // 如果是二阶段，我们希望它在 targetY 附近律动，而不是死板的 200
    float hoverBaseY = isPhaseTwoActive ? targetY : 200.0f;

    // 计算悬浮偏移量
    float hoverOffset = sin(GetTickCount() / 500.0f) * 20;

    // 平滑水平位移逻辑
    if (isTeleporting) {
        // 使用 Lerp 公式：当前位置 += (目标 - 当前) * 速度因子
        // 0.15f 这个值越大，移动越快；越小，拖尾越长越慢
        x += (targetX - x) * 0.10f;

        // --- 新增：Y 轴平滑移动 ---
        // 只有二阶段才需要大幅度垂直移动
        if (isPhaseTwoActive) {
            y += (targetY - y) * 0.10f;
        }
        else {
            // 一阶段还是保持原来的悬浮逻辑
            y = 200 + hoverOffset;
        }

        // 记录当前位置到拖尾数组
        TrailPoint p = { x, y, alpha };
        trails.push_back(p);
        if (trails.size() > MAX_TRAILS) {
            trails.erase(trails.begin()); // 保持长度
        }

        // 判定到达目标
        // 二阶段需要同时判断 X 和 Y 都到了
        float dist = sqrt(pow(x - targetX, 2) + pow(y - targetY, 2));
        if (dist < 2.0f) { // 稍微放宽一点判断
            x = targetX;
            if (isPhaseTwoActive) y = targetY; // 修正 Y
            isTeleporting = false;
        }
    }
    else {
        // 非位移状态下的悬浮
        if (isPhaseTwoActive) {
            // 二阶段：在当前位置悬浮
            y = targetY + hoverOffset;
        }
        else {
            // 一阶段：固定高度悬浮
            y = 200 + hoverOffset;
        }

        // 清空拖尾 (保持不变)
        if (!trails.empty()) trails.erase(trails.begin());
        
    }

    // --- 二阶段复活判定 ---
    // 条件：Boss是隐藏的 + 处于攀爬阶段 + 二阶段还没正式激活 + 玩家爬得够高了
    // 假设你的最后一个台阶大概在 y = -300 左右，那玩家跳到 y < -400 就触发
    if (!active && PhaseTwo && !isPhaseTwoActive && player.y < -200) {
        active = true;           // 1. 显示 Boss
        isPhaseTwoActive = true; // 2. 标记二阶段战斗正式开始
        hp = 500;                // 3. 重置血量 (随你定)
        x = WINDOW_W / 2;        // 4. 初始位置：屏幕中间
        y = -600;                // 5. 初始高度：在玩家头顶出现
        alpha = 0.0f;            // 6. 重新开始淡入动画

        // 这里可以重置一下瞬移状态，防止刚出来就乱飞
        isTeleporting = false;
        phaseTwoTargetIndex = -1;
    }

    // --- 死亡转场逻辑 ---
    if (hp <= 0 && active) {
        // 1. Boss 消失
        active = false;

        PhaseTwo = true;

        // 2. 停止所有攻击状态
        isFinalPhase = false;
        orbAttackActive = false;
        swordAttackActive = false;
        burstAttackActive = false;
        laserAttackActive = false;

        // 3. 清理地刺 (重要！否则玩家没法走)
        currentSpikeState = SPIKE_HIDDEN;

        // 4. 触发阶梯生成 (我们需要在 world.h 里声明这个函数)
        GenerateStairs();

        // 5. 可以在这里播放一个震屏或者音效，增加仪式感
        // PlaySound(...);
        return;
    }
    
}

void Boss::InitSunCache() {
    // 创建一个足够大的画布 (比如 200x200)
    sunCache.Resize(300, 300);

    // 切换绘图目标到这张图片上
    SetWorkingImage(&sunCache);

    // 把你原来的太阳绘制逻辑搬过来，坐标改为图片中心 (100, 100)
    int mid = 150;
    for (int i = 0; i < 90; i++) {
        // 注意：这里先不要 Fade，alpha 留到贴图时整体处理
        setfillcolor(RGB(255, 192 - i, 107 - i));
        solidellipse(mid - 125 + i, mid - 125 + i, mid + 125 - i, mid + 125 - i);
    }

    // 恢复绘图目标为屏幕
    SetWorkingImage(NULL);
}

void Boss::InitHitCache() {
	hitCache.Resize(300, 300);
	SetWorkingImage(&hitCache);
	int mid = 150;
	for (int i = 0; i < 45; i++) {
		setfillcolor(RGB(255, 255 - 2 * i, 255 - 2 * i));
		solidellipse(mid - 125 + i, mid - 125 + i, mid + 125 - i, mid + 125 - i);
	}
	SetWorkingImage(NULL);
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
                solidcircle(tx - cameraX, ty - cameraY, 110 - k);
        }
    }


    int cx = (int)x;
    int cy = (int)y;
    int r = 185;
    // 1. 旋转背景

    for (int j = 0; j < 3; j++) {
        setlinecolor(Fade(RGB(240, 240, 240),alpha));
        setlinestyle(PS_SOLID, 5);
        setfillcolor(NULL);
        circle(cx - cameraX, cy - cameraY, 125 + 30 * j);
    }

    for (int i = 0; i < 8; i++) {
           
        float angle = i * 3.1415f / 4.0f + (GetTickCount() / 1000.0f); //45度的间隔,随时间旋转(通过时间偏移实现)
        int ex = cx + (int)(cos(angle) * r);
        int ey = cy + (int)(sin(angle) * r);
        
        setlinecolor(Fade(RGB(255, 255, 220),alpha));
        setlinestyle(PS_SOLID, 11+ (rand() % 3));
        line(cx - cameraX, cy - cameraY, ex - cameraX, ey - cameraY);
        
        setlinecolor(Fade(RGB(204, 218, 221),alpha));
        setlinestyle(PS_SOLID, 9);
        line(cx - cameraX, cy - cameraY, ex - cameraX, ey - cameraY);
        
        // --- 末端尖刺逻辑 ---
        float spikeLen = 45.0f;    // 尖刺从末端往外延伸的长度
        float spikeWidth = 18.0f;  // 尖刺底部的宽度（一半）
        float baseR = r - 10.0f;   // 尖刺底座的位置（稍微缩进光束一点，衔接更自然）

        POINT pts[3];
        // 尖刺顶点：在 angle 方向延伸
        pts[0].x = cx - cameraX + (int)((baseR + spikeLen) * cos(angle));
        pts[0].y = cy - cameraY + (int)((baseR + spikeLen) * sin(angle));

        // 尖刺底边左角：利用垂直向量 (-sin, cos) 进行偏移
        pts[1].x = cx - cameraX + (int)(baseR * cos(angle) - spikeWidth * sin(angle));
        pts[1].y = cy - cameraY + (int)(baseR * sin(angle) + spikeWidth * cos(angle));

        // 尖刺底边右角：利用垂直向量 (sin, -cos) 进行偏移
        pts[2].x = cx - cameraX + (int)(baseR * cos(angle) + spikeWidth * sin(angle));
        pts[2].y = cy - cameraY + (int)(baseR * sin(angle) - spikeWidth * cos(angle));

        // 填充颜色：使用稍微亮一点的米白色，制造立体感
        setfillcolor(Fade(RGB(255, 255, 240),alpha));
        solidpolygon(pts, 3);

    }

    // SRCPAINT参数会将太阳图片和原有的主体进行了位运算
	// 会去掉黑色背景，同时把太阳的亮色部分叠加上去
	// 造成某种视觉上的发光效果，透明感，能量体的感觉
    putimage(x-150 - cameraX, y-150 - cameraY, &sunCache,SRCPAINT);

    if (boss.boss_is_invincible){
		putimage(x - 150 - cameraX, y - 150 - cameraY, &hitCache, SRCPAINT);
    }
    

    setfillcolor(NULL);
	setlinecolor(Fade(RGB(255, 200, 17),alpha));
	setlinestyle(PS_SOLID, 3);
	circle(cx - cameraX, cy - cameraY, 90 + (rand() % 3)); // 微微抖动的边框

}

Rect Boss::getRect() {
    return { x - 125,y - 125, 250,250 };
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
    rectangle((int)r.x - cameraX, (int)r.y - cameraY, (int)(r.x - cameraX + r.w), (int)(r.y - cameraY + r.h));
    
    settextcolor(WHITE);
    settextstyle(20, 0, _T("Consolas"));
    TCHAR s[50];
    _stprintf_s(s, _T("BOSS HP: %d"), boss.hp);
    outtextxy(10, 100, s);
}

void Boss::SpawnSwordWallHorizontal(bool fromLeft) {
    //printf("横剑\n");
	// 先定义一下画横剑的参数,startX,vx, angle,这都是好直接确定的
    float startX = fromLeft ? -280.0f : WINDOW_W + 280.0f; // 从左还是从右，初始坐标
    float vx = fromLeft ? 20.0f : -20.0f; // 对应的速度方向
    float angle = fromLeft ? 0.0f : 3.14159f;

    // 现在我们要算y的分布了
    int totalSwords = 16; // 定义多少把剑
    int spacing = 70;     // 剑之间的间隔多大,注意我们的主角是40格宽,65格高的

    int gapA = rand() % (totalSwords * 1 / 8) + 1 + (rand() % 3 - 1); //2到4随机一个
	int gapB = rand() % (totalSwords * 3 / 8) + (rand() % 5 - 1); // 4到8随机一个
	int gapC = rand() % (totalSwords * 3 / 8) + (rand() % 5 - 1); // 4到8随机一个
	int gapD = rand() % (totalSwords * 5 / 8) + (rand() % 5 - 1); // 8到12随机一个

    int randomOffset = (rand() % 50) - 30; // 在 Y 轴方向整体上下浮动 -25 到 +25 像素

    float baseY = isPhaseTwoActive ? this->y : PLATFORM_Y;
    if (isPhaseTwoActive) baseY += 100;

    for (int i = 0; i < totalSwords; i++) {
        if ((i == gapA || i == gapB || i == gapC || i == gapD)) continue;
        // 分布在Y轴上，覆盖玩家可能跳跃的高度
        float py = baseY - (totalSwords * spacing / 2.0f) + (i * spacing) + randomOffset;
        projectiles.push_back(new Sword(startX, py, vx, 0, angle, false));
    }
}

void Boss::SpawnSwordWallVertical() {
    //printf("竖剑\n");
	int totalSwords = 24; // 定义多少把剑
	float spacing = 75.0f; // 剑之间的间隔多大

    int gapA = rand() % (totalSwords * 1 / 8) + (rand() % 3 -1);
    int gapB = rand() % (totalSwords * 3/ 8) + (rand() % 5 - 1);
    int gapC = rand() % (totalSwords * 3 / 8) + (rand() % 5 - 1);
    int gapD = rand() % (totalSwords * 5 / 8) + (rand() % 5 - 1);
    int gapE = rand() % (totalSwords * 5 / 8) + (rand() % 5 - 1);
    int gapF = rand() % (totalSwords * 7 / 8) + (rand() % 3 - 1);
    
    int randomOffset = (rand() % 60) - 30;

    float startXBase = PLATFORM_X - 300;

    for (int i = 0; i < 25; i++) {
        if ((i == gapA || i == gapB || i == gapC || i == gapD || i == gapE || i == gapF)) continue; // gap跳过
        float px = startXBase + randomOffset + (i * spacing);
        projectiles.push_back(new Sword(px, -100.0f, 0, 20.0f, 1.5708f, false));
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
        oy = (PLATFORM_Y - 300) - sin(angle) * 100;
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
   /* float dx = (player.x + player.w / 2) - x;
    float dy = (player.y + player.h / 2) - y;
    baseRotation = atan2(dy, dx);*/ 

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
        if (swordAttackType == 1) {
            if (swordAttackCount < SWORD_ATTACK_TOTAL && currentTime - swordAttackLastTime > SWORD_ATTACK_INTERVAL_HORIZON) {
                swordAttackLastTime = currentTime;
                swordAttackCount++;
                SpawnSwordWallHorizontal(swordAttackFromLeft);
			}
		}
		if (swordAttackType == 2) {
			if (swordAttackCount < SWORD_ATTACK_TOTAL && currentTime - swordAttackLastTime > SWORD_ATTACK_INTERVAL_VERTICAL) {
				swordAttackLastTime = currentTime;
				swordAttackCount++;
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
        if (elapsed > 500 && laserWaveCount < 3) {
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

    // =========================================================
    //  第二部分：大脑决策 (决定下一步干什么)
    // =========================================================

    // --- 如果是二阶段，决策权交给 PhaseTwoAI ---
    if (isPhaseTwoActive) {
        PhaseTwoAI(); // 委托给二阶段 AI 处理
        return;       // 只要在二阶段，就不跑后面的一阶段逻辑
    }

    // --- 攀爬阶段休息 ---
    if (PhaseTwo) {return;}

    // --- 三阶段触发判定 ---
    if (hp <= 200 && !isFinalPhase) {
        isFinalPhase = true;

        // 1. 清空所有正在进行的旧攻击状态
        orbAttackActive = false;
        swordAttackActive = false;
        burstAttackActive = false;
        laserAttackActive = false;

        // 2. 强行回归中心点
        targetX = WINDOW_W / 2.0f;
        isTeleporting = true;

        // 3. 强行修改地刺逻辑 (我们可以直接修改全局变量)
        currentSpikeState = SPIKE_ACTIVE;
        // 这里我们可以稍微改一下 SpikeManager，让它支持“全屏两边刺”
        return;
    }

    // --- 三阶段疯狂模式逻辑 ---
    if (isFinalPhase) {
        // 锁定位置：防止位移结束后再次触发随机位移
        if (!isTeleporting) x = WINDOW_W / 2.0f;

        // 持续使用垂直剑雨，缩短间隔
        static DWORD lastFinalSwordTime = 0;
        if (currentTime - lastFinalSwordTime > 900) { // 0.8秒一波，非常密集
            SpawnSwordWallVertical();
            lastFinalSwordTime = currentTime;
        }
        return; // 拦截后续的老 AI 逻辑
    }

    // 基础攻击间隔
    if (currentTime - lastAttackTime > 1500) {
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

void Boss::PhaseTwoAI() {
    DWORD now = GetTickCount();

    // 0. 状态检查：如果在瞬移或攻击中，啥都别干
    if (isTeleporting || orbAttackActive || swordAttackActive || burstAttackActive || laserAttackActive) {
        return;
    }

    // 1. 瞬移决策逻辑
    if (phaseTwoTargetIndex == -1) {
        // 随机选一个新位置
        int newIndex = rand() % phaseTwoAnchors.size();

        // 简单的防重复：如果跟上次一样，就取下一个
        if (newIndex == phaseTwoTargetIndex) {
            newIndex = (newIndex + 1) % phaseTwoAnchors.size();
        }
        phaseTwoTargetIndex = newIndex;

        // 启动瞬移
        isTeleporting = true;
        teleportStartTime = now;

        // 设置目标 (更新 targetX 和 targetY)
        targetX = phaseTwoAnchors[newIndex].x;
        targetY = phaseTwoAnchors[newIndex].y;

        // 决定到了之后打几次 (1次或2次)
        phaseTwoAttackCount = 1 + rand() % 2;
        return;
    }

    // 2. 攻击决策逻辑
    if (phaseTwoAttackCount > 0) {
        // 瞬移到位后，发呆 1 秒再攻击，给玩家反应时间
        static DWORD waitTimer = 0;
        if (now - waitTimer < 1000) return; // 等待中...

        // 随机选招式 (去掉了不适合高空的纵向剑雨)
        int attackType = rand() % 3;

        if (attackType == 0) { // 光球
            orbAttackActive = true;
            orbAttackCount = 0;
            orbAttackLastTime = now;
        }
        else if (attackType == 1) { // 横向飞剑 (需要修改生成函数支持高度！)
            swordAttackActive = true;
            swordAttackType = 1; // 标记为横向
            swordAttackCount = 0;
            swordAttackLastTime = now;
            swordAttackFromLeft = rand() % 2 == 0;
        }
        else if (attackType == 2) { // 脸刺 (Burst)
            burstAttackActive = true;
            burstWaveCount = 0;
            lastBurstTime = now;
        }

        phaseTwoAttackCount--; // 消耗一次攻击次数
        waitTimer = now;       // 重置等待计时
    }
    else {
        // 3. 次数用尽 -> 准备下一次瞬移
        phaseTwoTargetIndex = -1; // 设置为 -1 会在下一帧触发上面的瞬移逻辑
    }
}