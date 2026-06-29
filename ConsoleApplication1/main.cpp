#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>

#include "common.h"
#include "player.h"
#include "projectile.h"
#include "world.h"
#include "boss.h"
#include "GameManager.h"

#pragma comment(lib, "winmm.lib")

bool debug_mode = false;

SpikeState currentSpikeState = SPIKE_HIDDEN;
bool spikeOnLeft = true;
DWORD spikeTimer = 0;

Boss boss;
Player player;
std::vector<Projectile*> projectiles;

int waveCount = 0;

float cameraX = 0;
float cameraY = 0;
float currentLevelBottom = WINDOW_H;
float g_deltaTime = 1.0f / 60.0f;
float g_shakeDur = 0, g_shakeInt = 0;
float g_shakeX = 0, g_shakeY = 0;

void TriggerScreenShake(float intensity) {
    g_shakeInt = intensity; g_shakeDur = 0.25f;
}
void UpdateScreenShake(float dt) {
    if (g_shakeDur > 0) {
        g_shakeDur -= dt;
        float d = g_shakeDur / 0.25f;
        g_shakeX = (float)((rand()%100)-50)/50.0f * g_shakeInt * d;
        g_shakeY = (float)((rand()%100)-50)/50.0f * g_shakeInt * d;
        if (g_shakeDur <= 0) { g_shakeX=g_shakeY=0; g_shakeInt=0; }
    }
}

GLFWwindow* window = nullptr;
Texture g_bgTex;
ULONG_PTR g_gdiplusToken = 0;

// --- OpenGL 初始化 ---
void InitOpenGL() {
    // 设置 2D 正交投影，左上角为原点（与 EasyX 一致）
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_W, WINDOW_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 启用混合（用于半透明）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 背景色
    glClearColor(COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 1.0f);
}

int main() {
    // GDI+ init
    Gdiplus::GdiplusStartupInput gdiSI;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiSI, NULL);

    // --- 初始化 GLFW ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Radiance", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // --- 初始化 GLEW ---
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: "
                  << glewGetErrorString(glewErr) << std::endl;
        return -1;
    }

    InitOpenGL();

    // Load background
    if (!g_bgTex.loadFromFile(L"background.png"))
        g_bgTex.loadFromFile(L"assets/scene/background.png");
    boss.InitSunCache();
    boss.InitHitCache();

    // --- 音频（保持 Windows MCI，不受 EasyX 影响） ---
    MCIERROR err = mciSendString(L"open \"output.mp3\" type mpegvideo alias bgm", NULL, 0, NULL);
    if (err != 0) {
        wchar_t errMsg[256];
        mciGetErrorString(err, errMsg, 256);
        MessageBoxW(NULL, errMsg, L"MCI Error", MB_OK);
    }
    mciSendString(L"play bgm repeat", NULL, 0, NULL);

    // --- 游戏初始化 ---
    DWORD gameStartTime = GetTickCount();
    printf("Game start time: %lu ms\n", gameStartTime);
    srand((unsigned int)time(NULL));
    InitPlatform();

    // --- 主循环 ---
    glfwSwapInterval(1);
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        g_deltaTime = (float)(now - lastTime);
        lastTime = now;
        if (g_deltaTime > 0.1f) g_deltaTime = 0.1f;

        GameLogic(gameStartTime);

        UpdateScreenShake(g_deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);

        if (g_shakeDur > 0) { glMatrixMode(GL_PROJECTION); glPushMatrix(); glTranslatef(g_shakeX, g_shakeY, 0); }

        // Static background (fixed, fills entire screen)
        if (g_bgTex.id) {
            float bw = (float)g_bgTex.w, bh = (float)g_bgTex.h;
            float scale = (float)WINDOW_H / bh;
            float sw = bw * scale;
            float px = (WINDOW_W - sw) / 2.0f;
            drawTextureRect(px, 0, sw, (float)WINDOW_H, g_bgTex);
        }

        DrawEntities();
        DrawPlatform();
        SpikeManager(gameStartTime);
        DrawUI();

        if (g_shakeDur > 0) { glMatrixMode(GL_PROJECTION); glPopMatrix(); }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 清理 ---
    mciSendString(L"close bgm", NULL, 0, NULL);
    Gdiplus::GdiplusShutdown(g_gdiplusToken);
    glfwTerminate();
    return 0;
}
