#pragma once
#include <graphics.h>
#include <vector>

// --- 常量定义 ---
const int WINDOW_W = 1262;
const int WINDOW_H = 780;
const int PLATFORM_Y = 718;
const int PLATFORM_W = 1135;
const int PLATFORM_H = 62;
const int PLATFORM_X = (WINDOW_W - PLATFORM_W) / 2;

// 颜色
const COLORREF COLOR_KNIGHT = RGB(0, 0, 128);       // 海军蓝
const COLORREF COLOR_ATTACK = WHITE;                // 攻击白
const COLORREF COLOR_PLATFORM = RGB(184, 134, 11);  // 棕黄
const COLORREF COLOR_ORB = RGB(255, 255, 200);      // 浅黄
const COLORREF COLOR_SWORD = RGB(255, 250, 200);    // 亮黄
const COLORREF COLOR_BEAM = RGB(255, 255, 224);     // 光束色
const COLORREF COLOR_BG = RGB(135, 206, 235); // 天空蓝

// 物理常量
const float GRAVITY = 1.0f;
const float MOVE_SPEED = 6.0f;
const float JUMP_FORCE = -17.0f;
const float DASH_SPEED = 15.0f;

const int ORB_ATTACK_TOTAL = 3;    // 一次攻击发射3个
const int ORB_ATTACK_INTERVAL = 1500; // 每1.5秒发射一个

const int SWORD_ATTACK_TOTAL = 3; //一次三波
const int SWORD_ATTACK_INTERVAL_VERTICAL = 1000; //每1.5秒发射一波
const int SWORD_ATTACK_INTERVAL_HORIZON = 1250; //每1.5秒发射一波

const int TOTAL_BURST_WAVES = 2; // 总共两波

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

// 结构体：矩形碰撞箱
struct Rect {
    float x, y, w, h; // x,y是矩形的左上角坐标
    // AABB碰撞检测
    bool checkCollision(const Rect& other) {
        return x < other.x + other.w && x + w > other.x &&
            y < other.y + other.h && y + h > other.y;
    }
};

extern std::vector<Rect> platforms; // 全局平台列表
