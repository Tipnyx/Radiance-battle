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

// GLFW 窗口指针（全局，供回调使用）
GLFWwindow* window = nullptr;

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
    while (!glfwWindowShouldClose(window)) {
        // 游戏逻辑（不变）
        GameLogic(gameStartTime);

        // 渲染
        glClear(GL_COLOR_BUFFER_BIT);

        DrawEntities();
        DrawPlatform();
        SpikeManager(gameStartTime);
        DrawUI();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // 约 60 FPS（后续可改为 glfwSetTime 精确控帧）
        Sleep(16);
    }

    // --- 清理 ---
    mciSendString(L"close bgm", NULL, 0, NULL);
    glfwTerminate();
    return 0;
}
