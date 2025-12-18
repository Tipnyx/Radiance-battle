#include"player.h"
#include"common.h"
#include"projectile.h"


Rect Projectile::getRect() { return { x, y, w, h }; }

void Projectile::drawDebug() {
        if (!debug_mode) return;
        setlinecolor(RED);
        //setfillstyle(BS_NULL); // 无填充
        Rect r = getRect();
        rectangle((int)r.x, (int)r.y, (int)(r.x + r.w), (int)(r.y + r.h));
 }

Orb::Orb(float sx, float sy, float px, float py) {
    x = sx; 
    y = sy;
    w = 60; 
    h = 60; 
    type = 0;
    spawnTime = GetTickCount();
    stateStartTime = spawnTime;

    // 初始方向指向玩家
    float angle = atan2(py - sy, px - sx);
    vx = cos(angle) * 2.0f; // 初始慢速移动
    vy = sin(angle) * 2.0f;
    currentSpeed = 2.0f;
}

void Orb::update(Player& p) {
    DWORD now = GetTickCount();
    float dt = 1.0f; // 步长
    float timeInState = (now - stateStartTime) / 1000.0f; // 转为秒

    // 1. 状态切换逻辑
    if (state == SPAWNING) {
        orbScale += 0.05f;
        if (orbScale > 1.0f) orbScale = 1.0f;

        if (timeInState > 0.5f) { // 0.5秒生成结束
            state = CHARGING;
            stateStartTime = now;
        }
    }
    else if (state == CHARGING) {
        // 冲刺一段时间后进入折返逻辑
        if (timeInState > 0.8f) {
            state = RECOVERING;
            stateStartTime = now;
            attackCount++;
        }
    }
    else if (state == RECOVERING) {
        // 弧线折返一段时间后，如果轮次没完，再次发起冲刺
        if (timeInState > 1.0f) { // 1秒的弧线时间，足以划出大圆
            if (attackCount < MAX_ATTACKS) {
                state = CHARGING;
                stateStartTime = now;
            }
            else {
                // 攻击次数用完，可以消失或飞出屏幕
            }
        }
    }

    // 2. 运动物理逻辑
    float targetX = p.x + p.w / 2;
    float targetY = p.y + p.h / 2;
    float dx = targetX - x;
    float dy = targetY - y;
    float dist = sqrt(dx * dx + dy * dy);

    if (state != SPAWNING) {
        // 计算理想方向
        float desiredVx = (dx / dist) * MAX_SPEED;
        float desiredVy = (dy / dist) * MAX_SPEED;

        // 转向力：当前速度向理想速度靠拢
        // 在 CHARGING 阶段转向力小（显得冲刺直），在 RECOVERING 阶段转向力适中
        float steeringWeight = (state == CHARGING) ? 0.04f : TURN_FORCE;

        vx += (desiredVx - vx) * steeringWeight;
        vy += (desiredVy - vy) * steeringWeight;

        // 保持恒定高速
        float actualSpeed = sqrt(vx * vx + vy * vy);
        vx = (vx / actualSpeed) * MAX_SPEED;
        vy = (vy / actualSpeed) * MAX_SPEED;
    }

    // 3. 应用位移
    x += vx;
    y += vy;

    // 边界处理
    if (now - spawnTime > 8000 || y > PLATFORM_Y) active = false;
    if (x < -100 || x > WINDOW_W + 100 || y < -100) active = false;
}

void Orb::draw() {
    DWORD now = GetTickCount();
    int baseR = (int)(30 * orbScale);

    // 配色：完全复用光柱色 COLOR_BEAM
    COLORREF colorMain = COLOR_BEAM;       // 淡黄色 RGB(255, 255, 224)
    COLORREF colorGlow = RGB(255, 255, 150); // 稍深的背景黄

    // --- 1. 绘制能量场（周围跳动的曲线） ---
    if (state != SPAWNING) {
        setlinecolor(colorMain);
        setlinestyle(PS_SOLID, 1);
        for (int i = 0; i < 6; i++) {
            // 利用随机数和时间戳，在球体周围生成不规则的弧线
            float angle = (float)(i * 60 + (now % 100)) * 3.1415f / 180.0f;
            int offsetX = (int)(cos(angle) * (baseR + 5 + rand() % 8));
            int offsetY = (int)(sin(angle) * (baseR + 5 + rand() % 8));
            // 绘制随机的小圆弧线段，模拟能量溢出
            arc((int)x + offsetX - 10, (int)y + offsetY - 10, (int)x + offsetX + 10, (int)y + offsetY + 10, angle, angle + 1.5f);
        }
    }

    // --- 2. 绘制多层核心 ---
    // 最外层淡色晕影
    setfillcolor(colorGlow);
    solidcircle((int)x, (int)y, baseR + 3 + (now % 3));

    // 主体层 (与光柱色一致)
    setfillcolor(colorMain);
    solidcircle((int)x, (int)y, baseR);

    // 极亮核心
    setfillcolor(WHITE);
    solidcircle((int)x, (int)y, (int)(baseR * 0.6f));

    // --- 3. 冲刺时的激波 ---
    if (state == CHARGING) {
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, 2);
        circle((int)x, (int)y, baseR + 8); // 纯白激波圆环
        setlinestyle(PS_SOLID, 1);
    }
}

Rect Orb::getRect() {
    // 只有生成完毕后才有碰撞判定
    if (state == SPAWNING) return { -100, -100, 0, 0 };
    return { x - 30, y - 30, 60, 60 };
}

Sword::Sword(float startX, float startY, float _vx, float _vy, float _angle, bool isCurve) {
    x = startX; y = startY;
    vx = _vx; vy = _vy;
    angle = _angle;
    speed = sqrt(vx * vx + vy * vy); // 记录预设速度
    spawnTime = GetTickCount();
    type = 1;
    w = 150; h = 14;

    // 根据波次决定弧线方向，产生交错旋转感
    // 或者简单给一个固定值。0.01f 左右是比较自然的弧线
    if (isCurve) {
        curveRate = 0.01f;
    }
}

std::vector<POINT> Sword::getHitPoints() {
    std::vector<POINT> points;
    float len = w; // 剑身长度
    float cosA = cos(angle);
    float sinA = sin(angle);

    // 在剑身上均匀分布 9 个点
    for (float i = 0; i <= 1.0f; i += 0.11f) {
        float dist = i * len;
        points.push_back({
            (long)(x + dist * cosA), // 画个斜着的杠杠,然后找个点,你自己算一算它的坐标就知道了
            (long)(y + dist * sinA) //很简单的,高中数学
            });
    }
    return points;
}

// 同时重写 drawDebug 以便在 P 键模式下看到这些点
void Sword::drawDebug(){
    if (!debug_mode) return;
    setlinecolor(RED);
    // 依然画出矩形框作为参考
    Rect r = getRect();

	//稍微调整一下矩形框的位置,让它更贴合剑身,你问我为什么?你自己算一算就知道了
    rectangle((int)r.x - sin(angle)* (0.5*h), (int)r.y + cos(angle) * (0.5*h), (int)(r.x + r.w), (int)(r.y + r.h));

    // 画出实际生效的红色判定点
    setfillcolor(RED);
    auto hPts = getHitPoints();
    for (auto& p : hPts) {
        solidcircle(p.x, p.y, 4); // 用小红点表示实际伤害位
    }
}

void Sword::update(Player& p) {
    DWORD now = GetTickCount();
    // 停留 0.5 秒后发射
    if (state == SWORD_PREVIEW && now - spawnTime > 500) {
        state = SWORD_LAUNCH;
    }

    if (state == SWORD_LAUNCH) {
        // --- 核心：弧线轨迹算法 ---
        // 旋转速度向量 vx, vy
        if (curveRate != 0) {
            float oldVx = vx;
            float oldVy = vy;
            vx = oldVx * cos(curveRate) - oldVy * sin(curveRate);
            vy = oldVx * sin(curveRate) + oldVy * cos(curveRate);
            angle += curveRate; // 剑身跟着轨迹转
        }

        x += vx;
        y += vy;
    }

    if (x < -200 || x > WINDOW_W + 200 || y < -200 || y > WINDOW_H + 200) active = false;
}

void Sword::draw(){
    // 预览阶段颜色稍微暗一点或带点透明感，发射时纯白
    if (state == SWORD_PREVIEW) {
        setfillcolor(RGB(200, 200, 200));
        setlinecolor(RGB(200, 200, 200));
    }
    else {
        setfillcolor(WHITE);
        setlinecolor(WHITE);
    }

    // --- 使用你最满意的纯白核心几何体 ---
    float totalLen = 150.0f;
    float bladeW = 7.0f;
    float tipLen = 20.0f;
    float hiltLen = 25.0f;
    float pommelSize = 8.0f;

    float cosA = cos(angle);
    float sinA = sin(angle);
    auto trans = [&](POINT p) -> POINT {
        return { (long)(x + p.x * cosA - p.y * sinA), (long)(y + p.x * sinA + p.y * cosA) };
        };

    // 剑身组件
    POINT rBody[] = { trans({(int)hiltLen, (int)-bladeW}), trans({(int)(totalLen - tipLen), (int)-bladeW}),
                        trans({(int)(totalLen - tipLen), (int)bladeW}), trans({(int)hiltLen, (int)bladeW}) };
    POINT rTip[] = { trans({(int)totalLen, 0}), trans({(int)(totalLen - tipLen), (int)-bladeW}), trans({(int)(totalLen - tipLen), (int)bladeW}) };
    POINT rHilt[] = { trans({(int)pommelSize, (int)-4}), trans({(int)hiltLen, (int)-4}),
                        trans({(int)hiltLen, (int)4}), trans({(int)pommelSize, (int)4}) };
    POINT rPommel[] = { trans({(int)pommelSize, 0}), trans({0, (int)-4}), trans({(int)-pommelSize, 0}), trans({0, (int)4}) };

    solidpolygon(rBody, 4);
    solidpolygon(rTip, 3);
    solidpolygon(rHilt, 4);
    solidpolygon(rPommel, 4);

    if (state == SWORD_LAUNCH) {
        setlinestyle(PS_SOLID, 2);
        line(trans({ (int)hiltLen, 0 }).x, trans({ (int)hiltLen, 0 }).y, trans({ (int)totalLen - 10, 0 }).x, trans({ (int)totalLen - 10, 0 }).y);
        setlinestyle(PS_SOLID, 1);
    }
}

Rect Sword::getRect(){
    // 预览阶段没有伤害判定，防止生成时直接杀掉玩家
    if (state == SWORD_PREVIEW) return { -1000, -1000, 0, 0 };

    float absCos = fabsf(cos(angle));
    float absSin = fabsf(sin(angle));
    float newW = 150 * absCos + 14 * absSin;
    float newH = 150 * absSin + 14 * absCos;
    return { x - (vx < 0 ? newW : 0), y - (vy < 0 ? newH : 0), newW, newH };
}

Beam::Beam(float startX, float _speed) {
    x = startX;
    y = 0;
    w = 50; h = WINDOW_H;
    speed = _speed;
    type = 2;
}

void Beam::update(Player& p) {
    x += speed;
    if (x < -200 || x > WINDOW_W + 200) active = false;
}

void Beam::draw() {
    DWORD now = GetTickCount();
    // 利用正弦函数产生 8 到 12 像素之间的呼吸波动
    int dynamicGlow = 8 + (int)(2.0f * sin(now / 100.0f));

    // 绘制稍微暗一点的余辉
    setfillcolor(RGB(180, 180, 140));
    solidrectangle((int)(x - dynamicGlow), 0, (int)x, WINDOW_H);
    solidrectangle((int)(x + w), 0, (int)(x + w + dynamicGlow), WINDOW_H);

    // 主光柱
    setfillcolor(COLOR_BEAM);
    solidrectangle((int)x, 0, (int)(x + w), WINDOW_H);

    // 3. (可选) 增加中心亮线，强化视觉冲击力
    setfillcolor(WHITE);
    solidrectangle((int)(x + w / 2 - 2), 0, (int)(x + w / 2 + 2), WINDOW_H);
}
