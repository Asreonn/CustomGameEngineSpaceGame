#include "raylib.h"
#include <math.h>

#include "player.h"
#include "planet.h"
#include "asteroids.h"

static Texture2D CreateBeamTexture(int width, int height)
{
    Image image = GenImageColor(width, height, BLANK);
    float half_h = height * 0.5f;
    float max_x = (float)(width - 1);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float u = (float)x / max_x;
            float taper = sinf(u * PI);
            float half_w = half_h * (0.15f + 0.85f * taper);
            float dy = fabsf((float)y - half_h) / (half_w + 0.001f);
            float body = 1.0f - dy;
            if (body < 0.0f) body = 0.0f;

            float fade_in = (u < 0.12f) ? (u / 0.12f) : 1.0f;
            float fade_out = (u > 0.88f) ? ((1.0f - u) / 0.12f) : 1.0f;
            float end_fade = fade_in * fade_out;

            float core = 1.0f - (dy * 1.6f);
            if (core < 0.0f) core = 0.0f;

            float alpha = body * body * end_fade;
            unsigned char a = (unsigned char)(alpha * 255.0f);
            Color color = (Color){ 80, 180, 255, a };
            Color core_color = (Color){ 210, 240, 255, a };
            Color mixed = ColorLerp(color, core_color, core);
            ImageDrawPixel(&image, x, y, mixed);
        }
    }

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    const float mapWidth = 5000.0f;
    const float mapHeight = 3000.0f;

    InitWindow(screenWidth, screenHeight, "Space Prototype");
    SetTargetFPS(60);

    Texture2D background = LoadTexture("Assets/Textures/Background/Space Background.png");
    Texture2D beamTex = CreateBeamTexture(240, 64);

    Player player;
    Player_Init(&player, (Vector2){ mapWidth * 0.5f, mapHeight * 0.5f });

    Planet planet;
    Planet_Init(&planet, (Vector2){ mapWidth * 0.5f, mapHeight * 0.3f }, 0.6f);

    AsteroidSystem asteroids;
    Asteroids_Init(&asteroids, "Assets/Textures/Asteroids/Stone");

    Camera2D camera = {0};
    camera.offset = (Vector2){ screenWidth * 0.5f, screenHeight * 0.5f };
    camera.target = player.position;
    camera.zoom = 1.0f;

    Rectangle mapBounds = {0.0f, 0.0f, mapWidth, mapHeight};
    const float beamRange = 180.0f;
    const float beamDps = 30.0f;
    const float beamScale = 0.5f;
    const float beamStepScale = 0.5f;
    float popupTimer = 0.0f;
    float popupInterval = 0.18f;
    int beamActive = 0;
    Vector2 beamTargetPos = {0};
    Vector2 beamEndPos = {0};
    float beamTargetDist = 0.0f;
    float beamTargetRadius = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Player_Update(&player, dt, camera, mapBounds);
        Planet_Update(&planet, dt);
        Asteroids_Update(&asteroids, dt, camera, player.position);
        camera.target = player.position;
        popupTimer -= dt;

        int targetIndex = Asteroids_FindClosest(&asteroids, player.position, beamRange, &beamTargetDist);
        beamActive = (targetIndex >= 0);
        if (beamActive)
        {
            Asteroids_GetInfo(&asteroids, targetIndex, &beamTargetPos, &beamTargetRadius);
            float damage = beamDps * dt;
            int destroyed = Asteroids_ApplyDamage(&asteroids, targetIndex, damage);

            if (popupTimer <= 0.0f)
            {
                Asteroids_AddPopup(&asteroids, beamTargetPos, damage * 10.0f);
                popupTimer = popupInterval;
            }

            if (destroyed) beamActive = 0;
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
        {
            camera.zoom += wheel * 0.1f;
            if (camera.zoom < 0.2f) camera.zoom = 0.2f;
            if (camera.zoom > 2.5f) camera.zoom = 2.5f;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);

        // Draw tiled background across the view for an endless feel.
        Vector2 topLeft = GetScreenToWorld2D((Vector2){0, 0}, camera);
        Vector2 bottomRight = GetScreenToWorld2D((Vector2){(float)screenWidth, (float)screenHeight}, camera);
        int startX = (int)floorf(topLeft.x / background.width) - 1;
        int endX = (int)floorf(bottomRight.x / background.width) + 1;
        int startY = (int)floorf(topLeft.y / background.height) - 1;
        int endY = (int)floorf(bottomRight.y / background.height) + 1;

        for (int y = startY; y <= endY; y++)
        {
            for (int x = startX; x <= endX; x++)
            {
                DrawTexture(background, x * background.width, y * background.height, WHITE);
            }
        }

        DrawRectangleLinesEx(mapBounds, 2.0f, Fade(SKYBLUE, 0.5f));
        if (beamActive && beamTex.id != 0)
        {
            Vector2 dir = { beamTargetPos.x - player.position.x, beamTargetPos.y - player.position.y };
            float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (dist > 0.01f)
            {
                dir.x /= dist;
                dir.y /= dist;
                float end_dist = dist - beamTargetRadius;
                if (end_dist < 12.0f) end_dist = 12.0f;
                beamEndPos = (Vector2){
                    player.position.x + dir.x * end_dist,
                    player.position.y + dir.y * end_dist
                };
                float angle = atan2f(dir.y, dir.x) * RAD2DEG;
                float step = beamTex.width * beamScale * beamStepScale;
                float startOffset = player.size.y * 0.65f;
                Vector2 start = {
                    player.position.x + dir.x * startOffset,
                    player.position.y + dir.y * startOffset
                };
                float beam_len = end_dist - startOffset;
                if (beam_len < 0.0f) beam_len = 0.0f;
                int count = (int)ceilf(beam_len / step);
                if (count < 1) count = 1;
                float actual_step = beam_len / (float)count;
                Rectangle src = {0, 0, (float)beamTex.width, (float)beamTex.height};
                Vector2 origin = { (beamTex.width * beamScale) * 0.5f, (beamTex.height * beamScale) * 0.5f };
                for (int i = 0; i <= count; i++)
                {
                    Vector2 pos = {
                        start.x + dir.x * actual_step * i,
                        start.y + dir.y * actual_step * i
                    };
                    Rectangle dst = { pos.x, pos.y, beamTex.width * beamScale, beamTex.height * beamScale };
                    DrawTexturePro(beamTex, src, dst, origin, angle, WHITE);
                    DrawTexturePro(beamTex, src, dst, origin, angle, Fade((Color){160, 220, 255, 255}, 0.35f));
                }
            }
        }
        Planet_Draw(&planet);
        Asteroids_Draw(&asteroids);
        Player_Draw(&player, camera);

        EndMode2D();

        DrawText("WASD or arrows to move", 20, 20, 20, RAYWHITE);
        DrawText("Hold RMB to boost", 20, 44, 18, RAYWHITE);
        DrawText("Mouse wheel to zoom", 20, 66, 18, RAYWHITE);
        DrawText("Map boundary shown in blue", 20, 88, 18, RAYWHITE);

        EndDrawing();
    }

    Planet_Unload(&planet);
    Player_Unload(&player);
    Asteroids_Unload(&asteroids);
    UnloadTexture(background);
    UnloadTexture(beamTex);

    CloseWindow();
    return 0;
}
