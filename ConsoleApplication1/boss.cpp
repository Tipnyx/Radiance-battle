# include"boss.h"
# include"projectile.h"
# include"player.h"
# include"world.h"
# include"BossState.h"

extern Player player;
extern Boss boss;
extern std::vector<Projectile*> projectiles;

// 将颜色根据 alpha 进行亮度缩放，模拟渐显
COLORREF Fade(COLORREF c, float a) {
    return RGB(GetRValue(c) * a, GetGValue(c) * a, GetBValue(c) * a);
}

Boss::Boss() {
        x = WINDOW_W / 2;
        y = -100; // 悬浮在空中
        currentState = new PhaseOneState(); // 初始状态
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
    
    UpdateAttacks();
    UpdateStateMachine();
}

void Boss::InitSunCache() {
    // 创建一个足够大的画布 (300x300)
    // 注意，原点在画布中心，而不是(0,0）点
    sunCache.Resize(300, 300);

    // 切换绘图目标到这张图片上
    SetWorkingImage(&sunCache);

    // 画布中心
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

    putimage(x - 150 - cameraX, y - 150 - cameraY, &sunCache, SRCPAINT);

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

    for (int i = 0; i < totalSwords; i++) {
        if ((i == gapA || i == gapB || i == gapC || i == gapD)) continue;
        // 分布在Y轴上，覆盖玩家可能跳跃的高度
        float py = currentLevelBottom - (i * spacing) + randomOffset;
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
        oy = (currentLevelBottom - 450) - sin(angle) * 100;
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

void Boss::ChangeState(BossState* newState) {
    if (currentState) {
        currentState->Exit(*this);
        delete currentState; 
    }
	currentState = newState;
    if (currentState) {
        currentState->Enter(*this);
    }
}

// 在 Boss::update 里调用这个
void Boss::UpdateStateMachine() {
    if (currentState) {
        currentState->Execute(*this);
    }
}

// Boss 具体的攻击逻辑，根据BOSS状态里传出的信号执行对应的动作
void Boss::UpdateAttacks(){

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
}