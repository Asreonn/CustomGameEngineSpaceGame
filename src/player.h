#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "spritesheet.h"

typedef struct Player
{
    Vector2 position;
    Vector2 size;
    float speed;
    float boost_speed;
    float angle;
    Texture2D body;
    SpriteSheet engine_idle_sheet;
    SpriteSheet engine_boost_sheet;
    SpriteAnim engine_idle_anim;
    SpriteAnim engine_boost_anim;
} Player;

void Player_Init(Player *player, Vector2 start_pos);
void Player_Update(Player *player, float dt, Camera2D camera, Rectangle map_bounds);
void Player_Draw(Player *player, Camera2D camera);
void Player_Unload(Player *player);

#endif
