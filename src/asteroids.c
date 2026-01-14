#include "asteroids.h"

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static float RandomFloat(float min, float max)
{
    float t = (float)GetRandomValue(0, 10000) / 10000.0f;
    return min + (max - min) * t;
}

static int HasPngExtension(const char *name)
{
    size_t len = strlen(name);
    if (len < 4) return 0;
    return (name[len - 4] == '.' && name[len - 3] == 'p' && name[len - 2] == 'n' && name[len - 1] == 'g');
}

static void LoadAsteroidTextures(AsteroidSystem *system, const char *directory)
{
    DIR *dir = opendir(directory);
    if (dir == NULL) return;

    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL)
    {
        if (system->asset_count >= ASTEROID_TEXTURE_MAX) break;
        if (entry->d_name[0] == '.') continue;
        if (!HasPngExtension(entry->d_name)) continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        Image image = LoadImage(path);
        if (image.data == NULL) continue;
        ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

        int pixel_count = image.width * image.height;
        unsigned char *mask = (unsigned char *)malloc((size_t)pixel_count);
        if (mask == NULL)
        {
            UnloadImage(image);
            continue;
        }

        Color *pixels = (Color *)image.data;
        for (int i = 0; i < pixel_count; i++)
        {
            mask[i] = (pixels[i].a >= 20) ? 1 : 0;
        }

        Texture2D tex = LoadTexture(path);
        if (tex.id == 0)
        {
            free(mask);
            UnloadImage(image);
            continue;
        }

        AsteroidAsset *asset = &system->assets[system->asset_count];
        asset->texture = tex;
        asset->mask = mask;
        asset->width = image.width;
        asset->height = image.height;
        system->asset_count++;

        UnloadImage(image);
    }

    closedir(dir);
}

static void SpawnAsteroid(AsteroidSystem *system, Camera2D camera, Vector2 player_pos)
{
    if (system->asset_count <= 0) return;
    if (system->asteroid_count >= ASTEROID_MAX) return;

    float screen_w = (float)GetScreenWidth();
    float screen_h = (float)GetScreenHeight();
    float half_w = screen_w * 0.5f;
    float half_h = screen_h * 0.5f;
    float view_radius = (half_w > half_h) ? half_w : half_h;

    float min_dist = (system->min_spawn_dist > 0.0f) ? system->min_spawn_dist : (view_radius + 320.0f);
    float max_dist = (system->max_spawn_dist > 0.0f) ? system->max_spawn_dist : (min_dist + 800.0f);

    float angle = RandomFloat(0.0f, 2.0f * PI);
    float dist = RandomFloat(min_dist, max_dist);
    Vector2 spawn_pos = { player_pos.x + cosf(angle) * dist, player_pos.y + sinf(angle) * dist };

    float drift_angle = RandomFloat(0.0f, 2.0f * PI);
    Vector2 dir = { cosf(drift_angle), sinf(drift_angle) };

    int tex_index = GetRandomValue(0, system->asset_count - 1);
    Asteroid *asteroid = &system->asteroids[system->asteroid_count++];
    asteroid->asset_index = tex_index;
    asteroid->position = spawn_pos;
    float speed = RandomFloat(system->speed * 0.5f, system->speed * 1.1f);
    asteroid->velocity = (Vector2){ dir.x * speed, dir.y * speed };
    asteroid->scale = RandomFloat(0.6f, 1.1f);
    asteroid->hp_max = RandomFloat(60.0f, 120.0f);
    asteroid->hp = asteroid->hp_max;
}

void Asteroids_Init(AsteroidSystem *system, const char *directory)
{
    *system = (AsteroidSystem){0};
    system->spawn_interval = 1.2f;
    system->speed = 140.0f;
    LoadAsteroidTextures(system, directory);
}

static void RemoveAsteroid(AsteroidSystem *system, int index)
{
    if (index < 0 || index >= system->asteroid_count) return;
    system->asteroids[index] = system->asteroids[system->asteroid_count - 1];
    system->asteroid_count--;
}

static void RemovePopup(AsteroidSystem *system, int index)
{
    if (index < 0 || index >= system->popup_count) return;
    system->popups[index] = system->popups[system->popup_count - 1];
    system->popup_count--;
}

static int MaskSolid(const AsteroidAsset *asset, int x, int y)
{
    if (x < 0 || y < 0 || x >= asset->width || y >= asset->height) return 0;
    return asset->mask[y * asset->width + x] != 0;
}

static int AsteroidsOverlap(const AsteroidSystem *system, const Asteroid *a, const Asteroid *b)
{
    const AsteroidAsset *asset_a = &system->assets[a->asset_index];
    const AsteroidAsset *asset_b = &system->assets[b->asset_index];

    float aw = asset_a->width * a->scale;
    float ah = asset_a->height * a->scale;
    float bw = asset_b->width * b->scale;
    float bh = asset_b->height * b->scale;

    float dx = a->position.x - b->position.x;
    float dy = a->position.y - b->position.y;
    float ra = 0.5f * ((aw > ah) ? aw : ah);
    float rb = 0.5f * ((bw > bh) ? bw : bh);
    float max_r = ra + rb;
    if (dx * dx + dy * dy > max_r * max_r) return 0;

    float a_left = a->position.x - aw * 0.5f;
    float a_top = a->position.y - ah * 0.5f;
    float b_left = b->position.x - bw * 0.5f;
    float b_top = b->position.y - bh * 0.5f;

    float left = (a_left > b_left) ? a_left : b_left;
    float top = (a_top > b_top) ? a_top : b_top;
    float right = ((a_left + aw) < (b_left + bw)) ? (a_left + aw) : (b_left + bw);
    float bottom = ((a_top + ah) < (b_top + bh)) ? (a_top + ah) : (b_top + bh);

    if (right <= left || bottom <= top) return 0;

    int start_ax = (int)floorf((left - a_left) / a->scale);
    int end_ax = (int)ceilf((right - a_left) / a->scale);
    int start_ay = (int)floorf((top - a_top) / a->scale);
    int end_ay = (int)ceilf((bottom - a_top) / a->scale);

    if (start_ax < 0) start_ax = 0;
    if (start_ay < 0) start_ay = 0;
    if (end_ax > asset_a->width) end_ax = asset_a->width;
    if (end_ay > asset_a->height) end_ay = asset_a->height;

    float inv_scale_b = 1.0f / b->scale;

    for (int y = start_ay; y < end_ay; y++)
    {
        float world_y = a_top + (y + 0.5f) * a->scale;
        int by = (int)floorf((world_y - b_top) * inv_scale_b);
        if (by < 0 || by >= asset_b->height) continue;

        float world_x = a_left + (start_ax + 0.5f) * a->scale;
        for (int x = start_ax; x < end_ax; x++, world_x += a->scale)
        {
            if (!MaskSolid(asset_a, x, y)) continue;

            int bx = (int)floorf((world_x - b_left) * inv_scale_b);
            if (bx < 0 || bx >= asset_b->width) continue;

            if (MaskSolid(asset_b, bx, by)) return 1;
        }
    }

    return 0;
}

int Asteroids_FindClosest(const AsteroidSystem *system, Vector2 position, float range, float *out_dist)
{
    float range_sq = range * range;
    int best_index = -1;
    float best_dist_sq = range_sq;

    for (int i = 0; i < system->asteroid_count; i++)
    {
        const Asteroid *asteroid = &system->asteroids[i];
        float dx = asteroid->position.x - position.x;
        float dy = asteroid->position.y - position.y;
        float dist_sq = dx * dx + dy * dy;
        if (dist_sq <= best_dist_sq)
        {
            best_dist_sq = dist_sq;
            best_index = i;
        }
    }

    if (out_dist != NULL) *out_dist = (best_index >= 0) ? sqrtf(best_dist_sq) : 0.0f;
    return best_index;
}

int Asteroids_ApplyDamage(AsteroidSystem *system, int index, float damage)
{
    if (index < 0 || index >= system->asteroid_count) return 0;
    Asteroid *asteroid = &system->asteroids[index];
    asteroid->hp -= damage;
    if (asteroid->hp <= 0.0f)
    {
        RemoveAsteroid(system, index);
        return 1;
    }
    return 0;
}

void Asteroids_AddPopup(AsteroidSystem *system, Vector2 position, float value)
{
    if (system->popup_count >= ASTEROID_MAX) return;
    DamagePopup *popup = &system->popups[system->popup_count++];
    popup->position = position;
    popup->position.x += (float)GetRandomValue(-8, 8);
    popup->position.y += (float)GetRandomValue(-8, 8);
    popup->value = value;
    popup->timer = 0.0f;
    popup->lifetime = 0.6f;
}

int Asteroids_GetInfo(const AsteroidSystem *system, int index, Vector2 *out_pos, float *out_radius)
{
    if (index < 0 || index >= system->asteroid_count) return 0;
    const Asteroid *asteroid = &system->asteroids[index];
    const AsteroidAsset *asset = &system->assets[asteroid->asset_index];
    if (out_pos != NULL) *out_pos = asteroid->position;
    if (out_radius != NULL)
    {
        float w = asset->width * asteroid->scale;
        float h = asset->height * asteroid->scale;
        float r = 0.5f * ((w > h) ? w : h);
        *out_radius = r;
    }
    return 1;
}

void Asteroids_Update(AsteroidSystem *system, float dt, Camera2D camera, Vector2 player_pos)
{
    system->spawn_timer -= dt;
    if (system->spawn_timer <= 0.0f)
    {
        SpawnAsteroid(system, camera, player_pos);
        system->spawn_timer = system->spawn_interval;
    }

    float max_dist = (system->max_spawn_dist > 0.0f) ? system->max_spawn_dist : 1200.0f;
    float despawn_dist = max_dist + 600.0f;
    float despawn_dist_sq = despawn_dist * despawn_dist;

    for (int i = 0; i < system->asteroid_count; )
    {
        Asteroid *asteroid = &system->asteroids[i];
        asteroid->position.x += asteroid->velocity.x * dt;
        asteroid->position.y += asteroid->velocity.y * dt;

        Vector2 delta = { asteroid->position.x - player_pos.x, asteroid->position.y - player_pos.y };
        float dist_sq = delta.x * delta.x + delta.y * delta.y;
        if (dist_sq > despawn_dist_sq)
        {
            RemoveAsteroid(system, i);
            continue;
        }
        i++;
    }

    for (int i = 0; i < system->asteroid_count; i++)
    {
        for (int j = i + 1; j < system->asteroid_count; )
        {
            if (AsteroidsOverlap(system, &system->asteroids[i], &system->asteroids[j]))
            {
                RemoveAsteroid(system, j);
                RemoveAsteroid(system, i);
                i--;
                break;
            }
            j++;
        }
    }

    for (int i = 0; i < system->popup_count; )
    {
        DamagePopup *popup = &system->popups[i];
        popup->timer += dt;
        popup->position.y -= 12.0f * dt;
        if (popup->timer >= popup->lifetime)
        {
            RemovePopup(system, i);
            continue;
        }
        i++;
    }
}

void Asteroids_Draw(AsteroidSystem *system)
{
    for (int i = 0; i < system->asteroid_count; i++)
    {
        Asteroid *asteroid = &system->asteroids[i];
        AsteroidAsset *asset = &system->assets[asteroid->asset_index];
        Texture2D *tex = &asset->texture;
        if (tex->id == 0) continue;

        float w = tex->width * asteroid->scale;
        float h = tex->height * asteroid->scale;
        Rectangle dest = { asteroid->position.x, asteroid->position.y, w, h };
        Vector2 origin = { w * 0.5f, h * 0.5f };
        DrawTexturePro(*tex, (Rectangle){0, 0, (float)tex->width, (float)tex->height}, dest, origin, 0.0f, WHITE);
    }

    for (int i = 0; i < system->popup_count; i++)
    {
        DamagePopup *popup = &system->popups[i];
        float t = popup->timer / popup->lifetime;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        unsigned char alpha = (unsigned char)(255 * (1.0f - t));

        char text[16];
        snprintf(text, sizeof(text), "+%d", (int)popup->value);
        DrawText(text, (int)popup->position.x, (int)popup->position.y, 14, (Color){255, 210, 120, alpha});
    }
}

void Asteroids_Unload(AsteroidSystem *system)
{
    for (int i = 0; i < system->asset_count; i++)
    {
        UnloadTexture(system->assets[i].texture);
        free(system->assets[i].mask);
        system->assets[i].mask = NULL;
    }
    system->asset_count = 0;
}
