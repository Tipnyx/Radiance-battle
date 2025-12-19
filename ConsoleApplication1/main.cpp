#include <graphics.h>
#include <conio.h>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>
#include <mmsystem.h>

#include"common.h"
#include"player.h"
#include"projectile.h"
#include"world.h"
#include"boss.h"
#include"GameManager.h"

#pragma comment(lib,"winmm.lib")

bool debug_mode = false; // 全局调试开关

SpikeState currentSpikeState = SPIKE_HIDDEN;
bool spikeOnLeft = true;            // 当前在哪一侧
DWORD spikeTimer = 0;               // 用于计算状态持续时间的状态计时器

Boss boss;
Player player;
std::vector<Projectile*> projectiles;

int waveCount = 0;

float cameraX = 0;
float cameraY = 0;

int main() {
    DWORD gameStartTime = GetTickCount();
	printf("游戏开始时间：%lu ms\n", gameStartTime);
    initgraph(WINDOW_W, WINDOW_H); // 初始化窗口

    MCIERROR err = mciSendString(L"open \"output.mp3\" type mpegvideo alias bgm", NULL, 0, NULL); // 播放背景音乐
    // MCI错误处理
    if (err != 0) {
        wchar_t errMsg[256];
        mciGetErrorString(err, errMsg, 256);
        MessageBoxW(NULL, errMsg, L"MCI Error", MB_OK);
    }
    mciSendString(L"play bgm repeat", NULL, 0, NULL);

    srand((unsigned int)time(NULL));
    BeginBatchDraw();
    IMAGE img_bg;
	loadimage(&img_bg, _T("background.png"), WINDOW_W + 200, WINDOW_H);
    //putimage((int)(-cameraX * 0.3f), 0, &img_bg);

    while (true) {
		// 1. 主逻辑更新
		GameLogic(gameStartTime);
        // 2. 绘图
        cleardevice();

        putimage((int)(-cameraX * 0.3f - 100), 0, &img_bg);
        DrawEntities(); // 画实体
        DrawPlatform(); //画平台
        SpikeManager(gameStartTime);  //地刺管理，15秒后开始生成，左右半区来回切换
        DrawUI(); // 画血条
        FlushBatchDraw(); // 批量绘图
        Sleep(16); // 约60帧
    }

    EndBatchDraw();
    mciSendString(L"close bgm", NULL, 0, NULL);
    closegraph();
    return 0;
}
