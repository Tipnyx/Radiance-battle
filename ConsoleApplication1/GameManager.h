#pragma once
#include"common.h"

class Player;

void GameLogic(DWORD& gameStartTime);
void GameReset(DWORD& gameStartTime);
void AttackBoss();
void DrawEntities();
void ProjectileManager();
void UpdateCamera(Player& p);