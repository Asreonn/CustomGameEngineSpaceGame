#include "player.h"
#include <math.h>

static Vector2 NormalizeSafe(Vector2 v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y);
    if (len < 0.0001f) return (Vector2){0.0f, 0.0f};
    return (Vector2){ v.x / len, v.y / len };
}

void Player_Init(Player *player, Vector2 start_pos)
{
    *player = (Player){0};
    player->position = start_pos;
    player->speed = 200.0f;
    player->boost_speed = 420.0f;
    player->body = LoadTexture("Assets/Textures/Ships/Ship/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Full health.png");
    player->size = (Vector2){ (float)player->body.width, (float)player->body.height };

    player->engine_idle_sheet = SpriteSheet_LoadAuto(
        "Assets/Textures/Ships/Ship/Main Ship/Main Ship - Engine Effects/PNGs/Main Ship - Engines - Base Engine - Idle.png");
    player->engine_boost_sheet = SpriteSheet_LoadAuto(
        "Assets/Textures/Ships/Ship/Main Ship/Main Ship - Engine Effects/PNGs/Main Ship - Engines - Base Engine - Powering.png");
    SpriteAnim_Init(&player->engine_idle_anim, &player->engine_idle_sheet, 0.12f);
    SpriteAnim_Init(&player->engine_boost_anim, &player->engine_boost_sheet, 0.08f);
}

void Player_Update(Player *player, float dt, Camera2D camera, Rectangle map_bounds)
{
    Vector2 move = {0};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1.0f;

    if (move.x != 0.0f || move.y != 0.0f)
    {
        Vector2 dir = NormalizeSafe(move);
        bool boosting = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
        float current_speed = boosting ? player->boost_speed : player->speed;
        player->position.x += dir.x * current_speed * dt;
        player->position.y += dir.y * current_speed * dt;
    }

    Vector2 mouse_screen = GetMousePosition();
    Vector2 mouse_world = GetScreenToWorld2D(mouse_screen, camera);
    Vector2 to_mouse = { mouse_world.x - player->position.x, mouse_world.y - player->position.y };
    player->angle = atan2f(to_mouse.y, to_mouse.x) * RAD2DEG + 90.0f;

    float half_w = player->size.x * 0.5f;
    float half_h = player->size.y * 0.5f;
    if (player->position.x < map_bounds.x + half_w) player->position.x = map_bounds.x + half_w;
    if (player->position.y < map_bounds.y + half_h) player->position.y = map_bounds.y + half_h;
    if (player->position.x > map_bounds.x + map_bounds.width - half_w) player->position.x = map_bounds.x + map_bounds.width - half_w;
    if (player->position.y > map_bounds.y + map_bounds.height - half_h) player->position.y = map_bounds.y + map_bounds.height - half_h;

    SpriteAnim_Update(&player->engine_idle_anim, dt);
    SpriteAnim_Update(&player->engine_boost_anim, dt);
}

void Player_Draw(Player *player, Camera2D camera)
{
    Vector2 mouse_screen = GetMousePosition();
    Vector2 mouse_world = GetScreenToWorld2D(mouse_screen, camera);
    Vector2 forward = NormalizeSafe((Vector2){ mouse_world.x - player->position.x, mouse_world.y - player->position.y });
    if (forward.x == 0.0f && forward.y == 0.0f) forward = (Vector2){0.0f, -1.0f};

    bool boosting = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    SpriteAnim *engine = boosting ? &player->engine_boost_anim : &player->engine_idle_anim;

    float engine_offset = player->size.y * 0.05f;
    Vector2 engine_pos = { player->position.x - forward.x * engine_offset, player->position.y - forward.y * engine_offset };
    Rectangle engine_dest = { engine_pos.x, engine_pos.y, engine->frame.width, engine->frame.height };
    Vector2 engine_origin = { engine_dest.width * 0.5f, engine_dest.height * 0.5f };
    DrawTexturePro(engine->sheet->texture, engine->frame, engine_dest, engine_origin, player->angle, WHITE);

    Rectangle ship_dest = { player->position.x, player->position.y, player->size.x, player->size.y };
    Vector2 ship_origin = { player->size.x * 0.5f, player->size.y * 0.5f };
    DrawTexturePro(player->body, (Rectangle){0, 0, player->size.x, player->size.y}, ship_dest, ship_origin, player->angle, WHITE);
}

void Player_Unload(Player *player)
{
    UnloadTexture(player->body);
    SpriteSheet_Unload(&player->engine_idle_sheet);
    SpriteSheet_Unload(&player->engine_boost_sheet);
}
