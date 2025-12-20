#include"world.h"
#include"player.h"
#include"boss.h"

std::vector<Rect> platforms; // 定义

void make_one_stair(int X,int Y,int W,int H) {
    Rect platform;
    platform.x = X;
    platform.y = Y;
    platform.w = W;
    platform.h = H;
    platforms.push_back(platform);
}

void GenerateStairs() {

    float startX = WINDOW_W / 2 ;     // 屏幕水平中心
    float currentY = PLATFORM_Y - 250; // 第一级台阶的高度

    make_one_stair(681, 450, 150, 50); // 第一级台阶

    //后面的都是二阶段用的平台
    make_one_stair(500, 200, 175, 50);
    make_one_stair(120, 50, 175, 50);
    make_one_stair(800, 0, 100, 50);
    make_one_stair(480, -300, 175, 50);
    make_one_stair(0, -200, 100, 50);
    make_one_stair(980, -280, 100, 50);
}

void GenerateUPStairs() {
    // --- 承接二阶段顶部的过渡平台 ---
    // 二阶段最高的台阶在 (480, -300)，我们从这里开始往上带
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

    // --- 最终决战平台 ---
    make_one_stair(401, -4550, 100, 50);
    make_one_stair(801, -4550, 100, 50);
}

void InitPlatform(){
    platforms.clear();
    platforms.push_back({ (float)PLATFORM_X, (float)PLATFORM_Y, (float)PLATFORM_W, 500 });
}

void DrawSpikes(float x, float y, float w, float h) {
    // --- 配色方案 (与白色光剑保持一致) ---
    COLORREF colorGlow = RGB(255, 120, 20);  // 最外层橙色辉光
    COLORREF colorHalo = RGB(255, 230, 150); // 中层淡黄晕影
    COLORREF colorCore = WHITE;              // 纯白核心

    // 计算地刺分布：确保左右两端都有地刺，消除安全盲区
    int numSpikes = (int)(w / 42) + 1;       // 根据宽度计算需要的数量
    float margin = 15.0f;                    // 左右预留微调边距
    float step = (w - 2 * margin) / (numSpikes - 1); // 动态计算间距

    for (int i = 0; i < numSpikes; i++) {
        float cx = x + margin + i * step;    // 每一枚地刺的中心 X 坐标
        float bot = y + h;
        float top = bot - 40;                // 刺的高度

        // 2. 绘制纯白实体
        setlinestyle(PS_SOLID, 1);
        setfillcolor(colorCore);
        setlinecolor(colorCore);

        // 倒三角形剑身 (底在下，尖在上)
        POINT pts[] = {
            {(int)cx - 7 - cameraX, (int)bot - cameraY},
            {(int)cx + 7 - cameraX, (int)bot - cameraY},
            {(int)cx - cameraX, (int)top - cameraY}
        };
        solidpolygon(pts, 3);

        // 倒置的翼状剑格 (增加压迫感)
        POINT guard[] = {
            {(int)cx - cameraX, (int)bot - 12 - cameraY},      // 下拉中心点
            {(int)cx - 16 - cameraX, (int)bot - 2 - cameraY},  // 左翼
            {(int)cx + 16 - cameraX, (int)bot - 2 - cameraY}   // 右翼
        };
        solidpolygon(guard, 3);

        // 核心亮线
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, 2);
        line((int)cx - cameraX, (int)bot - 2 - cameraY, (int)cx - cameraX, (int)top + 8 - cameraY);
    }
    setlinestyle(PS_SOLID, 1); // 还原线型
}

extern Player player;
extern Boss boss;

// 绘制UI
void DrawUI() {
    int maxHp = 10;         // 硬编码上限为 10
    int maskSize = 18;
    int startX = 60;        // 稍微往右挪一点，防止太靠边
    int startY = 60;
    int spacing = 45;

    for (int i = 0; i < maxHp; i++) {
        int cx = startX + i * spacing;
        int cy = startY;

        if (i < player.hp) {
            // --- 1. 绘制完整的白色面具 (当前血量) ---
            setfillcolor(WHITE);
            setlinecolor(WHITE);
            // 绘制略带棱角的面具外形
            solidellipse(cx - 14, cy - 18, cx + 14, cy + 18);

            // --- 2. 绘制黑色眼珠 ---
            setfillcolor(BLACK);
            // 稍微倾斜的眼部，更具神韵
            solidellipse(cx - 8, cy - 5, cx - 2, cy + 5);
            solidellipse(cx + 2, cy - 5, cx + 8, cy + 5);
        }
        else {
            // --- 3. 绘制“空面具”/破碎感 (血量上限但已扣除) ---
            // 使用暗灰蓝色，模拟空壳感
            setlinecolor(RGB(40, 40, 60));
            setlinestyle(PS_SOLID, 2);
            // 只画出淡淡的轮廓
            ellipse(cx - 12 , cy - 15, cx + 12, cy + 15);
        }
    }

    if (player.hp <= 0) {
        settextcolor(WHITE);
        settextstyle(60, 0, _T("Consolas"));
        outtextxy(WINDOW_W / 2 - 150, WINDOW_H / 2 - 30, _T("YOU DIED"));
    }
    setlinestyle(PS_SOLID, 1);

}

// 辅助函数：专门画一个单独的平台块 (把原来 DrawPlatform 的逻辑搬进来了)
void DrawSinglePlatform(const Rect& r) {
    int radius = 15;

    // 1. 底板
    setfillcolor(RGB(60, 70, 90));
    solidroundrect((int)r.x - cameraX, (int)r.y - cameraY, (int)(r.x + r.w - cameraX), (int)(r.y + r.h - cameraY), radius, radius);

    // 2. 主体
    setfillcolor(RGB(20, 24, 35));
    solidroundrect((int)r.x - cameraX, (int)r.y + 4 - cameraY, (int)(r.x + r.w - cameraX), (int)(r.y + r.h - cameraY), radius, radius);

    // 3. 纹理绘制 (关键修复点！)
    if (r.w * r.h > 200) { // 太小的台阶直接不画纹理，省事又安全

        int dotCount = (int)(r.w * r.h / 400);
        if (dotCount < 3) dotCount = 3;
        if (dotCount > 50) dotCount = 50;

        int padding = 5; // 强制使用较小的 padding

        // --- 安全计算范围 ---
        // 这里的 rangeX 和 rangeY 必须保证大于 0
        int rangeX = (int)r.w - 2 * padding;
        int rangeY = (int)r.h - 8 - padding; // 针对高度特别小的台阶

        if (rangeX > 0 && rangeY > 0) { // 【绝对防御】只有范围合法才画点
            for (int i = 0; i < dotCount; i++) {
                int rx = (int)r.x + padding + rand() % rangeX;
                int ry = (int)r.y + 6 + rand() % rangeY;

                int type = rand() % 3;
                if (type == 0) {
                    setfillcolor(RGB(70, 80, 100));
                    solidcircle(rx - cameraX, ry - cameraY, 1);
                }
                else if (type == 1) {
                    setfillcolor(RGB(10, 12, 18));
                    solidcircle(rx - cameraX, ry - cameraY, 1);
                }
            }
        }
    }

    // 4. 裂痕 (只在大台阶画)
    if (r.w > 100 && r.h > 40) { // 加个高度限制，防止在薄台阶上画出界
        setlinecolor(RGB(45, 50, 70));
        for (int i = 0; i < 3; i++) {
            int sx = (int)r.x + 5 + rand() % ((int)r.w - 10);
            int sy = (int)r.y + 10 + rand() % ((int)r.h - 20);
            int len = 10 + rand() % 20;
            line(sx - cameraX, sy - cameraY, sx - cameraX + len, sy - cameraY + (rand() % 3 - 1));
        }
    }

    // 5. 描边
    setlinecolor(RGB(40, 50, 70));
    setlinestyle(PS_SOLID, 1);
    setfillstyle(BS_NULL);
    roundrect((int)r.x - cameraX, (int)r.y + 4 - cameraY, (int)(r.x + r.w - cameraX), (int)(r.y + r.h - cameraY), radius, radius);
    setfillstyle(BS_SOLID);
}

void DrawPlatform() {
    for (const auto& p : platforms) {
        DrawSinglePlatform(p);
    }
}

void LastSpike() {
    // --- 新增：三阶段（决战模式）的特殊处理 ---

        // 1. 确定两边地刺的范围（左右各占 1/4）
        float sideWidth = PLATFORM_W / 4.0f;

        Rect leftSpikeRect = { (float)PLATFORM_X, (float)PLATFORM_Y - 5, sideWidth, 10 };
        Rect rightSpikeRect = { (float)PLATFORM_X + PLATFORM_W * 0.75f, (float)PLATFORM_Y - 5, sideWidth, 10 };

        // 2. 绘制地刺（两边同时画）
        DrawSpikes(leftSpikeRect.x, leftSpikeRect.y, leftSpikeRect.w, leftSpikeRect.h);
        DrawSpikes(rightSpikeRect.x, rightSpikeRect.y, rightSpikeRect.w, rightSpikeRect.h);

        // 3. 伤害判定（两边都要检测）
        if (!player.isInvincible) {
            Rect pRect = player.getHitbox();
            pRect.x += 10; pRect.w -= 20; // 缩小判定，更公平

            // 检测左边或右边是否撞刺
            if (pRect.checkCollision(leftSpikeRect) || pRect.checkCollision(rightSpikeRect)) {
                if (pRect.y + pRect.h > PLATFORM_Y - 25) {
                    player.hp--;
                    player.isInvincible = true;
                    player.vy = -8.0f;
                }
            }
        }

        // 4. 下劈弹起判定（可选，让玩家能在两边的刺上“跳跳乐”）
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
    
    // 1. 状态机更新逻辑
    if (currentSpikeState == SPIKE_HIDDEN) {
		// boss 血量低于 650 -> 进入预警状态
        if (boss.hp < 650 && !boss.isPhaseTransition) {
            currentSpikeState = SPIKE_WARNING;
            spikeTimer = now;
            spikeOnLeft = true; // 第一次总是出现在左边
        }
    }
    else if (currentSpikeState == SPIKE_WARNING) {
        // 预警时间结束 -> 切换到激活状态
        if (now - spikeTimer > DURATION_WARNING) {
            currentSpikeState = SPIKE_ACTIVE;
            spikeTimer = now;
        }
    }
    else if (currentSpikeState == SPIKE_ACTIVE) {
        // 激活时间结束 -> 切换方向并进入预警
        if (now - spikeTimer > DURATION_ACTIVE) {
            currentSpikeState = SPIKE_WARNING; // 再次进入预警
            spikeTimer = now;
            spikeOnLeft = !spikeOnLeft; // 换边
        }
    }

    // 2. 确定当前地刺的区域
    Rect currentSpikeRect = { 0, 0, 0, 0 };
    int spikeY = PLATFORM_Y - 5;
    if (spikeOnLeft) {
        currentSpikeRect = { (float)PLATFORM_X, (float)spikeY, (float)PLATFORM_W / 2.0f, 10 };
    }
    else {
        currentSpikeRect = { (float)PLATFORM_X + PLATFORM_W / 2.0f, (float)spikeY, (float)PLATFORM_W / 2.0f, 10 };
    }

    // 3. 表现与判定逻辑
    if (currentSpikeState == SPIKE_WARNING) {
        // --- 预警阶段：闪烁淡黄色方块 ---
        // 利用时间取余实现快速闪烁效果 (200ms周期)
        if ((now / 100) % 2 == 0) {
            setfillcolor(RGB(255, 255, 200)); // 亮黄
        }
        else {
            setfillcolor(RGB(100, 100, 0));   // 暗黄 (产生闪烁感)
        }
        // 绘制半透明提示区 (实心矩形模拟)
        solidrectangle((int)currentSpikeRect.x - cameraX, PLATFORM_Y - 5 - cameraY,
            (int)(currentSpikeRect.x - cameraX + currentSpikeRect.w), PLATFORM_Y - cameraY);

        // 预警阶段没有任何伤害判定
    }
    else if (currentSpikeState == SPIKE_ACTIVE) {
        // --- 激活阶段：绘制白色尖刺 ---
        DrawSpikes(currentSpikeRect.x, currentSpikeRect.y, currentSpikeRect.w, currentSpikeRect.h);

        // --- 伤害与下劈判定 (只在激活时生效) ---

        // A. 下劈弹起
        if (player.isAttacking && player.attackDir == 2) {
            // 判定攻击框是否碰到地刺区域
            if (player.attackBox.checkCollision(currentSpikeRect)) {
                // 确保是在刺的上方
                if (player.y + player.h <= PLATFORM_Y + 10) {
                    player.vy = -9.0f;    // 向上弹起
                    player.jumpCount = 1;  // 刷新二段跳
                    player.hasDashedInAir = false;
                }
            }
        }
        // B. 受伤判定
        if (!player.isInvincible) {
            // 稍微缩小一点玩家的判定框，因为刺是三角形的，判定太宽会觉得假
            Rect pRect = player.getHitbox();
            pRect.x += 10; pRect.w -= 20;

            if (pRect.checkCollision(currentSpikeRect)) {
                // 只有当玩家脚部接触到刺的高度时才受伤
                if (pRect.y + pRect.h > PLATFORM_Y - 25) {
                    player.hp--;
                    player.isInvincible = true;
                    player.vy = -8.0f; // 受伤小弹起
                }
            }
        }
    }
};

