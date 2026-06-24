#include "raylib.h"
#include "rlgl.h"
#include <math.h>
#include <stdbool.h>

// The build creates optimized media copies in out/generated-assets and links
// them directly into the executable with objcopy (see nobs.full.txt).
extern const unsigned char _binary_bckg_png_start[];
extern const unsigned char _binary_bckg_png_end[];
extern const unsigned char _binary_braccada_jam_ogg_start[];
extern const unsigned char _binary_braccada_jam_ogg_end[];
extern const unsigned char __start_logo[];
extern const unsigned char __stop_logo[];

static const char *SCROLL_TEXT =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat "
    "cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id "
    "est laborum.        ";

int main(void)
{
    const int screenW = 800;
    const int screenH = 450;

    InitWindow(screenW, screenH, "raylib demo");
    InitAudioDevice();
    SetTargetFPS(60);

    int musicSize = (int)(_binary_braccada_jam_ogg_end -
                          _binary_braccada_jam_ogg_start);
    Music music = LoadMusicStreamFromMemory(".ogg",
                                            _binary_braccada_jam_ogg_start,
                                            musicSize);
    music.looping = true;
    PlayMusicStream(music);

    int bgSize = (int)(_binary_bckg_png_end - _binary_bckg_png_start);
    Image bgImg = LoadImageFromMemory(".png", _binary_bckg_png_start, bgSize);
    Texture2D bg = LoadTextureFromImage(bgImg);
    UnloadImage(bgImg);
    SetTextureFilter(bg, TEXTURE_FILTER_BILINEAR);

    int logoSize = (int)(__stop_logo - __start_logo);
    Image logoImg = LoadImageFromMemory(".png", __start_logo, logoSize);
    Texture2D logo = LoadTextureFromImage(logoImg);
    UnloadImage(logoImg);
    SetTextureFilter(logo, TEXTURE_FILTER_BILINEAR);

    // The vertical 16:9 background quad is larger than the viewport so camera
    // movement never reveals black edges.
    const float planeW = 20.0f;
    const float planeH = 11.25f;

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 7.0f };  // Slightly zoomed in.
    camera.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy     = 55.0f;                          // Narrower field of view.
    camera.projection = CAMERA_PERSPECTIVE;

    // Wavy text scroller moving from right to left.
    Font font = GetFontDefault();
    const float fontSize  = 28.0f;
    const float spacing   = 2.0f;
    const float waveAmp   = 16.0f;    // Wave height.
    const float waveFreq  = 0.013f;   // Wave density along the x-axis.
    const float waveSpeed = 4.0f;     // Wave animation speed.
    const float scrollSpeed = 150.0f; // Pixels per second.
    const float pauseTime = 2.0f;     // Delay before restarting.

    const float logoWidth = (float)logo.width;
    const float logoHeight = (float)logo.height;
    const float contentGap = 24.0f;
    const float scrollerHeight = fontSize + waveAmp * 2.0f;
    const float contentHeight = logoHeight + contentGap + scrollerHeight;
    const float contentTop = ((float)screenH - contentHeight) * 0.5f;
    const Vector2 logoPosition = {
        ((float)screenW - logoWidth) * 0.5f,
        contentTop
    };

    // Measure the text using the same per-character method as the draw loop.
    float totalWidth = 0.0f;
    for (int i = 0; SCROLL_TEXT[i] != '\0'; i++)
    {
        char ch[2] = { SCROLL_TEXT[i], '\0' };
        totalWidth += MeasureTextEx(font, ch, fontSize, spacing).x + spacing;
    }

    const float travel = (float)screenW + totalWidth;
    const float baseY = contentTop + logoHeight + contentGap + waveAmp;

    float t = 0.0f;
    float scroll = 0.0f;   // Distance travelled by the start of the text.
    float pauseT = 0.0f;
    bool paused = false;

    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);

        float dt = GetFrameTime();
        t += dt;

        // Slowly pan across the background and gently animate the field of view.
        float sway = sinf(t * 0.4f);
        camera.position.x = sway * 1.4f;
        camera.target.x   = sway * 0.6f;
        camera.fovy = 55.0f + sinf(t * 0.25f) * 1.5f;

        // Scroll from right to left, then pause and restart.
        if (!paused)
        {
            scroll += scrollSpeed * dt;
            if (scroll >= travel) { paused = true; pauseT = 0.0f; }
        }
        else
        {
            pauseT += dt;
            if (pauseT >= pauseTime) { paused = false; scroll = 0.0f; }
        }
        float offset = (float)screenW - scroll;

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
            rlDisableBackfaceCulling();
            rlSetTexture(bg.id);
            rlBegin(RL_QUADS);
                rlColor4ub(255, 255, 255, 255);
                rlNormal3f(0.0f, 0.0f, 1.0f);
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-planeW*0.5f,  planeH*0.5f, 0.0f);
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-planeW*0.5f, -planeH*0.5f, 0.0f);
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f( planeW*0.5f, -planeH*0.5f, 0.0f);
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f( planeW*0.5f,  planeH*0.5f, 0.0f);
            rlEnd();
            rlSetTexture(0);
        EndMode3D();

        // Draw the logo above the scroller as one vertically centered group.
        DrawTextureV(logo, logoPosition, WHITE);

        // Draw the wavy text one character at a time.
        float penX = offset;
        for (int i = 0; SCROLL_TEXT[i] != '\0'; i++)
        {
            char ch[2] = { SCROLL_TEXT[i], '\0' };
            float w = MeasureTextEx(font, ch, fontSize, spacing).x;
            float y = baseY + sinf(penX * waveFreq + t * waveSpeed) * waveAmp;
            if (penX + w >= 0.0f && penX <= (float)screenW)
                DrawTextEx(font, ch, (Vector2){ penX, y }, fontSize, spacing, RAYWHITE);
            penX += w + spacing;
        }

        EndDrawing();
    }

    StopMusicStream(music);
    UnloadMusicStream(music);
    CloseAudioDevice();
    UnloadTexture(logo);
    UnloadTexture(bg);
    CloseWindow();
    return 0;
}
