#include "raylib.h"
#include <math.h>

#include "player.h"
#include "planet.h"
#include "asteroids.h"

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    const float mapWidth = 5000.0f;
    const float mapHeight = 3000.0f;

    InitWindow(screenWidth, screenHeight, "Space Prototype");
    SetTargetFPS(60);

    Texture2D background = LoadTexture("Assets/Textures/Background/Space Background.png");
    Texture2D beamHeadTex = LoadTexture("Assets/Textures/Lasers/Laser Sprites/04.png");
    Texture2D beamBodyTex = LoadTexture("Assets/Textures/Lasers/Laser Sprites/23.png");

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
    const float beamBodyScale = 0.75f;
    const float beamHeadScale = 0.65f;
    const float beamStepScale = 0.55f;
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
        if (beamActive && beamBodyTex.id != 0 && beamHeadTex.id != 0)
        {
            Vector2 dir = { beamTargetPos.x - player.position.x, beamTargetPos.y - player.position.y };
            float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (dist > 0.01f)
            {
                dir.x /= dist;
                dir.y /= dist;
                float clamped_dist = (dist > beamRange) ? beamRange : dist;
                float end_dist = clamped_dist - (beamTargetRadius * 0.95f);
                if (end_dist < 12.0f) end_dist = 12.0f;
                float angle = atan2f(dir.y, dir.x) * RAD2DEG;
                float body_w = beamBodyTex.width * beamBodyScale;
                float body_h = beamBodyTex.height * beamBodyScale;
                float head_w = beamHeadTex.width * beamHeadScale;
                float head_h = beamHeadTex.height * beamHeadScale;
                float head_forward = head_w * 0.5f;

                float nose_offset = player.size.y * 0.5f;
                float start_dist = nose_offset + body_w * 0.5f;
                float head_center_dist = end_dist - head_forward * 0.9f;
                if (head_center_dist < start_dist) head_center_dist = start_dist;
                float body_end_dist = head_center_dist - body_w * 0.5f;
                float beam_len = body_end_dist - start_dist;

                if (beam_len < 0.0f) beam_len = 0.0f;
                float step = body_w * beamStepScale;
                int count = (int)ceilf(beam_len / step);
                if (count < 1) count = 1;
                float actual_step = (count > 0) ? (beam_len / (float)count) : 0.0f;

                Rectangle body_src = {0, 0, (float)beamBodyTex.width, (float)beamBodyTex.height};
                Vector2 body_origin = { body_w * 0.5f, body_h * 0.5f };
                for (int i = 0; i <= count; i++)
                {
                    float dist_along = start_dist + actual_step * i;
                    Vector2 pos = {
                        player.position.x + dir.x * dist_along,
                        player.position.y + dir.y * dist_along
                    };
                    Rectangle body_dst = { pos.x, pos.y, body_w, body_h };
                    DrawTexturePro(beamBodyTex, body_src, body_dst, body_origin, angle, WHITE);
                    DrawTexturePro(beamBodyTex, body_src, body_dst, body_origin, angle, Fade((Color){180, 210, 255, 255}, 0.35f));
                }

                beamEndPos = (Vector2){
                    player.position.x + dir.x * head_center_dist,
                    player.position.y + dir.y * head_center_dist
                };
                Rectangle head_src = {0, 0, (float)beamHeadTex.width, (float)beamHeadTex.height};
                Rectangle head_dst = { beamEndPos.x, beamEndPos.y, head_w, head_h };
                Vector2 head_origin = { head_w * 0.5f, head_h * 0.5f };
                DrawTexturePro(beamHeadTex, head_src, head_dst, head_origin, angle, WHITE);
                DrawTexturePro(beamHeadTex, head_src, head_dst, head_origin, angle, Fade((Color){200, 230, 255, 255}, 0.35f));
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
    UnloadTexture(beamHeadTex);
    UnloadTexture(beamBodyTex);

    CloseWindow();
    return 0;
}
