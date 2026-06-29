#pragma once
#define NOMINMAX
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

const int WINDOW_W = 1262;
const int WINDOW_H = 780;

const int PLATFORM_W = 1135;
const int PLATFORM_H = 80;
const int PLATFORM_X = (WINDOW_W - PLATFORM_W) / 2;
const int PLATFORM_Y = 700;

struct Color {
    float r, g, b;
    Color(float r = 0, float g = 0, float b = 0) : r(r), g(g), b(b) {}
};

inline Color MakeColor(int rr, int gg, int bb) {
    return Color(rr / 255.0f, gg / 255.0f, bb / 255.0f);
}

const Color COLOR_KNIGHT   = MakeColor(0, 0, 128);
const Color COLOR_ATTACK   = MakeColor(255, 255, 255);
const Color COLOR_PLATFORM = MakeColor(184, 134, 11);
const Color COLOR_ORB      = MakeColor(255, 255, 200);
const Color COLOR_SWORD    = MakeColor(255, 250, 200);
const Color COLOR_BEAM     = MakeColor(255, 255, 224);
const Color COLOR_BG       = MakeColor(135, 206, 235);
const Color COLOR_WHITE    = MakeColor(255, 255, 255);
const Color COLOR_BLACK    = MakeColor(0, 0, 0);
const Color COLOR_RED      = MakeColor(255, 0, 0);
const Color COLOR_GREEN    = MakeColor(0, 255, 0);
const Color COLOR_YELLOW   = MakeColor(255, 255, 0);
const Color COLOR_MAGENTA  = MakeColor(255, 0, 255);
const Color COLOR_GRAY     = MakeColor(200, 200, 200);

inline Color Fade(const Color& c, float a) {
    return Color(c.r * a, c.g * a, c.b * a);
}

extern bool debug_mode;

enum SpikeState { SPIKE_HIDDEN, SPIKE_WARNING, SPIKE_ACTIVE };
extern SpikeState currentSpikeState;
extern bool spikeOnLeft;
extern DWORD spikeTimer;
const int TIME_TO_START_SPIKES = 30000;
const int DURATION_WARNING = 2000;
const int DURATION_ACTIVE = 8000;

extern float cameraX;
extern float cameraY;
extern float currentLevelBottom;
extern float g_deltaTime;
void TriggerScreenShake(float intensity);
void UpdateScreenShake(float dt);


struct Rect {
    float x, y, w, h;
    Rect(float x = 0, float y = 0, float w = 0, float h = 0) : x(x), y(y), w(w), h(h) {}
    bool checkCollision(const Rect& other) const {
        return x < other.x + other.w && x + w > other.x &&
            y < other.y + other.h && y + h > other.y;
    }
};

extern std::vector<Rect> platforms;

// ============================================================
// OpenGL drawing helpers
// ============================================================

inline void glSetColor(const Color& c) {
    glColor3f(c.r, c.g, c.b);
}

inline void glSetLineWidth(float w) {
    glLineWidth(w);
}

inline void drawFilledRect(float x, float y, float w, float h) {
    glRectf(x, y, x + w, y + h);
}
inline void drawFilledRect(const Rect& r) {
    glRectf(r.x, r.y, r.x + r.w, r.y + r.h);
}

inline void drawLineRect(float x, float y, float w, float h) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

inline void drawFilledCircle(float cx, float cy, float r, int segments = 36) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float a = i * 2.0f * 3.14159f / segments;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}

inline void drawLineCircle(float cx, float cy, float r, int segments = 36) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float a = i * 2.0f * 3.14159f / segments;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}

inline void drawFilledEllipse(float cx, float cy, float rx, float ry, int segments = 36) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float a = i * 2.0f * 3.14159f / segments;
        glVertex2f(cx + cos(a) * rx, cy + sin(a) * ry);
    }
    glEnd();
}

inline void drawLineEllipse(float cx, float cy, float rx, float ry, int segments = 36) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float a = i * 2.0f * 3.14159f / segments;
        glVertex2f(cx + cos(a) * rx, cy + sin(a) * ry);
    }
    glEnd();
}

inline void drawLine(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

inline void drawFilledPolygon(POINT* pts, int n) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < n; i++) glVertex2f((float)pts[i].x, (float)pts[i].y);
    glEnd();
}

inline void drawArc(float cx, float cy, float rx, float ry, float startAngle, float sweepAngle, int segments = 24) {
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; i++) {
        float a = startAngle + i * sweepAngle / segments;
        glVertex2f(cx + cos(a) * rx, cy + sin(a) * ry);
    }
    glEnd();
}

inline void drawFilledRoundRect(float x, float y, float w, float h, float r) {
    float x2 = x + w, y2 = y + h;
    glRectf(x + r, y, x2 - r, y2);
    glRectf(x, y + r, x2, y2 - r);
    int segs = 8;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + r, y + r);
    for (int i = 0; i <= segs; i++) {
        float a = 3.14159f + i * (3.14159f / 2) / segs;
        glVertex2f(x + r + cos(a) * r, y + r + sin(a) * r);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x2 - r, y + r);
    for (int i = 0; i <= segs; i++) {
        float a = 3.14159f * 1.5f + i * (3.14159f / 2) / segs;
        glVertex2f(x2 - r + cos(a) * r, y + r + sin(a) * r);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x2 - r, y2 - r);
    for (int i = 0; i <= segs; i++) {
        float a = i * (3.14159f / 2) / segs;
        glVertex2f(x2 - r + cos(a) * r, y2 - r + sin(a) * r);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + r, y2 - r);
    for (int i = 0; i <= segs; i++) {
        float a = 3.14159f / 2 + i * (3.14159f / 2) / segs;
        glVertex2f(x + r + cos(a) * r, y2 - r + sin(a) * r);
    }
    glEnd();
}

inline void drawLineRoundRect(float x, float y, float w, float h, float r) {
    float x2 = x + w, y2 = y + h;
    int segs = 8;
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segs; i++) {
        float a = 3.14159f + i * (3.14159f / 2) / segs;
        glVertex2f(x + r + cos(a) * r, y + r + sin(a) * r);
    }
    for (int i = 0; i <= segs; i++) {
        float a = 3.14159f * 1.5f + i * (3.14159f / 2) / segs;
        glVertex2f(x2 - r + cos(a) * r, y + r + sin(a) * r);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segs; i++) {
        float a = 3.14159f / 2 + i * (3.14159f / 2) / segs;
        glVertex2f(x + r + cos(a) * r, y2 - r + sin(a) * r);
    }
    for (int i = 0; i <= segs; i++) {
        float a = i * (3.14159f / 2) / segs;
        glVertex2f(x2 - r + cos(a) * r, y2 - r + sin(a) * r);
    }
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y + r); glVertex2f(x, y2 - r);
    glVertex2f(x2, y + r); glVertex2f(x2, y2 - r);
    glEnd();
}

struct Texture {
    GLuint id = 0;
    int w = 0, h = 0;

    void create(int width, int height) {
        w = width; h = height;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    bool loadFromFile(const wchar_t* path) {
        Gdiplus::Bitmap bmp(path);
        if (bmp.GetLastStatus() != Gdiplus::Ok) return false;
        w = bmp.GetWidth(); h = bmp.GetHeight();
        Gdiplus::Rect grect(0, 0, w, h);
        Gdiplus::BitmapData bmpData;
        bmp.LockBits(&grect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmpData.Scan0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        bmp.UnlockBits(&bmpData);
        return true;
    }

    void bind() const { glBindTexture(GL_TEXTURE_2D, id); }
    static void unbind() { glBindTexture(GL_TEXTURE_2D, 0); }
};

inline void drawTextureRect(float x, float y, float w, float h, const Texture& tex) {
    if (!tex.id) return;
    glEnable(GL_TEXTURE_2D);
    tex.bind();
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x, y);
    glTexCoord2f(1, 0); glVertex2f(x + w, y);
    glTexCoord2f(1, 1); glVertex2f(x + w, y + h);
    glTexCoord2f(0, 1); glVertex2f(x, y + h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    Texture::unbind();
}
