#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "raylib.h"

#define ASTEROID_MAX 128
#define ASTEROID_TEXTURE_MAX 64

typedef struct AsteroidAsset
{
    Texture2D texture;
    unsigned char *mask;
    int width;
    int height;
} AsteroidAsset;

typedef struct Asteroid
{
    Vector2 position;
    Vector2 velocity;
    float scale;
    int asset_index;
    float hp;
    float hp_max;
} Asteroid;

typedef struct DamagePopup
{
    Vector2 position;
    float value;
    float timer;
    float lifetime;
} DamagePopup;

typedef struct AsteroidSystem
{
    AsteroidAsset assets[ASTEROID_TEXTURE_MAX];
    int asset_count;
    Asteroid asteroids[ASTEROID_MAX];
    int asteroid_count;
    DamagePopup popups[ASTEROID_MAX];
    int popup_count;
    float spawn_timer;
    float spawn_interval;
    float min_spawn_dist;
    float max_spawn_dist;
    float speed;
} AsteroidSystem;

void Asteroids_Init(AsteroidSystem *system, const char *directory);
void Asteroids_Update(AsteroidSystem *system, float dt, Camera2D camera, Vector2 player_pos);
void Asteroids_Draw(AsteroidSystem *system);
int Asteroids_FindClosest(const AsteroidSystem *system, Vector2 position, float range, float *out_dist);
int Asteroids_ApplyDamage(AsteroidSystem *system, int index, float damage);
void Asteroids_AddPopup(AsteroidSystem *system, Vector2 position, float value);
int Asteroids_GetInfo(const AsteroidSystem *system, int index, Vector2 *out_pos, float *out_radius);
void Asteroids_Unload(AsteroidSystem *system);

#endif
