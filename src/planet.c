#include "planet.h"

void Planet_Init(Planet *planet, Vector2 position, float scale)
{
    *planet = (Planet){0};
    planet->position = position;
    planet->scale = scale;
    planet->sheet = SpriteSheet_Load(
        "Assets/Textures/Planets/PlanetSpriteSheet.png",
        500,
        500);
    SpriteAnim_Init(&planet->anim, &planet->sheet, 0.25f);
}

void Planet_Update(Planet *planet, float dt)
{
    SpriteAnim_Update(&planet->anim, dt);
}

void Planet_Draw(Planet *planet)
{
    Rectangle dest = {
        planet->position.x,
        planet->position.y,
        planet->anim.frame.width * planet->scale,
        planet->anim.frame.height * planet->scale
    };
    Vector2 origin = { dest.width * 0.5f, dest.height * 0.5f };
    DrawTexturePro(planet->sheet.texture, planet->anim.frame, dest, origin, 0.0f, WHITE);
}

void Planet_Unload(Planet *planet)
{
    SpriteSheet_Unload(&planet->sheet);
}
