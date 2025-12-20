#pragma once
#include <graphics.h>
#include <vector>

// --- 常量定义 ---
const int WINDOW_W = 1262; // 窗口宽度
const int WINDOW_H = 780; // 窗口高度

const int PLATFORM_W = 1135; // 底层平台宽度
const int PLATFORM_H = 80; // 底层平台高度
const int PLATFORM_X = (WINDOW_W - PLATFORM_W) / 2; // 底层平台X坐标
const int PLATFORM_Y = 700; // 底层平台Y坐标

// 颜色
const COLORREF COLOR_KNIGHT = RGB(0, 0, 128);       // 海军蓝
const COLORREF COLOR_ATTACK = WHITE;                // 攻击白
const COLORREF COLOR_PLATFORM = RGB(184, 134, 11);  // 棕黄
const COLORREF COLOR_ORB = RGB(255, 255, 200);      // 浅黄
const COLORREF COLOR_SWORD = RGB(255, 250, 200);    // 亮黄
const COLORREF COLOR_BEAM = RGB(255, 255, 224);     // 光束色
const COLORREF COLOR_BG = RGB(135, 206, 235); // 天空蓝

// 物理常量

extern bool debug_mode; // 全局调试开关

// --- 地刺相关常量与变量 ---
enum SpikeState { SPIKE_HIDDEN, SPIKE_WARNING, SPIKE_ACTIVE };
extern SpikeState currentSpikeState;
extern bool spikeOnLeft;            // 当前在哪一侧
extern DWORD spikeTimer;               // 用于计算状态持续时间的状态计时器
const int TIME_TO_START_SPIKES = 30000; // 30秒后开始
const int DURATION_WARNING = 2000;      // 预警持续 2秒
const int DURATION_ACTIVE = 8000;       // 攻击持续 8秒 (总共10秒一轮)

// --- 摄像机位置 ---
extern float cameraX;
extern float cameraY;
extern float currentLevelBottom; // 当前关卡的最低底线 

// --- 矩形碰撞箱 ---
struct Rect {
    float x, y, w, h; // x,y是矩形的左上角坐标
    // AABB碰撞检测
    bool checkCollision(const Rect& other) {
        return x < other.x + other.w && x + w > other.x &&
            y < other.y + other.h && y + h > other.y;
    }
};

extern std::vector<Rect> platforms; // 全局平台列表

