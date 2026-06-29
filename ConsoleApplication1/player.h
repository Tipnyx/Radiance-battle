#pragma once
#include "common.h"

// �����
class Player {
private:
    const float GRAVITY = 1.0f;  //����
    const float MOVE_SPEED = 6.0f; // ˮƽ�ƶ��ٶ�
    const float JUMP_FORCE = -18.0f; // ��Ծ���ٶ�
    const float DASH_SPEED = 15.0f; // ����ٶ�
    const int DASH_DURATION = 300; // ms
    const int SHADOW_DASH_COOLDOWN = 1500; // ��Ӱ�����ȴ (1.5s)
    const int NORMAL_DASH_COOLDOWN = 400;  // ��ͨ�����ȴ (����)
public:
    float x, y;
    float vx, vy;
    int w = 40, h = 65;

    bool onGround = false;
    int facing = 1; // 1 Right, -1 Left

    // �޵�֡
    int hurtTimer = 0;            // �����޵е���ʱ
    const int HURT_DURATION = 60; // ���˺��޵� 1�� (Լ60֡)

    // ������
    int jumpCount = 0; //��ǰ��Ծ����
    const int MAX_JUMP = 2; // �����Ծ��������������
    bool lastJumpKey = false; //��һ֡��Ծ��״̬

    // ������
    bool isDashing = false;
    bool isShadowDash = false; // ���ֵ�ǰ�Ƿ�Ϊ��Ӱ���
    bool canShadowDash = true; // ��Ӱ��̾���״̬
    bool canDash = true;
    DWORD dashStartTime = 0;
    DWORD lastShadowDashTime = 0;
    bool hasDashedInAir; // ��¼�Ƿ��Ѿ��ڿ��г�̹�
    DWORD lastNormalDashTime = 0;

    // �������
    bool isAttacking = false;
    bool lastAttackKey = false;
    DWORD attackStartTime = 0;
    int attackDir = 0; // 0:Side, 1:Up, 2:Down
    Rect attackBox;
    int atkTimer = 0;         // ��ǰ������ʱ����֡����
    const int atkDuration = 10; // ������������֡�� (Լ0.16��)

    // ״̬
    int hp = 10;
    bool isInvincible = false; // �޵�֡

    // --- ��Ӱ��̲�Ӱ��� ---
    struct ShadowGhost {
        float x, y;
        int life; // �������� 255 ���� 0
    };

    std::vector<ShadowGhost> ghosts;
    DWORD lastGhostTime = 0; // ���Ʋ�Ӱ����Ƶ��

    // ����������ֻд���֣���д����������߼���
    Player();
    
    void reset();
    void update();
    void draw();
    Rect getHitbox();
};

extern Player player;