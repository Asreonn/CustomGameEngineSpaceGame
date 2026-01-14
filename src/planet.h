#ifndef PLANET_H
#define PLANET_H

#include "raylib.h"
#include "spritesheet.h"

typedef struct Planet
{
    Vector2 position;
    float scale;
    SpriteSheet sheet;
    SpriteAnim anim;
} Planet;

void Planet_Init(Planet *planet, Vector2 position, float scale);
void Planet_Update(Planet *planet, float dt);
void Planet_Draw(Planet *planet);
void Planet_Unload(Planet *planet);

#endif
