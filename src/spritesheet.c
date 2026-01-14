#include "spritesheet.h"
#include <stddef.h>

static int ClampFrameCount(int value)
{
    if (value <= 0) return 1;
    return value;
}

SpriteSheet SpriteSheet_LoadAuto(const char *path)
{
    SpriteSheet sheet = {0};
    sheet.texture = LoadTexture(path);

    if (sheet.texture.width <= 0 || sheet.texture.height <= 0)
    {
        sheet.frame_width = 1;
        sheet.frame_height = 1;
        sheet.columns = 1;
        sheet.rows = 1;
        sheet.frame_count = 1;
        return sheet;
    }

    sheet.frame_height = sheet.texture.height;
    sheet.frame_width = sheet.frame_height;
    if (sheet.frame_width <= 0) sheet.frame_width = sheet.texture.width;

    sheet.columns = ClampFrameCount(sheet.texture.width / sheet.frame_width);
    sheet.rows = 1;
    sheet.frame_count = sheet.columns * sheet.rows;
    return sheet;
}

SpriteSheet SpriteSheet_Load(const char *path, int frame_width, int frame_height)
{
    SpriteSheet sheet = {0};
    sheet.texture = LoadTexture(path);

    if (sheet.texture.width <= 0 || sheet.texture.height <= 0)
    {
        sheet.frame_width = 1;
        sheet.frame_height = 1;
        sheet.columns = 1;
        sheet.rows = 1;
        sheet.frame_count = 1;
        return sheet;
    }

    sheet.frame_width = (frame_width > 0) ? frame_width : sheet.texture.height;
    sheet.frame_height = (frame_height > 0) ? frame_height : sheet.texture.height;

    if (sheet.frame_width <= 0) sheet.frame_width = sheet.texture.width;
    if (sheet.frame_height <= 0) sheet.frame_height = sheet.texture.height;

    sheet.columns = ClampFrameCount(sheet.texture.width / sheet.frame_width);
    sheet.rows = ClampFrameCount(sheet.texture.height / sheet.frame_height);
    sheet.frame_count = sheet.columns * sheet.rows;
    return sheet;
}

void SpriteSheet_Unload(SpriteSheet *sheet)
{
    UnloadTexture(sheet->texture);
}

void SpriteAnim_Init(SpriteAnim *anim, SpriteSheet *sheet, float frame_time)
{
    *anim = (SpriteAnim){0};
    anim->sheet = sheet;
    anim->frame_time = frame_time;
    anim->frame = (Rectangle){0.0f, 0.0f, (float)sheet->frame_width, (float)sheet->frame_height};
}

void SpriteAnim_Update(SpriteAnim *anim, float dt)
{
    if (anim->sheet == NULL || anim->sheet->frame_count <= 1) return;
    anim->timer += dt;
    if (anim->timer >= anim->frame_time)
    {
        anim->timer = 0.0f;
        anim->index = (anim->index + 1) % anim->sheet->frame_count;
        int col = anim->index % anim->sheet->columns;
        int row = anim->index / anim->sheet->columns;
        anim->frame.x = (float)col * anim->sheet->frame_width;
        anim->frame.y = (float)row * anim->sheet->frame_height;
    }
}
