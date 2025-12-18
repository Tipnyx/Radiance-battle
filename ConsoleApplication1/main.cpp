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
DWORD gameGlobalTime = 0;           // 记录游戏开始时间

Boss boss;
Player player;
std::vector<Projectile*> projectiles;

int waveCount = 0;

int main() {

    DWORD gameStartTime = GetTickCount();
    initgraph(WINDOW_W, WINDOW_H); // 初始化窗口

    MCIERROR err = mciSendString(L"open \"output.mp3\" type mpegvideo alias bgm", NULL, 0, NULL); // 播放背景音乐
    // MCI错误处理
    if (err != 0) {
        wchar_t errMsg[256];
        mciGetErrorString(err, errMsg, 256);
        MessageBoxW(NULL, errMsg, L"MCI Error", MB_OK);
    }
    mciSendString(L"play bgm repeat", NULL, 0, NULL);

    srand((unsigned int)time(0));
    BeginBatchDraw();
    setbkcolor(COLOR_BG);

    while (true) {
		// 1. 逻辑更新
		GameLogic(gameStartTime);
        // 2. 绘图
        cleardevice();

        if (debug_mode) {
            setlinecolor(GREEN); // 玩家本体用绿色框
            Rect pRect = player.getHitbox();
            rectangle((int)pRect.x, (int)pRect.y, (int)(pRect.x + pRect.w), (int)(pRect.y + pRect.h));

            if (player.isAttacking) {
                setlinecolor(YELLOW); // 玩家攻击范围用黄色框
                rectangle((int)player.attackBox.x, (int)player.attackBox.y,
                    (int)(player.attackBox.x + player.attackBox.w),
                    (int)(player.attackBox.y + player.attackBox.h));
            }
        }
        // 如果开启了调试模式则绘制红框
        for (auto p : projectiles) {p->draw();if (debug_mode) p->drawDebug();} // 画弹幕
        boss.draw(); //画Boss
        if (player.hp > 0) player.draw(); // 画玩家

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
