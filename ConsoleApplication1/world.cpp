#include"world.h"
#include"player.h"
#include"boss.h"

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

void DrawPlatform() {
    // --- 画平台逻辑 (圆角黑曜石风格) ---
    
    int r = 15; // 圆角半径 (控制圆滑程度，值越大越圆)

    // 1. 绘制高光底板 (Highlight Layer)
        // 这一层画在下面，颜色稍亮。因为上面的层会向下偏移，所以这层的顶部会露出来形成高光倒角。
    setfillcolor(RGB(60, 70, 90)); // 亮蓝灰色
    solidroundrect(PLATFORM_X - cameraX, PLATFORM_Y - cameraY, PLATFORM_X + PLATFORM_W - cameraX, PLATFORM_Y + PLATFORM_H - cameraY, r, r);

    // 2. 绘制主体层 (Body Layer)
    // 这一层颜色深，向下偏移 4 像素，覆盖住底板的下半部分
    setfillcolor(RGB(20, 24, 35)); // 深邃暗色
    // 注意：y 坐标 +4，但高度保持一致，这样底部也会覆盖住底板（因为底板也是圆角，重叠起来刚好）
    solidroundrect(PLATFORM_X - cameraX, PLATFORM_Y + 4 - cameraY, PLATFORM_X - cameraX + PLATFORM_W, PLATFORM_Y - cameraY + PLATFORM_H, r, r);

    int padding = 10; // 纹理向内缩进，防止画到圆角外面

    for (int i = 0; i < 300; i++) {
        int rx = PLATFORM_X + padding + rand() % (PLATFORM_W - 2 * padding);
        int ry = PLATFORM_Y + 6 + rand() % (PLATFORM_H - 8 - padding); // 从高光下方开始

        int type = rand() % 3;
        if (type == 0) {
            // 亮斑
            setfillcolor(RGB(70, 80, 100));
            solidcircle(rx - cameraX, ry - cameraY, 1); // 改用圆点，更细腻
        }
        else if (type == 1) {
            // 暗坑
            setfillcolor(RGB(10, 12, 18));
            solidcircle(rx - cameraX, ry - cameraY, 1);
        }
    }

    // 4. 添加裂痕 (范围限制)
    setlinecolor(RGB(45, 50, 70));
    for (int i = 0; i < 12; i++) {
        int sx = PLATFORM_X + padding + rand() % (PLATFORM_W - 2 * padding);
        int sy = PLATFORM_Y + 10 + rand() % (PLATFORM_H - 20);
        int len = 10 + rand() % 30;
        line(sx - cameraX, sy - cameraY, sx - cameraX + len, sy - cameraY + (rand() % 3 - 1));
    }

    // 5. 边缘描边
    setlinecolor(RGB(40, 50, 70)); // 很淡的描边
    setlinestyle(PS_SOLID, 1);
    //setfillcolor(NULL); // 不填充，只画框
    roundrect(PLATFORM_X - cameraX, PLATFORM_Y + 4 - cameraY, PLATFORM_X + PLATFORM_W - cameraX, PLATFORM_Y + PLATFORM_H - cameraY, r, r);
    setfillstyle(BS_SOLID);
}

void SpikeManager(DWORD gameStartTime) {
    DWORD now = GetTickCount();

    // 1. 状态机更新逻辑
    if (currentSpikeState == SPIKE_HIDDEN) {
		// boss 血量低于 650 -> 进入预警状态
        if (boss.hp < 650) {
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

