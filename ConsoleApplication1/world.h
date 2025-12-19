#pragma once
#include "common.h"
#include<ctime>

void GenerateStairs();

void InitPlatform();

void DrawSpikes(float x, float y, float w, float h);

void DrawSinglePlatform(const Rect& r);
void DrawPlatform();

void DrawUI();

void LastSpike();
void SpikeManager(DWORD gameStartTime);