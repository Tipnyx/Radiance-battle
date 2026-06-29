#include"player.h"
#include"common.h"
#include"projectile.h"
#include"world.h"

Rect Projectile::getRect() { return { x, y, w, h }; }

void Projectile::drawDebug() {
    if (!debug_mode) return;
    glSetColor(COLOR_RED);
    Rect r = getRect();
    drawLineRect(r.x - cameraX, r.y - cameraY, r.w, r.h);
}

// ==================== Orb ====================

Orb::Orb(float sx, float sy, float px, float py) {
    x = sx;
    y = sy;
    w = 80;
    h = 80;
    type = 0;
    spawnTime = GetTickCount();
    stateStartTime = spawnTime;

    float angle = atan2(py - sy, px - sx);
    vx = cos(angle) * 2.0f;
    vy = sin(angle) * 2.0f;
    currentSpeed = 2.0f;
}

void Orb::update(Player& p) {
    DWORD now = GetTickCount();
    float timeInState = (now - stateStartTime) / 1000.0f;

    if (state == SPAWNING) {
        orbScale += 0.05f;
        if (orbScale > 1.0f) orbScale = 1.0f;
        if (timeInState > 0.5f) { state = CHARGING; stateStartTime = now; }
    }
    else if (state == CHARGING) {
        if (timeInState > 0.8f) { state = RECOVERING; stateStartTime = now; attackCount++; }
    }
    else if (state == RECOVERING) {
        if (timeInState > 1.0f) {
            if (attackCount < MAX_ATTACKS) { state = CHARGING; stateStartTime = now; }
        }
    }

    float targetX = p.x + p.w / 2;
    float targetY = p.y + p.h / 2;
    float dx = targetX - x;
    float dy = targetY - y;
    float dist = sqrt(dx * dx + dy * dy);

    if (state != SPAWNING) {
        float desiredVx = (dx / dist) * MAX_SPEED;
        float desiredVy = (dy / dist) * MAX_SPEED;
        float steeringWeight = (state == CHARGING) ? 0.04f : TURN_FORCE;
        vx += (desiredVx - vx) * steeringWeight;
        vy += (desiredVy - vy) * steeringWeight;
        float actualSpeed = sqrt(vx * vx + vy * vy);
        vx = (vx / actualSpeed) * MAX_SPEED;
        vy = (vy / actualSpeed) * MAX_SPEED;
    }

    x += vx * g_deltaTime * 60.0f;
    y += vy * g_deltaTime * 60.0f;

    if (now - spawnTime > 8000) active = false;
    if (x < -100 || x > WINDOW_W + 100 || y < -5450) active = false;
    for (auto& plat : platforms) {
        if (getRect().checkCollision(plat)) { active = false; break; }
    }
}

void Orb::draw() {
    DWORD now = GetTickCount();
    int baseR = (int)(40 * orbScale);

    if (state != SPAWNING) {
        glSetColor(COLOR_BEAM);
        glSetLineWidth(1);
        for (int i = 0; i < 6; i++) {
            float ang = (float)(i * 60 + (now % 100)) * 3.1415f / 180.0f;
            int offsetX = (int)(cos(ang) * (baseR + 5 + rand() % 8));
            int offsetY = (int)(sin(ang) * (baseR + 5 + rand() % 8));
            drawArc(x - cameraX + offsetX, y - cameraY + offsetY,
                    10, 10, ang, 1.5f);
        }
        glSetLineWidth(1);
    }

    // 光晕
    glSetColor(MakeColor(255, 255, 150));
    drawFilledCircle(x - cameraX, y - cameraY, baseR + 3 + (now % 3));

    // 主体
    glSetColor(COLOR_BEAM);
    drawFilledCircle(x - cameraX, y - cameraY, baseR);

    // 高光
    glSetColor(COLOR_WHITE);
    drawFilledCircle(x - cameraX, y - cameraY, (int)(baseR * 0.6f));

    // 蓄力环
    if (state == CHARGING) {
        glSetColor(COLOR_WHITE);
        glSetLineWidth(2);
        drawLineCircle(x - cameraX, y - cameraY, baseR + 8);
        glSetLineWidth(1);
    }
}

Rect Orb::getRect() {
    if (state == SPAWNING) return { -100, -100, 0, 0 };
    return { x - 40, y - 40, 80, 80 };
}

// ==================== Sword ====================

Sword::Sword(float startX, float startY, float _vx, float _vy, float _angle, bool isCurve) {
    x = startX; y = startY;
    vx = _vx; vy = _vy;
    angle = _angle;
    speed = sqrt(vx * vx + vy * vy);
    spawnTime = GetTickCount();
    type = 1;
    w = 200; h = 14;
    if (isCurve) curveRate = 0.01f;
}

std::vector<POINT> Sword::getHitPoints() {
    std::vector<POINT> points;
    float len = w;
    float cosA = cos(angle);
    float sinA = sin(angle);
    for (float i = 0; i <= 1.0f; i += 0.11f) {
        float dist = i * len;
        points.push_back({ (long)(x + dist * cosA), (long)(y + dist * sinA) });
    }
    return points;
}

void Sword::drawDebug() {
    if (!debug_mode) return;
    glSetColor(COLOR_RED);
    Rect r = getRect();
    drawLineRect(r.x - cameraX - sin(angle) * (0.5f * h),
                 r.y - cameraY + cos(angle) * (0.5f * h),
                 r.w, r.h);
    glSetColor(COLOR_RED);
    auto hPts = getHitPoints();
    for (auto& p : hPts) {
        drawFilledCircle(p.x - cameraX, p.y - cameraY, 4);
    }
}

void Sword::update(Player& p) {
    DWORD now = GetTickCount();
    if (state == SWORD_PREVIEW && now - spawnTime > 500) state = SWORD_LAUNCH;
    if (state == SWORD_LAUNCH) {
        if (curveRate != 0) {
            float cr = curveRate * g_deltaTime * 60.0f;
            float oldVx = vx, oldVy = vy;
            vx = oldVx * cos(cr) - oldVy * sin(cr);
            vy = oldVx * sin(cr) + oldVy * cos(cr);
            angle += cr;
        }
        x += vx * g_deltaTime * 60.0f;
        y += vy * g_deltaTime * 60.0f;
    }
    if (x < -400 || x > WINDOW_W + 400 || y < -2000 || y > WINDOW_H + 200) active = false;
}

void Sword::draw() {
    if (state == SWORD_PREVIEW) {
        glSetColor(MakeColor(200, 200, 200));
    }
    else {
        glSetColor(COLOR_WHITE);
    }

    float totalLen = 200.0f;
    float bladeW = 7.0f;
    float tipLen = 20.0f;
    float hiltLen = 25.0f;
    float pommelSize = 8.0f;

    float cosA = cos(angle);
    float sinA = sin(angle);
    auto trans = [&](POINT p) -> POINT {
        return { (long)(x - cameraX + p.x * cosA - p.y * sinA),
                 (long)(y - cameraY + p.x * sinA + p.y * cosA) };
    };

    // 发光
    if (state == SWORD_LAUNCH) {
        float glowW = bladeW + 2.0f;
        glSetColor(MakeColor(200, 200, 200));
        POINT gBody[] = { trans({(int)hiltLen - 4, (int)-glowW}), trans({(int)(totalLen - tipLen + 5), (int)-glowW}),
                          trans({(int)(totalLen - tipLen + 5), (int)glowW}), trans({(int)hiltLen - 4, (int)glowW}) };
        POINT gTip[] = { trans({(int)totalLen + 8, 0}), trans({(int)(totalLen - tipLen + 5), (int)-glowW}),
                         trans({(int)(totalLen - tipLen + 5), (int)glowW}) };
        drawFilledPolygon(gBody, 4);
        drawFilledPolygon(gTip, 3);
    }

    // 描边颜色
    if (state == SWORD_PREVIEW) {
        glSetColor(MakeColor(100, 100, 100));
    }
    else {
        glSetColor(MakeColor(255, 200, 50));
    }

    // 剑身
    POINT rBody[] = { trans({(int)hiltLen, (int)-bladeW}), trans({(int)(totalLen - tipLen), (int)-bladeW}),
                      trans({(int)(totalLen - tipLen), (int)bladeW}), trans({(int)hiltLen, (int)bladeW}) };
    POINT rTip[] = { trans({(int)totalLen, 0}), trans({(int)(totalLen - tipLen), (int)-bladeW}),
                     trans({(int)(totalLen - tipLen), (int)bladeW}) };
    POINT rHilt[] = { trans({(int)pommelSize, (int)-4}), trans({(int)hiltLen, (int)-4}),
                      trans({(int)hiltLen, (int)4}), trans({(int)pommelSize, (int)4}) };
    POINT rPommel[] = { trans({(int)pommelSize, 0}), trans({0, (int)-4}),
                        trans({(int)-pommelSize, 0}), trans({0, (int)4}) };

    drawFilledPolygon(rBody, 4);
    drawFilledPolygon(rTip, 3);
    drawFilledPolygon(rHilt, 4);
    drawFilledPolygon(rPommel, 4);

    // 高亮中线
    if (state == SWORD_LAUNCH) {
        glSetColor(COLOR_WHITE);
        glSetLineWidth(2);
        drawLine(trans({ (int)hiltLen, 0 }).x, trans({ (int)hiltLen, 0 }).y,
                 trans({ (int)totalLen - 10, 0 }).x, trans({ (int)totalLen - 10, 0 }).y);
        glSetLineWidth(1);
    }
}

Rect Sword::getRect() {
    if (state == SWORD_PREVIEW) return { -1000, -1000, 0, 0 };
    float absCos = fabsf(cos(angle));
    float absSin = fabsf(sin(angle));
    float newW = 150 * absCos + 14 * absSin;
    float newH = 150 * absSin + 14 * absCos;
    return { x - (vx < 0 ? newW : 0), y - (vy < 0 ? newH : 0), newW, newH };
}

// ==================== Beam ====================

Beam::Beam(float startX, float _speed) {
    x = startX;
    y = 0;
    w = 100; h = WINDOW_H;
    speed = _speed;
    type = 2;
}

void Beam::update(Player& p) {
    x += speed * g_deltaTime * 60.0f;
    if (x < -200 || x > WINDOW_W + 200) active = false;
}

void Beam::draw() {
    DWORD now = GetTickCount();
    int dynamicGlow = 8 + (int)(2.0f * sin(now / 100.0f));

    glSetColor(MakeColor(180, 180, 140));
    drawFilledRect(x - cameraX - dynamicGlow, 0, dynamicGlow, WINDOW_H);
    drawFilledRect(x - cameraX + w, 0, dynamicGlow, WINDOW_H);

    glSetColor(COLOR_BEAM);
    drawFilledRect(x - cameraX, 0, w, WINDOW_H);

    glSetColor(COLOR_WHITE);
    drawFilledRect(x - cameraX + w / 2 - 2, 0, 4, WINDOW_H);
}

// ==================== Laser ====================

Laser::Laser(float _cx, float _cy, float _angle) {
    cx = _cx; cy = _cy;
    angle = _angle;
    x = cx; y = cy;
    type = 3;
    active = true;
    stateStartTime = GetTickCount();
    currentWidth = 2.0f;
}

void Laser::update(Player& p) {
    DWORD now = GetTickCount();
    float timeInState = (now - stateStartTime) / 1000.0f;

    if (state == LASER_PREPARE) {
        currentWidth = 3.0f + sin(now / 50.0f) * 2.0f;
        if (timeInState > 0.5f) { state = LASER_FIRE; stateStartTime = now; }
    }
    else if (state == LASER_FIRE) {
        if (timeInState < 0.1f) currentWidth = 100.0f * (timeInState / 0.1f);
        else currentWidth = 80.0f;
        if (timeInState > 0.5f) { state = LASER_FADE; stateStartTime = now; }
    }
    else if (state == LASER_FADE) {
        currentWidth = 50.0f * (1.0f - (timeInState / 0.3f));
        if (timeInState > 0.3f) active = false;
    }
}

void Laser::draw() {
    float ex = cx + cos(angle) * length;
    float ey = cy + sin(angle) * length;
    float w = currentWidth / 2.0f;
    float nx = -sin(angle) * w;
    float ny = cos(angle) * w;

    POINT pts[4];
    pts[0] = { (long)(cx + nx - cameraX), (long)(cy + ny - cameraY) };
    pts[1] = { (long)(ex + nx - cameraX), (long)(ey + ny - cameraY) };
    pts[2] = { (long)(ex - nx - cameraX), (long)(ey - ny - cameraY) };
    pts[3] = { (long)(cx - nx - cameraX), (long)(cy - ny - cameraY) };

    if (state == LASER_PREPARE) {
        glSetColor(MakeColor(255, 200, 100));
        glSetLineWidth(2);
        drawLine(cx - cameraX, cy - cameraY, ex - cameraX, ey - cameraY);
        glSetLineWidth(1);
    }
    else if (state == LASER_FIRE) {
        glSetColor(MakeColor(255, 255, 180));
        drawFilledPolygon(pts, 4);

        float cw = currentWidth / 4.0f;
        float cnx = -sin(angle) * cw;
        float cny = cos(angle) * cw;
        POINT cpts[4] = {
            { (long)(cx + cnx - cameraX), (long)(cy + cny - cameraY) },
            { (long)(ex + cnx - cameraX), (long)(ey + cny - cameraY) },
            { (long)(ex - cnx - cameraX), (long)(ey - cny - cameraY) },
            { (long)(cx - cnx - cameraX), (long)(cy - cny - cameraY) }
        };
        glSetColor(COLOR_WHITE);
        drawFilledPolygon(cpts, 4);
    }
    else if (state == LASER_FADE) {
        glSetColor(MakeColor(255, 220, 150));
        drawFilledPolygon(pts, 4);
    }
}

std::vector<POINT> Laser::getHitPoints() {
    std::vector<POINT> points;
    if (state != LASER_FIRE) return points;
    for (float d = 0; d < length; d += 30.0f) {
        if (d > 1000 && d > length) break;
        points.push_back({ (long)(cx + cos(angle) * d), (long)(cy + sin(angle) * d) });
    }
    return points;
}

void Laser::drawDebug() {
    if (!debug_mode) return;
    glSetColor(COLOR_MAGENTA);
    auto pts = getHitPoints();
    for (auto& p : pts) {
        drawLineCircle(p.x - cameraX, p.y - cameraY, (int)(currentWidth / 2));
    }
}
