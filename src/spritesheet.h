#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include "raylib.h"

typedef struct SpriteSheet
{
    Texture2D texture;
    int frame_width;
    int frame_height;
    int columns;
    int rows;
    int frame_count;
} SpriteSheet;

typedef struct SpriteAnim
{
    SpriteSheet *sheet;
    Rectangle frame;
    float timer;
    float frame_time;
    int index;
} SpriteAnim;

SpriteSheet SpriteSheet_LoadAuto(const char *path);
SpriteSheet SpriteSheet_Load(const char *path, int frame_width, int frame_height);
void SpriteSheet_Unload(SpriteSheet *sheet);

void SpriteAnim_Init(SpriteAnim *anim, SpriteSheet *sheet, float frame_time);
void SpriteAnim_Update(SpriteAnim *anim, float dt);

#endif
