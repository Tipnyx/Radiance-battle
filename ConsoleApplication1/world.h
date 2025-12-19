#pragma once
#include "common.h"
#include<ctime>

void DrawSpikes(float x, float y, float w, float h);

void DrawPlatform();

void DrawUI();

void LastSpike();
void SpikeManager(DWORD gameStartTime);