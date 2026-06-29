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
    lastAttackKey = false;
}

void Player::update() {
    DWORD currentTime = GetTickCount();

    if (onGround) hasDashedInAir = false;
    if (hurtTimer > 0) hurtTimer--;

    if (!canShadowDash && currentTime - lastShadowDashTime > SHADOW_DASH_COOLDOWN)
        canShadowDash = true;

    for (int i = 0; i < ghosts.size(); i++) {
        ghosts[i].life -= 15;
        if (ghosts[i].life <= 0) {
            ghosts.erase(ghosts.begin() + i);
            i--;
        }
    }

    if (isDashing) {
        if (currentTime - dashStartTime > DASH_DURATION) {
            isDashing = false;
            isShadowDash = false;
            vx = 0;
        }
        else {
            vx = facing * DASH_SPEED;
            vy = 0;
            isInvincible = isShadowDash || (hurtTimer > 0);
        }
    }
    else {
        isInvincible = false;
        if (hurtTimer > 0) isInvincible = true;
        if (!canDash && currentTime - lastNormalDashTime > NORMAL_DASH_COOLDOWN)
            canDash = true;
    }

    if (!isDashing) {
        int inputDir = 0;
        if (GetAsyncKeyState(VK_LEFT)) inputDir = -1;
        else if (GetAsyncKeyState(VK_RIGHT)) inputDir = 1;

        float t = 0.0f;
        if (MOVE_SPEED > 0) t = sqrt(fabs(vx) / MOVE_SPEED);
        if (t > 1.0f) t = 1.0f;
        float delta = 0.25f;

        if (inputDir != 0) {
            if (inputDir != facing && t > 0.01f) {
                t -= delta;
                if (t < 0.0f) { t = 0.0f; facing = inputDir; }
            }
            else {
                facing = inputDir;
                t += delta;
                if (t > 1.0f) t = 1.0f;
            }
        }
        else {
            t -= delta;
            if (t < 0.0f) t = 0.0f;
        }
        vx = facing * MOVE_SPEED * (t * t);

        bool jumpKey = (GetAsyncKeyState('A') & 0x8000) != 0;
        if (jumpKey && !lastJumpKey && jumpCount < MAX_JUMP) {
            vy = JUMP_FORCE;
            onGround = false;
            jumpCount++;
        }
        lastJumpKey = jumpKey;
    }

    if (GetAsyncKeyState('P') & 1) debug_mode = !debug_mode;

    if (GetAsyncKeyState('D')) {
        bool canNormal = (currentTime - lastNormalDashTime > NORMAL_DASH_COOLDOWN);
        if (!isDashing && !isAttacking && canNormal && (!hasDashedInAir || onGround)) {
            isDashing = true;
            dashStartTime = currentTime;
            lastNormalDashTime = currentTime;
            if (!onGround) hasDashedInAir = true;
            if (canShadowDash) {
                isShadowDash = true;
                canShadowDash = false;
                lastShadowDashTime = currentTime;
            }
            else {
                isShadowDash = false;
                isInvincible = false;
            }
        }
    }

    bool attackKey = (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState('J') & 0x8000);
    if (attackKey && !lastAttackKey && !isAttacking) {
        isAttacking = true;
        attackStartTime = currentTime;
        atkTimer = atkDuration;
        if (GetAsyncKeyState(VK_UP)) attackDir = 1;
        else if (GetAsyncKeyState(VK_DOWN)) attackDir = 2;
        else attackDir = 0;
    }
    lastAttackKey = attackKey;

    if (isAttacking) {
        atkTimer--;
        if (atkTimer <= 0) isAttacking = false;
        else {
            float atkLen = h * 2.5f;
            if (attackDir == 1)
                attackBox = { x + 0.1f * w, y - atkLen, (float)w * 0.8f, atkLen };
            else if (attackDir == 2)
                attackBox = { x + 0.1f * w, y + h, (float)w * 0.8f, atkLen };
            else
                attackBox = { facing == 1 ? x + w : x - atkLen, y - h * 0.25f, atkLen, h * 1.5f };
        }
    }

    if (!isDashing) vy += GRAVITY * g_deltaTime * 60.0f;
    x += vx * g_deltaTime * 60.0f;

    Rect pRect = getHitbox();
    for (const auto& wall : platforms) {
        if (pRect.checkCollision(wall)) {
            if (vx > 0) x = wall.x - w;
            else if (vx < 0) x = wall.x + wall.w;
            vx = 0;
            break;
        }
    }

    y += vy * g_deltaTime * 60.0f;
    onGround = false;
    pRect = getHitbox();
    for (const auto& wall : platforms) {
        if (pRect.checkCollision(wall)) {
            if (vy > 0) {
                y = wall.y - h;
                onGround = true;
                jumpCount = 0;
            }
            else if (vy < 0) {
                y = wall.y + wall.h;
                vy = 1.0f;
            }
            if (vy > 0) vy = 0;
            break;
        }
    }

    if (isDashing) {
        if (currentTime - lastGhostTime > 30) {
            ghosts.push_back({ x, y, 255 });
            lastGhostTime = currentTime;
        }
    }

    if (y > currentLevelBottom - 25) {
        hp--;
        hurtTimer = HURT_DURATION;
        isInvincible = true;
        if (currentLevelBottom <= -4400) { x = 450; y = -4650; }
        else if (currentLevelBottom <= 400) { x = 550; y = 100; }
        else { x = WINDOW_W / 2; y = PLATFORM_Y - h - 100; }
        vx = 0; vy = 0;
        jumpCount = 0;
    }
}

void Player::draw() {
    // ghost shadows
    for (const auto& g : ghosts) {
        float alpha = g.life / 255.0f;
        if (isShadowDash || (g.life > 100)) {
            Color c = Fade(COLOR_BG, 1.0f - alpha);
            glSetColor(c);
        }
        else {
            float r = 1.0f * alpha + COLOR_BG.r * (1.0f - alpha);
            float gv = 1.0f * alpha + COLOR_BG.g * (1.0f - alpha);
            float b = 1.0f * alpha + COLOR_BG.b * (1.0f - alpha);
            glSetColor(Color(r, gv, b));
        }
        drawFilledRect(g.x - cameraX + 3, g.y - cameraY + 3, w - 6, h - 6);
    }

    // body
    if (isShadowDash) glSetColor(COLOR_BLACK);
    else if (isDashing) glSetColor(COLOR_WHITE);
    else if (hurtTimer > 0 && (hurtTimer / 4) % 2 == 0) glSetColor(MakeColor(100, 50, 50));
    else glSetColor(COLOR_KNIGHT);

    drawFilledRect(x - cameraX, y - cameraY, w, h);

    // shadow dash indicator
    if (canShadowDash) {
        glSetColor(COLOR_BLACK);
        drawFilledCircle(x - cameraX + w / 2, y - cameraY - 8, 3);
    }

    // attack effects
    if (isAttacking) {
        glSetColor(COLOR_WHITE);
        float ratio = (float)atkTimer / atkDuration;
        float reach = h * 2.5f;
        float baseSpread = (attackDir == 0) ? h : w;
        float halfSpread = baseSpread / 2.0f;
        float maxThickness = h * 1.1f;
        float innerReach = reach - maxThickness;

        int px = (int)(x + w / 2), py = (int)(y + h / 2);
        const int SAMPLE_COUNT = 16;
        POINT pts[SAMPLE_COUNT * 2];

        for (int i = 0; i < SAMPLE_COUNT; i++) {
            float t = -1.0f + 2.0f * (float)i / (SAMPLE_COUNT - 1);
            float curveX_Outer = reach * (1 - t * t);
            float curveX_Inner = innerReach * (1 - t * t);
            float curveY = t * halfSpread;

            float drawX_Outer, drawY_Outer, drawX_Inner, drawY_Inner;
            if (attackDir == 0) {
                drawX_Outer = px + curveX_Outer * facing;
                drawY_Outer = py + curveY;
                drawX_Inner = px + curveX_Inner * facing;
                drawY_Inner = py + curveY;
            }
            else if (attackDir == 1) {
                drawX_Outer = px + curveY;
                drawY_Outer = py - curveX_Outer;
                drawX_Inner = px + curveY;
                drawY_Inner = py - curveX_Inner;
            }
            else {
                drawX_Outer = px + curveY;
                drawY_Outer = py + curveX_Outer;
                drawX_Inner = px + curveY;
                drawY_Inner = py + curveX_Inner;
            }

            pts[i].x = (long)drawX_Outer - cameraX;
            pts[i].y = (long)drawY_Outer - cameraY;
            int innerIndex = (SAMPLE_COUNT * 2 - 1) - i;
            pts[innerIndex].x = (long)drawX_Inner - cameraX;
            pts[innerIndex].y = (long)drawY_Inner - cameraY;
        }
        drawFilledPolygon(pts, SAMPLE_COUNT * 2);
    }

    // debug text skipped
}

Rect Player::getHitbox() {
    return { x, y, (float)w, (float)h };
}
