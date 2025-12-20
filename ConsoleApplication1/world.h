#pragma once
#include "common.h"
#include<ctime>

void GenerateStairs(); //二阶段台阶
void GenerateUPStairs(); //攀爬阶段向上台阶

void InitPlatform();

void DrawSpikes(float x, float y, float w, float h);

void DrawSinglePlatform(const Rect& r);
void DrawPlatform();

void DrawUI();

void LastSpike();
void SpikeManager(DWORD gameStartTime);