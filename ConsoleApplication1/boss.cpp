# include"boss.h"
# include"projectile.h"
# include"player.h"
# include"world.h"
# include"BossState.h"

extern Player player;
extern Boss boss;
extern std::vector<Projectile*> projectiles;

Boss::Boss() {
    x = WINDOW_W / 2;
    y = -100;
    currentState = new PhaseOneState();
}

void Boss::InitSunCache() {
    const int S = 512;
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    sunTex.create(S, S);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sunTex.id, 0);
    glViewport(0, 0, S, S);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, S, S, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    int mid = S / 2;
    for (int i = 0; i < 90; i++) {
        glColor3ub(255, (GLubyte)(192 - i > 0 ? 192 - i : 0), (GLubyte)(107 - i > 0 ? 107 - i : 0));
        glBegin(GL_TRIANGLE_FAN);
        float rx = (float)(125 - i) / 300.0f * (float)mid;
        float ry = (float)(125 - i) / 300.0f * (float)mid;
        glVertex2f((float)mid, (float)mid);
        for (int j = 0; j <= 36; j++) {
            float a = j * 2.0f * 3.14159f / 36.0f;
            glVertex2f((float)mid + cos(a) * rx, (float)mid + sin(a) * ry);
        }
        glEnd();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_W, WINDOW_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Boss::InitHitCache() {
    const int S = 512;
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    hitTex.create(S, S);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hitTex.id, 0);
    glViewport(0, 0, S, S);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, S, S, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    int mid = S / 2;
    for (int i = 0; i < 45; i++) {
        glColor3ub(255, (GLubyte)(255 - 2 * i > 0 ? 255 - 2 * i : 0), (GLubyte)(255 - 2 * i > 0 ? 255 - 2 * i : 0));
        glBegin(GL_TRIANGLE_FAN);
        float rx = (float)(125 - i) / 300.0f * (float)mid;
        float ry = (float)(125 - i) / 300.0f * (float)mid;
        glVertex2f((float)mid, (float)mid);
        for (int j = 0; j <= 36; j++) {
            float a = j * 2.0f * 3.14159f / 36.0f;
            glVertex2f((float)mid + cos(a) * rx, (float)mid + sin(a) * ry);
        }
        glEnd();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_W, WINDOW_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Boss::update() {
    if (alpha < 1.0f) {
        alpha += 0.0033f;
        if (!isPhaseTwoActive) y += 1.0f;
        return;
    }
    float hoverBaseY = isPhaseTwoActive ? targetY : 200.0f;
    float hoverOffset = sin(GetTickCount() / 500.0f) * 20;
    if (isTeleporting) {
        x += (targetX - x) * 0.10f;
        if (isPhaseTwoActive) y += (targetY - y) * 0.10f;
        else if (isPhaseClimbing || isPhaseThree) y += (targetY - y) * 0.01f;
        else y = 200 + hoverOffset;
        TrailPoint p = { x, y, alpha };
        trails.push_back(p);
        if (trails.size() > MAX_TRAILS) trails.erase(trails.begin());
        float dist = sqrt(pow(x - targetX, 2) + pow(y - targetY, 2));
        if (dist < 2.0f) {
            x = targetX;
            if (isPhaseTwoActive || isPhaseClimbing || isPhaseThree) y = targetY;
            isTeleporting = false;
        }
    }
    else {
        if (isPhaseTwoActive || isPhaseClimbing || isPhaseThree) y = targetY + hoverOffset;
        else y = 200 + hoverOffset;
        if (!trails.empty()) trails.erase(trails.begin());
    }
    UpdateAttacks();
    UpdateStateMachine();
}

void Boss::draw() {
    if (!active) return;

    for (size_t i = 0; i < trails.size(); i++) {
        float trailAlpha = (i / (float)trails.size()) * alpha * 0.4f;
        int tx = (int)trails[i].x, ty = (int)trails[i].y;
        for (int k = 0; k < 60; k += 10) {
            Color c = Fade(MakeColor(255, 180 - k, 100 - k), trailAlpha);
            glSetColor(c);
            drawFilledCircle(tx - (int)cameraX, ty - (int)cameraY, (float)(70 - k));
        }
    }

    int cx = (int)x, cy = (int)y;
    int r = 185;

    // Draw cached sun texture as base glow
    Texture* tex = (boss_is_invincible && hitTex.id) ? &hitTex : &sunTex;
    if (tex->id) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        tex->bind();
        glColor4f(1, 1, 1, 1.0f);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // premultiplied to avoid dark edges
        float sz = 380.0f;
        float hsz = sz / 2.0f;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(cx - cameraX - hsz, cy - cameraY - hsz);
        glTexCoord2f(1, 0); glVertex2f(cx - cameraX + hsz, cy - cameraY - hsz);
        glTexCoord2f(1, 1); glVertex2f(cx - cameraX + hsz, cy - cameraY + hsz);
        glTexCoord2f(0, 1); glVertex2f(cx - cameraX - hsz, cy - cameraY + hsz);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        Texture::unbind();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // restore default
    }

    for (int j = 0; j < 3; j++) {
        glSetColor(Fade(MakeColor(240, 240, 240), alpha));
        glSetLineWidth(5);
        drawLineCircle((float)(cx - (int)cameraX), (float)(cy - (int)cameraY), (float)(125 + 30 * j));
        glSetLineWidth(1);
    }

    for (int i = 0; i < 8; i++) {
        float ang = i * 3.1415f / 4.0f + (GetTickCount() / 1000.0f);
        int ex = cx + (int)(cos(ang) * r);
        int ey = cy + (int)(sin(ang) * r);

        glSetColor(Fade(MakeColor(255, 255, 220), alpha));
        glSetLineWidth((float)(11 + (rand() % 3)));
        drawLine((float)(cx - (int)cameraX), (float)(cy - (int)cameraY), (float)(ex - (int)cameraX), (float)(ey - (int)cameraY));

        glSetColor(Fade(MakeColor(204, 218, 221), alpha));
        glSetLineWidth(9);
        drawLine((float)(cx - (int)cameraX), (float)(cy - (int)cameraY), (float)(ex - (int)cameraX), (float)(ey - (int)cameraY));

        float spikeLen = 45.0f, spikeWidth = 18.0f, baseR = r - 10.0f;
        POINT pts[3];
        pts[0].x = (LONG)(cx - cameraX + (baseR + spikeLen) * cos(ang));
        pts[0].y = (LONG)(cy - cameraY + (baseR + spikeLen) * sin(ang));
        pts[1].x = (LONG)(cx - cameraX + baseR * cos(ang) - spikeWidth * sin(ang));
        pts[1].y = (LONG)(cy - cameraY + baseR * sin(ang) + spikeWidth * cos(ang));
        pts[2].x = (LONG)(cx - cameraX + baseR * cos(ang) + spikeWidth * sin(ang));
        pts[2].y = (LONG)(cy - cameraY + baseR * sin(ang) - spikeWidth * cos(ang));
        glSetColor(Fade(MakeColor(255, 255, 240), alpha));
        drawFilledPolygon(pts, 3);
    }
    glSetLineWidth(1);

    glSetColor(Fade(MakeColor(255, 200, 17), alpha));
    glSetLineWidth(3);
    drawLineCircle((float)(cx - (int)cameraX), (float)(cy - (int)cameraY), (float)(90 + (rand() % 3)));
    glSetLineWidth(1);
}

Rect Boss::getRect() {
    return { x - 125, y - 125, 250, 250 };
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
    glSetColor(COLOR_RED);
    Rect r = getRect();
    drawLineRect(r.x - cameraX, r.y - cameraY, r.w, r.h);
}

void Boss::SpawnSwordWallHorizontal(bool fromLeft) {
    float startX = fromLeft ? -280.0f : WINDOW_W + 280.0f;
    float vx = fromLeft ? 20.0f : -20.0f;
    float angle = fromLeft ? 0.0f : 3.14159f;
    int totalSwords = 16, spacing = 70;
    int gapA = rand() % (totalSwords * 1 / 8) + 1 + (rand() % 3 - 1);
    int gapB = rand() % (totalSwords * 3 / 8) + (rand() % 5 - 1);
    int gapC = rand() % (totalSwords * 3 / 8) + (rand() % 5 - 1);
    int gapD = rand() % (totalSwords * 5 / 8) + (rand() % 5 - 1);
    int randomOffset = (rand() % 50) - 30;
    for (int i = 0; i < totalSwords; i++) {
        if ((i == gapA || i == gapB || i == gapC || i == gapD)) continue;
        float py = currentLevelBottom - (i * spacing) + randomOffset;
        projectiles.push_back(new Sword(startX, py, vx, 0, angle, false));
    }
}

void Boss::SpawnSwordWallVertical() {
    int totalSwords = 24; float spacing = 75.0f;
    int gapA = rand() % (totalSwords * 1 / 8) + (rand() % 3 - 1);
    int gapB = rand() % (totalSwords * 3 / 8) + (rand() % 5 - 1);
    int gapC = rand() % (totalSwords * 3 / 8) + (rand() % 5 - 1);
    int gapD = rand() % (totalSwords * 5 / 8) + (rand() % 5 - 1);
    int gapE = rand() % (totalSwords * 5 / 8) + (rand() % 5 - 1);
    int gapF = rand() % (totalSwords * 7 / 8) + (rand() % 3 - 1);
    int randomOffset = (rand() % 60) - 30;
    float startXBase = PLATFORM_X - 300;
    for (int i = 0; i < 25; i++) {
        if ((i == gapA || i == gapB || i == gapC || i == gapD || i == gapE || i == gapF)) continue;
        float px = startXBase + randomOffset + (i * spacing);
        projectiles.push_back(new Sword(px, -100.0f, 0, 20.0f, 1.5708f, false));
    }
}

void Boss::SpawnOrbs() {
    const float minDistToPlayer = 200.0f;
    const float minRectBuffer = 80.0f;
    float ox, oy; int tryCount = 0;
    do {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float dist = 300.0f;
        ox = (WINDOW_W / 2) + cos(angle) * dist;
        oy = (currentLevelBottom - 450) - sin(angle) * 100;
        float dx = ox - (player.x + player.w / 2);
        float dy = oy - (player.y + player.h / 2);
        if (sqrt(dx * dx + dy * dy) < minDistToPlayer) { tryCount++; continue; }
        bool isSafe = true;
        for (const auto& rect : platforms) {
            if (ox > rect.x - minRectBuffer && ox < rect.x + rect.w + minRectBuffer &&
                oy > rect.y - minRectBuffer && oy < rect.y + rect.h + minRectBuffer) {
                isSafe = false; break;
            }
        }
        if (isSafe) break;
        tryCount++;
    } while (tryCount < 30);
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
    int count = 12; float startRadius = 60.0f;
    float step = (2 * 3.14159f) / count;
    float offsetAngle = (burstWaveCount == 1) ? (step / 2.0f) : 0.0f;
    for (int i = 0; i < count; i++) {
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
    int count = 8; float step = (2 * 3.14159f) / count;
    float waveOffset = (laserWaveCount % 3) * (step / 3.0f);
    float randomJitter = ((rand() % 100) / 100.0f * 0.17f) - 0.08f;
    for (int i = 0; i < count; i++) {
        float angle = waveOffset + randomJitter + (i * step);
        projectiles.push_back(new Laser(x, y, angle));
    }
}

void Boss::ChangeState(BossState* newState) {
    if (currentState) { currentState->Exit(*this); delete currentState; }
    currentState = newState;
    if (currentState) currentState->Enter(*this);
}

void Boss::UpdateStateMachine() {
    if (currentState) currentState->Execute(*this);
}

void Boss::UpdateAttacks() {
    DWORD currentTime = GetTickCount();
    if (climbingLaserActive) {
        if (currentTime - lastClimbingLaserTime > 1250) {
            float dx = (player.x + player.w / 2) - x;
            float dy = (player.y + player.h / 2) - y;
            float angle = atan2(dy, dx);
            projectiles.push_back(new Laser(x, y, angle));
            lastClimbingLaserTime = currentTime;
        }
    }
    if (burstAttackActive) {
        DWORD elapsed = currentTime - lastBurstTime;
        if (burstWaveCount == 0) { SpawnSwordBurst(); burstWaveCount = 1; }
        else if (burstWaveCount == 1 && elapsed >= 500) { SpawnSwordBurst(); burstWaveCount = 2; }
        else if (burstWaveCount == 2 && elapsed >= 1000) { burstAttackActive = false; }
        return;
    }
    if (orbAttackActive) {
        if (isPhaseThree) { SpawnOrbs(); return; }
        if (orbAttackCount < ORB_ATTACK_TOTAL && currentTime - orbAttackLastTime > ORB_ATTACK_INTERVAL) {
            orbAttackLastTime = currentTime; orbAttackCount++; SpawnOrbs();
        }
        if (orbAttackCount >= ORB_ATTACK_TOTAL) orbAttackActive = false;
        return;
    }
    if (swordAttackActive) {
        if (swordAttackType == 1) {
            if (swordAttackCount < SWORD_ATTACK_TOTAL && currentTime - swordAttackLastTime > SWORD_ATTACK_INTERVAL_HORIZON) {
                swordAttackLastTime = currentTime; swordAttackCount++; SpawnSwordWallHorizontal(swordAttackFromLeft);
            }
        }
        else if (swordAttackType == 2) {
            if (swordAttackCount < SWORD_ATTACK_TOTAL && currentTime - swordAttackLastTime > SWORD_ATTACK_INTERVAL_VERTICAL) {
                swordAttackLastTime = currentTime; swordAttackCount++; SpawnSwordWallVertical();
            }
        }
        if (swordAttackCount >= SWORD_ATTACK_TOTAL) swordAttackActive = false;
        return;
    }
    if (laserAttackActive) {
        DWORD elapsed = currentTime - lastLaserTime;
        if (elapsed > 500 && laserWaveCount < 3) {
            SpawnLaserBurst(); lastLaserTime = currentTime; laserWaveCount++;
        }
        if (laserWaveCount >= 3 && elapsed > 500) laserAttackActive = false;
    }
}
