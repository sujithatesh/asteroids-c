#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define W 800
#define H 450
#define MAX_ASTEROIDS 10
#define MAX_BULLETS 20

typedef struct { Vector2 pos, vel; float size; bool active; } Entity;

int main() {
    InitWindow(W, H, "Asteroids");
    SetTargetFPS(60);
    srand(time(0));

    Entity player = {{W/2, H/2}, {0, 0}, 20, true};
    Entity asteroids[MAX_ASTEROIDS];
    Entity bullets[MAX_BULLETS];
    int asteroidCount = 0, bulletCount = 0;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].size = (float)(rand() % 30 + 15);
        asteroids[i].pos = (Vector2){(float)(rand() % W), (float)(rand() % H)};
        asteroids[i].vel = (Vector2){(float)(rand() % 200 - 100), (float)(rand() % 200 - 100)};
        asteroids[i].active = true;
        asteroidCount++;
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Player movement/rotation (simplified)
        if (IsKeyDown(KEY_W)) player.vel.y -= 100 * dt;
        if (IsKeyDown(KEY_S)) player.vel.y += 100 * dt;
        if (IsKeyDown(KEY_A)) player.vel.x -= 100 * dt;
        if (IsKeyDown(KEY_D)) player.vel.x += 100 * dt;
        player.pos.x += player.vel.x * dt;
        player.pos.y += player.vel.y * dt;
        player.vel = (Vector2){0,0}; // Reset velocity each frame for simple movement

        // Keep player on screen (wrap around)
        if (player.pos.x > W) player.pos.x = 0;
        if (player.pos.x < 0) player.pos.x = W;
        if (player.pos.y > H) player.pos.y = 0;
        if (player.pos.y < 0) player.pos.y = H;

        // Shooting
        if (IsKeyPressed(KEY_SPACE) && bulletCount < MAX_BULLETS) {
            bullets[bulletCount] = (Entity){player.pos, {0, -300}, 4, true};  // Simplified bullet velocity
            bulletCount++;
        }

        // Bullet update
        for (int i = 0; i < bulletCount; i++) {
            if (bullets[i].active) {
                bullets[i].pos.y += bullets[i].vel.y * dt;
                if (bullets[i].pos.y < 0) bullets[i].active = false; // Bullet off-screen
            }
        }

        // Asteroid update
        for (int i = 0; i < asteroidCount; i++) {
            if (asteroids[i].active) {
                asteroids[i].pos.x += asteroids[i].vel.x * dt;
                asteroids[i].pos.y += asteroids[i].vel.y * dt;

                // Keep asteroids on screen (wrap around)
                if (asteroids[i].pos.x > W) asteroids[i].pos.x = 0;
                if (asteroids[i].pos.x < 0) asteroids[i].pos.x = W;
                if (asteroids[i].pos.y > H) asteroids[i].pos.y = 0;
                if (asteroids[i].pos.y < 0) asteroids[i].pos.y = H;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw player (triangle)
        DrawTriangleLines((Vector2){player.pos.x, player.pos.y - player.size},
                         (Vector2){player.pos.x - player.size, player.pos.y + player.size},
                         (Vector2){player.pos.x + player.size, player.pos.y + player.size}, WHITE);

        // Draw bullets
        for (int i = 0; i < bulletCount; i++) {
            if (bullets[i].active) DrawCircleV(bullets[i].pos, bullets[i].size, WHITE);
        }

        // Draw asteroids
        for (int i = 0; i < asteroidCount; i++) {
            if (asteroids[i].active) DrawCircleV(asteroids[i].pos, asteroids[i].size, GRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
