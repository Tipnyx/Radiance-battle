# include"common.h"
# include"player.h"

extern std::vector<Rect> platforms;

Player::Player() {
    reset();
    hasDashedInAir = false;
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

    if (onGround) {
        hasDashedInAir = false; // 落地了，恢复空中冲刺能力
    }

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
            //x += vx;
            //if (currentTime - lastGhostTime > 30) {
            //    // 初始生命值给满 (255)，让它存在感更强
            //    ghosts.push_back({ x, y, 255 });
            //    lastGhostTime = currentTime;
            //}
            //return; // 冲刺时不处理常规移动
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

    if (!isDashing) {
        int inputDir = 0;
        if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT)) {inputDir = -1;}
        else if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) {inputDir = 1;}
        
        float t = 0.0f;
        if (MOVE_SPEED > 0) {
            t = sqrt(fabs(vx) / MOVE_SPEED);
        }
        // 限制一下范围，防止浮点误差
        if (t > 1.0f) t = 1.0f;

        // 3. 定义每帧的变化量
        // 4帧充满
        float delta = 0.25f;

        // 4. 计算新的进度 t
        if (inputDir != 0) {
            // --- 玩家正在按键 ---

            // 情况A: 正在急停/转向 (按键方向 与 角色朝向 不一致)
            if (inputDir != facing && t > 0.01f) {
                // 先执行减速逻辑
                t -= delta;
                if (t < 0.0f) {
                    t = 0.0f;
                    facing = inputDir; // 减速到0后，正式掉头
                }
            }
            else {
                // 情况B: 正常加速
                facing = inputDir; // 确保朝向正确
                t += delta;
                if (t > 1.0f) t = 1.0f;
            }
        }
        else {
            // --- 玩家松开按键 ---
            // 执行减速
            t -= delta;
            if (t < 0.0f) t = 0.0f;
        }

        // 5. 应用指数曲线 (Quadratic Curve / t^2)
        // t=0.2 (1帧) -> 速度 4% (起步非常柔和)
        // t=0.4 (2帧) -> 速度 16%
        // t=0.6 (3帧) -> 速度 36%
        // t=0.8 (4帧) -> 速度 64% (后半段发力)
        // t=1.0 (5帧) -> 速度 100%
        vx = facing * MOVE_SPEED * (t * t);

        // 跳跃
        bool jumpKey = (GetAsyncKeyState('Z') & 0x8000) != 0;

        // 只在按下瞬间触发跳跃
        if (jumpKey && !lastJumpKey && jumpCount < MAX_JUMP) {
            vy = JUMP_FORCE;
            onGround = false;
            jumpCount++;
        }
        lastJumpKey = jumpKey;
    }

    if (GetAsyncKeyState('P') & 1) { // 使用 &1 确保只在按下瞬间触发一次切换
        debug_mode = !debug_mode;
    }

    // 冲刺触发
    if (GetAsyncKeyState('C')) {
        bool canNormal = (currentTime - lastNormalDashTime > NORMAL_DASH_COOLDOWN);

        if (!isDashing && !isAttacking && canNormal && (!hasDashedInAir || onGround)) {
            isDashing = true;
            dashStartTime = currentTime;
            lastNormalDashTime = currentTime;

            if (!onGround) {
                hasDashedInAir = true; // 只要在空中冲了，就锁上
            }

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
    if (!isDashing) vy += GRAVITY;
    x += vx;

    // X 轴碰撞检测
    // 获取玩家当前的碰撞箱 (使用 float 精度)
    Rect pRect = getHitbox();

    for (const auto& wall : platforms) {
        if (pRect.checkCollision(wall)) {
            // 发生了碰撞，判断是撞到了左边还是右边
            if (vx > 0) {
                // 向右撞墙：把玩家拉回到墙的左边缘
                x = wall.x - w;
            }
            else if (vx < 0) {
                // 向左撞墙：把玩家拉回到墙的右边缘
                x = wall.x + wall.w;
            }
            vx = 0; // 撞墙后水平速度归零
            break; // 既然撞了一个，就不用检测其他的了（简化处理）
        }
    }

    y += vy;
    // 平台碰撞
    onGround = false;
    // Y 轴碰撞检测
    pRect = getHitbox(); // 更新因为 y 变化后的碰撞箱位置

    for (const auto& wall : platforms) {
        if (pRect.checkCollision(wall)) {
            // 发生了碰撞
            if (vy > 0) {
                // 向下掉落撞地板：修正到地板上方
                y = wall.y - h;
                onGround = true;
                jumpCount = 0; // 刷新跳跃次数
                // 落地时如果有落地动画或音效可以在这加
            }
            else if (vy < 0) {
                // 向上跳起顶头：修正到天花板下方
                y = wall.y + wall.h;
                // 顶头后可以选择让速度归零，或者稍微反弹
                // vy = 0; 
                vy = 1.0f; // 给一点向下的反弹力，手感更好
            }

            // 修正速度，防止持续穿透
            if (vy > 0) vy = 0; // 只有落地才清空速度，顶头已在上面处理
            break;
        }
    }

    if (isDashing) {
        if (currentTime - lastGhostTime > 30) {
            ghosts.push_back({ x, y, 255 });
            lastGhostTime = currentTime;
        }
    }

    // 掉出屏幕重置 (虚空伤害)
    if (y > WINDOW_H) {
        hp--;
        hurtTimer = HURT_DURATION;
        isInvincible = true;
        x = WINDOW_W / 2;
        y = PLATFORM_Y - h - 100;
        vx = 0; vy = 0;
        jumpCount = 0;
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
        solidrectangle((int)g.x -cameraX + 3, (int)g.y -cameraY + 3, (int)(g.x - cameraX + w - 3), (int)(g.y - cameraY + h - 3));
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
    solidrectangle((int)x - cameraX, (int)y - cameraY, (int)(x - cameraX + w), (int)(y - cameraY + h));

    // 绘制暗影冲刺就绪指示器
    if (canShadowDash) {
        setfillcolor(BLACK);
        solidcircle((int)(x - cameraX + w / 2), (int)(y - cameraY - 8), 3);
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
            pts[i].x = (long)drawX_Outer - cameraX;
            pts[i].y = (long)drawY_Outer - cameraY;

            // 后半部分逆序存内弧 (闭合形状)
            int innerIndex = (SAMPLE_COUNT * 2 - 1) - i;
            pts[innerIndex].x = (long)drawX_Inner - cameraX;
            pts[innerIndex].y = (long)drawY_Inner - cameraY;
        }

        // 绘制
        solidpolygon(pts, SAMPLE_COUNT * 2);
    }

    if (debug_mode) {
        settextcolor(RGB(0, 255, 0)); // 用亮绿色，显眼
        settextstyle(20, 0, _T("Consolas")); // 字号 20，清晰
        setbkmode(TRANSPARENT); // 文字背景透明

        TCHAR s[64];
        // 显示绝对坐标 (World Pos)，这就是你要填进 GenerateStairs 里的 x 和 y
        _stprintf_s(s, _T("(%.0f, %.0f)"), x, y + h); // y+h 显示的是脚底板的坐标，更适合对齐台阶

        // 画在玩家头顶上方 30 像素处
        // 注意：绘图位置必须减去 cameraX/Y，否则文字会跟着地图跑而不是跟着人跑
        outtextxy((int)(x - cameraX - 20), (int)(y - cameraY - 30), s);

        // 可选：顺便画个十字准星标出脚底中心，方便对齐
        setlinecolor(RED);
        int footX = (int)(x - cameraX + w / 2);
        int footY = (int)(y - cameraY + h);
        line(footX - 10, footY, footX + 10, footY);
        line(footX, footY - 10, footX, footY + 10);
    }
}

Rect Player::getHitbox() {
    return { x, y, (float)w, (float)h };
}
