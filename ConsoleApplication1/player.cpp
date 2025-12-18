# include"common.h"
# include"player.h"

Player::Player() {
    reset();
}

void Player::reset() {
    x = WINDOW_W / 2;
    y = PLATFORM_Y - h - 100;
    vx = 0; vy = 0;
    hp = 10;
    jumpCount = 0;
}

void Player::update() {
    // 获取当前时间
    DWORD currentTime = GetTickCount();

    // 1. 更新受伤无敌计时器
    if (hurtTimer > 0) {
        hurtTimer--;
    }

    // 状态恢复逻辑
    if (!canShadowDash && currentTime - lastShadowDashTime > SHADOW_DASH_COOLDOWN) {
        canShadowDash = true;
    }

    // 更新残影寿命 (无论是否在冲刺都要更新)
    for (int i = 0; i < ghosts.size(); i++) {
        ghosts[i].life -= 15; // 每一帧变淡
        if (ghosts[i].life <= 0) {
            ghosts.erase(ghosts.begin() + i);
            i--;
        }
    }

    // 如果正在冲刺
    if (isDashing) {
        // 不在冲刺时间内了
        if (currentTime - dashStartTime > DASH_DURATION) {
            isDashing = false; // 停止冲刺
            isShadowDash = false; // 结束暗影状态
            vx = 0; // 水平速度归零 
            // Optional：冲刺结束，可以稍微保留一点惯性或直接停止
        }
        else {
            vx = facing * DASH_SPEED; // 冲刺期间强制位移
            vy = 0; // 冲刺期间不受重力
            isInvincible = isShadowDash || (hurtTimer > 0); // 冲刺期间无敌
            x += vx;
            if (currentTime - lastGhostTime > 30) {
                // 初始生命值给满 (255)，让它存在感更强
                ghosts.push_back({ x, y, 255 });
                lastGhostTime = currentTime;
            }
            return; // 冲刺时不处理常规移动
        }
    }
    else {
        isInvincible = false; // 非冲刺期间取消无敌
        // 冲刺冷却检查
        if (hurtTimer > 0) {
            isInvincible = true;
        }
        else {
            isInvincible = false;
        }
        if (!canDash && currentTime - lastNormalDashTime > NORMAL_DASH_COOLDOWN) {
            canDash = true;
        }
    }

    // 常规移动输入
    if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT)) {
        vx = -MOVE_SPEED;
        facing = -1;
    }
    else if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) {
        vx = MOVE_SPEED;
        facing = 1;
    }
    else {
        vx = 0;
    }

    if (GetAsyncKeyState('P') & 1) { // 使用 &1 确保只在按下瞬间触发一次切换
        debug_mode = !debug_mode;
    }

    // 跳跃
    bool jumpKey = (GetAsyncKeyState('Z') & 0x8000) != 0;

    // 只在按下瞬间触发跳跃
    if (jumpKey && !lastJumpKey && jumpCount < MAX_JUMP) {
        vy = JUMP_FORCE;
        onGround = false;
        jumpCount++;
    }
    lastJumpKey = jumpKey;

    // 冲刺触发
    if (GetAsyncKeyState('C')) {
        bool canNormal = (currentTime - lastNormalDashTime > NORMAL_DASH_COOLDOWN);

        if (!isDashing && !isAttacking && canNormal) {
            isDashing = true;
            dashStartTime = currentTime;
            lastNormalDashTime = currentTime;

            if (canShadowDash) {
                // 触发暗影冲刺
                isShadowDash = true;
                canShadowDash = false;
                lastShadowDashTime = currentTime;
            }
            else {
                // 触发普通冲刺
                isShadowDash = false;
                isInvincible = false;
            }
        }
    }

    // 攻击触发
    if ((GetAsyncKeyState('X') || GetAsyncKeyState('J')) && !isAttacking) {
        isAttacking = true;
        attackStartTime = currentTime;
        atkTimer = atkDuration; // 重置计时器为最大值

        // 判定攻击方向
        if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W')) attackDir = 1;
        else if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S')) attackDir = 2;
        else attackDir = 0;
    }

    // 攻击持续逻辑
    if (isAttacking) {
        atkTimer--;
        if (atkTimer <= 0) {
            isAttacking = false;
        }
        else {
            // --- 关键修改：判定范围设定为人物高度的 1.5 倍 ---
            float atkLen = h * 2.5f;

            if (attackDir == 1) { // 向上
                attackBox = { x + 0.1f * w, y - atkLen, (float)w * 0.8f, atkLen };
            }
            else if (attackDir == 2) { // 向下
                attackBox = { x + 0.1f * w, y + h, (float)w * 0.8f, atkLen };
            }
            else { // 侧向（横斩）
                // 横斩长度也同步为 1.5 倍高度
                attackBox = { facing == 1 ? x + w : x - atkLen, y - h * 0.25f, atkLen, h * 1.5f };
            }
        }
    }

    // 物理应用
    vy += GRAVITY;
    x += vx;
    y += vy;

    // 平台碰撞
    onGround = false;
    // 只有下落时检测平台
    if (vy > 0 && x + w > PLATFORM_X && x < PLATFORM_X + PLATFORM_W) {
        if (y + h >= PLATFORM_Y && y + h <= PLATFORM_Y + 20) {
            y = PLATFORM_Y - h;
            vy = 0;
            onGround = true;
            jumpCount = 0;
        }
    }

    // 掉出屏幕重置 (虚空伤害)
    if (y > WINDOW_H) {
        hp--;
        reset();
        y = 0; // 从天而降
    }
}

void Player::draw() {

    // --- 1. 绘制残影 (修复黑白混杂问题) ---
    for (const auto& g : ghosts) {
        float alpha = g.life / 255.0f;
        COLORREF c;

        // 根据初始生命值判定类型：255 为暗影，100 为普通
        // 注意：不要在绘制时根据实时 life 判定，否则颜色会随寿命衰减而突变
        if (isShadowDash || (g.life > 100)) {
            // 暗影残影：向背景色靠拢的黑色
            int r = (int)(GetRValue(COLOR_BG) * (1.0f - alpha));
            int g_val = (int)(GetGValue(COLOR_BG) * (1.0f - alpha));
            int b = (int)(GetBValue(COLOR_BG) * (1.0f - alpha));
            c = RGB(r, g_val, b);
        }
        else {
            // 普通冲刺：纯白色渐隐 (向背景色靠拢)
            int r = (int)(255 * alpha + GetRValue(COLOR_BG) * (1.0f - alpha));
            int g_val = (int)(255 * alpha + GetGValue(COLOR_BG) * (1.0f - alpha));
            int b = (int)(255 * alpha + GetBValue(COLOR_BG) * (1.0f - alpha));
            c = RGB(r, g_val, b);
        }

        setfillcolor(c);
        // 绘制稍微缩小的矩形，更有“烟雾”感
        solidrectangle((int)g.x + 3, (int)g.y + 3, (int)(g.x + w - 3), (int)(g.y + h - 3));
    }

    // --- 2. 绘制本体 ---
    if (isShadowDash) {
        setfillcolor(BLACK); // 暗影冲刺时本体完全黑化
    }
    else if (isDashing) {
        setfillcolor(WHITE); // 普通冲刺本体纯白
    }
    else if (hurtTimer > 0 && (hurtTimer / 4) % 2 == 0) {
        // 闪烁成半透明或深色，这里用深红提示受伤
        setfillcolor(RGB(100, 50, 50));
    }
    else {
        setfillcolor(COLOR_KNIGHT);
    }
    solidrectangle((int)x, (int)y, (int)(x + w), (int)(y + h));

    // 绘制暗影冲刺就绪指示器
    if (canShadowDash) {
        setfillcolor(BLACK);
        solidcircle((int)(x + w / 2), (int)(y - 8), 3);
    }


    // --- 完美月牙刀光绘制 ---
    // --- 2. 修正后的抛物线月牙刀光 ---
    if (isAttacking) {
        setfillcolor(WHITE);
        setlinecolor(WHITE);

        // 动画比率：1.0 -> 0.0
        float ratio = (float)atkTimer / atkDuration;

        // 核心参数
        // 攻击距离：固定为人物高度的 1.5 倍
        float reach = h * 2.5f;

        // 刀光两端的开口宽度 (Spread)
        // 侧向攻击时，两端对齐人物高度；纵向攻击时，两端对齐人物宽度
        float baseSpread = (attackDir == 0) ? h : w;
        float halfSpread = baseSpread / 2.0f;

        // 最大厚度 (随时间变薄，产生斩击感)
        float maxThickness = h * 1.1f;

        // 内层抛物线的延伸距离 (越厚则内层越短)
        float innerReach = reach - maxThickness;

        // 角色中心坐标
        int px = (int)(x + w / 2);
        int py = (int)(y + h / 2);

        const int SAMPLE_COUNT = 16;
        POINT pts[SAMPLE_COUNT * 2];

        // 循环生成抛物线点
        // 逻辑变量 t 从 -1.0 到 1.0 (对应开口的两端)
        for (int i = 0; i < SAMPLE_COUNT; i++) {
            float t = -1.0f + 2.0f * (float)i / (SAMPLE_COUNT - 1);

            // 抛物线公式：x = Reach * (1 - t^2)
            // 当 t=0 (中间) 时，x=Reach (最远)
            // 当 t=1/-1 (两端) 时，x=0 (紧贴身体)
            float curveX_Outer = reach * (1 - t * t);
            float curveX_Inner = innerReach * (1 - t * t);
            float curveY = t * halfSpread;

            // 根据攻击方向旋转/映射坐标
            float drawX_Outer, drawY_Outer;
            float drawX_Inner, drawY_Inner;

            if (attackDir == 0) { // 侧向 (Side)
                // X轴为攻击方向，Y轴为扩散方向
                drawX_Outer = px + curveX_Outer * facing;
                drawY_Outer = py + curveY;

                drawX_Inner = px + curveX_Inner * facing;
                drawY_Inner = py + curveY;
            }
            else if (attackDir == 1) { // 向上 (Up)
                // Y轴负方向为攻击方向
                drawX_Outer = px + curveY;
                drawY_Outer = py - curveX_Outer;

                drawX_Inner = px + curveY;
                drawY_Inner = py - curveX_Inner;
            }
            else { // 向下 (Down)
                // Y轴正方向为攻击方向
                drawX_Outer = px + curveY;
                drawY_Outer = py + curveX_Outer;

                drawX_Inner = px + curveY;
                drawY_Inner = py + curveX_Inner;
            }

            // 填充多边形顶点数组
            // 前半部分存外弧
            pts[i].x = (long)drawX_Outer;
            pts[i].y = (long)drawY_Outer;

            // 后半部分逆序存内弧 (闭合形状)
            int innerIndex = (SAMPLE_COUNT * 2 - 1) - i;
            pts[innerIndex].x = (long)drawX_Inner;
            pts[innerIndex].y = (long)drawY_Inner;
        }

        // 绘制
        solidpolygon(pts, SAMPLE_COUNT * 2);
    }
}

Rect Player::getHitbox() {
    return { x, y, (float)w, (float)h };
}
