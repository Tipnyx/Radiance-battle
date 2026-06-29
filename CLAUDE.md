# Radiance — 2D Boss Fight Game (OpenGL)

## Build & Run

```bash
# Dependencies (one-time)
vcpkg install    # glfw3 + glew

# Build
Open Radiance.sln in Visual Studio 2022 → Debug x64 → Build

# Manifest baseline if vcpkg complains
# Current baseline: a0400024711b283056538ac19ced80b91a83c24c
```

| Setting | Value |
|---------|-------|
| IDE | Visual Studio 2022 (v143) |
| Platform | x64 only |
| Encoding | `/utf-8` (required!) |
| Package manager | vcpkg (manifest mode) |
| Dependencies | `glfw3`, `glew` |
| Audio | Windows MCI (`winmm.lib`) |

## Project Structure

```
Radiance/
  ConsoleApplication1/     ← main game project
    main.cpp               ← entry point, GLFW init, game loop
    common.h               ← OpenGL drawing helpers, Color struct, Rect
    player.h / player.cpp  ← Player class (physics, input, HP)
    boss.h / boss.cpp      ← Boss class (AI states, attack spawning)
    BossState.h / BossState.cpp ← Boss state machine (Phase 1/2/3)
    projectile.h / projectile.cpp ← Orb, Sword, Beam, Laser projectiles
    world.h / world.cpp    ← Platforms, spikes, camera, UI
    GameManager.h / GameManager.cpp ← Game logic, collision, camera update
    boss.cpp               ← Clean version without FBO textures
  vcpkg.json               ← glfw3 + glew + baseline
  vcpkg_installed/         ← vcpkg packages (auto-generated)
  assets/                  ← Game assets (background.png, output.mp3, etc.)
  opengltest/              ← Unused test project → unload from solution
```

## OpenGL Rendering (replaced EasyX)

All EasyX calls migrated to OpenGL. See `common.h` for drawing helpers:

| EasyX | OpenGL replacement |
|-------|-------------------|
| `initgraph()` | `glfwCreateWindow()` |
| `cleardevice()` | `glClear()` |
| `BeginBatchDraw()/FlushBatchDraw()` | `glfwSwapBuffers()` |
| `solidrectangle()` | `drawFilledRect()` |
| `solidcircle()` | `drawFilledCircle()` |
| `solidellipse()` | `drawFilledEllipse()` |
| `solidroundrect()` | `drawFilledRoundRect()` |
| `solidpolygon()` | `drawFilledPolygon()` |
| `putimage()` / `IMAGE` | Not yet migrated (see TODO) |

Coordinate system: top-left origin, same as EasyX (`glOrtho(0, W, H, 0, -1, 1)`).

## Key Types

- `Color(r, g, b)` — float RGB struct, use `MakeColor(r,g,b)` for byte values
- `Rect(x, y, w, h)` — AABB collision rect
- `Texture` — OpenGL texture wrapper (GLuint id + dimensions)
- `Fade(Color, alpha)` — color with alpha blending

## TODO

1. **Background image** — Load `background.png` with `stb_image.h`, render as fullscreen texture
2. **Boss texture** — Sun gradient texture via FBO or pre-baked PNG
3. **Text rendering** — HP display and "YOU DIED" text (use FreeType or bitmap font)
4. **Sprite textures** — Replace colored shapes for player/boss/projectiles
5. **Precise frame timing** — Replace `Sleep(16)` with `glfwSetTime` / delta-time
6. **Unload `opengltest` project** from solution

## Known Issues

- **Encoding**: All source files must be UTF-8. VS project has `/utf-8` flag. If Chinese comments cause phantom errors, check this flag is present.
- **DLLs**: Post-build event copies `glfw3.dll` + `glew32.dll` to output dir. If "missing DLL" at runtime, check vcxproj PostBuildEvent.
- **vcpkg paths**: Only configured for x64. Building as Win32 will fail.
- **mmsystem.h**: Must be included AFTER `<windows.h>` and BEFORE `<GL/glew.h>`. Handled in `main.cpp`.
- **APIENTRY conflict**: `glfw3.h` and Windows SDK both define this. Managed by include order.
