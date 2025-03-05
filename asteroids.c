#define DEBUG 

#include <raylib.h>

#include <math.h>
#include <stdio.h>
#include <rand.h>
#include <sys/mman.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define CENTER_X SCREEN_WIDTH / 2
#define CENTER_Y SCREEN_HEIGHT / 2

#define TILE_SIZE 40
#define BULLET_SIZE 4


float sixtyDelta = 2 * PI / 6;

float sinAngles[6] = {};
float cosAngles[6] = {};

typedef enum{
    ASTEROID,
    BULLET
}entityType;

typedef struct Arena{
    void* memory;
    size_t used;
    size_t capacity;
}Arena;

typedef struct FreeList{
    void* memory;
    struct FreeList *next;
} FreeList;

Arena 
arena_init(size_t size)
{
    void* memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    return (Arena){
        .memory = memory,
        .capacity = size,
        .used = 0
    };
}

void*
arena_alloc(Arena* arena, size_t size)
{
    if(arena->used + size > arena->capacity){
        arena->used = 0;
    }
    void* ptr =  (char*)arena->memory + arena->used;
    arena->used += size;
    return ptr;
}

void 
arena_destroy(Arena* arena)
{
    munmap(arena->memory, arena->capacity);
    arena->memory = NULL;
    arena->used = 0;
    arena->capacity = 0;
}

typedef enum Direction{ W, A, S, D} Direction;


typedef struct player{
    Vector2 ver1;
    Vector2 ver2;
    Vector2 ver3;
} player;

typedef struct entity{
    Vector2 pos;
    Vector2 vec;
    float speed;
    bool render;
    Color entityColor;
} entity;

typedef struct bulletType{
    int ttl;
    float rate;
    Color bulletColor;
} bulletType;

typedef struct asteroidType{
    Color asteroidColor;
} asteroidType;

void 
movDir(player* p, Direction d, float scale, float dT)
{
    switch (d) {
        case W:
        p->ver1.y -= scale * dT;
        p->ver2.y -= scale * dT;
        p->ver3.y -= scale * dT;
        break;
        case A:
        p->ver1.x -= scale * dT;
        p->ver2.x -= scale * dT;
        p->ver3.x -= scale * dT;
        break;
        case S:
        p->ver1.y += scale * dT;
        p->ver2.y += scale * dT;
        p->ver3.y += scale * dT;
        break;
        case D:
        p->ver1.x += scale * dT;
        p->ver2.x += scale * dT;
        p->ver3.x += scale * dT;
        break;
        default:
        break;
    }
}

void 
DrawPlayer(Vector2* mousePos, player* p1, float theta)
{
    DrawCircle(p1->ver2.x, p1->ver2.y, 4, GREEN); 
    DrawCircle(p1->ver3.x, p1->ver3.y, 4, BLUE); 
    
    p1->ver2.x = p1->ver1.x - TILE_SIZE * cosf(theta - 1.0f * PI / 6.0f);
    p1->ver2.y = p1->ver1.y - TILE_SIZE * sinf(theta - 1.0f * PI / 6.0f);
    
    p1->ver3.x = p1->ver1.x - TILE_SIZE * cosf(theta + 1.0f * PI / 6.0f);
    p1->ver3.y = p1->ver1.y - TILE_SIZE * sinf(theta + 1.0f * PI / 6.0f);
    
    DrawTriangleLines(p1->ver1, p1->ver2, p1->ver3, RAYWHITE);
}

void 
DrawBullets(entity* b, float dT, int i)
{
    DrawPixel(b->pos.x, b->pos.y, GREEN);
}

void
DrawAsteroid(entity *a, float dT, int i)
{
    int n = 6;
    Vector2 Vertices[n];
    for(int i = 0; i < n; i++){
        Vertices[i].x = sinAngles[i] + a->pos.x;
        Vertices[i].y = cosAngles[i] + a->pos.y;
    }
    DrawPixel(a->pos.x, a->pos.y, BLUE);
    
    DrawLineStrip(Vertices, n, a->entityColor);
    DrawLine(Vertices[n-1].x, Vertices[n-1].y, Vertices[0].x, Vertices[0].y, a->entityColor);
}

void 
drawEntityArray(entityType testEntity, entity* entityArray, float dT, int i)
{
    entityArray[i].pos.x += entityArray[i].vec.x * entityArray[i].speed * dT;
    entityArray[i].pos.y += entityArray[i].vec.y * entityArray[i].speed * dT;
    
    if(entityArray[i].pos.x < 0 || entityArray[i].pos.y < 0 || entityArray[i].pos.x >= SCREEN_WIDTH || entityArray[i].pos.y >= SCREEN_HEIGHT ){
        entityArray[i].render = 0;
    }
    
    switch(testEntity){
        case ASTEROID:
        DrawAsteroid(&entityArray[i], dT, i);
        break;
        case BULLET:
        DrawBullets(&entityArray[i], dT, i);
        break;
    }
}

void 
renderEntity(entityType testEntity, int *addCounter, int maxEntity, entity* entityArray, float dT)
{
    if (*addCounter == 0) {
        return;
    }
    if (*addCounter < maxEntity && addCounter > 0) {
        for (size_t i = 0; i <= maxEntity; i++) {
            if (entityArray[i].render) {
                drawEntityArray(testEntity, entityArray, dT, i);
            }
        }
    }
    
    return;
}

bool 
isColliding(entity* asteroid, entity* bullet)
{
    if((asteroid->pos.x <= bullet->pos.x + 50) && (asteroid->pos.x >= bullet->pos.x - 50)
       && (asteroid->pos.y <= bullet->pos.y + 50) && (asteroid->pos.y >= bullet->pos.y - 50)){
        return true;
    }
    
    return false;
}

void 
initAngles()
{
    for(int i = 0; i < 6; i++){
        sinAngles[i] = sinf(sixtyDelta * i) * 30;
        cosAngles[i] = cosf(sixtyDelta * i) * 30;
    };
}

void 
checkCollisionAsteroidBullet(Arena* freeListArena, entity* bulletArray, entity* asteroidArray, int asteroidCount, int bulletCount)
{
    for(int i = 0; i < asteroidCount; i++){
        for(int j = 0; j < bulletCount; j++){
            if(bulletArray[j].render == true && asteroidArray[i].render == true && isColliding(&asteroidArray[i], &bulletArray[j])){
                asteroidArray[i].render = 0;
                bulletArray[j].render = 1;
                printf("%d\n", j);
            }
        }
    }
}


void 
checkCollisionAsteroidPlayer(player* player, entity* asteroidArray, int asteroidCount){
    for(int i = 0; i < asteroidCount; i++){
        if(asteroidArray[i].render && CheckCollisionPointTriangle(asteroidArray[i].pos, player->ver1, player->ver2, player->ver3)){
            asteroidArray[i].render = 0;
        }
    }
}

int
main(void)
{
    float speed = 350;
    int maxBullets = 6000;
    int maxAsteroids = 600;
    int SM = 100;
    int randCounter = 0;
    float asteroidRate = 100;
    int even = 0;
    int odd = 0;
    
    
    initAngles();
    
    player p1 = {.ver1 = {.x = 300, .y = 300},
        .ver2 = {.x = 300 - 0.5 * TILE_SIZE, .y = 300 + 0.86602540378 * TILE_SIZE},
        .ver3 = {.x = 300 + 0.5 * TILE_SIZE, .y = 300 + 0.86602540378 * TILE_SIZE}
    };
    
    bulletType newbulletType[] = { 
        { .ttl = 2, .rate = 1 * 0.1, .bulletColor = PINK},
        { .ttl = 3, .rate = 3 * 0.1, .bulletColor = GREEN},
        { .ttl = 4, .rate = 4 * 0.1, .bulletColor = BLUE}
    };
    
    int addBulletCounter = 0;
    int addAsteroidCounter = 0;
    int switchBullet = 0;
    float dT = 0;
    float bulletTicker = 0;
    float asteroidTicker = 0;
    
    entity* bulletArray;
    Arena bulletArena = arena_init(maxBullets * sizeof(entity));
    Arena freeListArena = arena_init(maxBullets * sizeof(FreeList));
    
    entity* asteroidArray;
    Arena asteroidArena = arena_init(maxAsteroids * sizeof(entity));
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Init Window");
    while(!WindowShouldClose()){
        BeginDrawing(); 
        ClearBackground(BLACK);
        
        dT = GetFrameTime();
        bulletTicker += dT;
        asteroidTicker += dT;
        Vector2 mousePos = GetMousePosition();
        float theta = atan2f((mousePos.y - p1.ver1.y), (mousePos.x - p1.ver1.x));
        
        if(IsKeyDown(KEY_W)) {
            movDir(&p1, W, speed, dT);
        }
        if(IsKeyDown(KEY_A)) {
            movDir(&p1, A, speed, dT);
        }
        if(IsKeyDown(KEY_S)) {
            movDir(&p1, S, speed, dT);
        }
        if(IsKeyDown(KEY_D)) {
            movDir(&p1, D, speed, dT);
        }
        
        if(IsKeyPressed(KEY_ONE)) {
            switchBullet = 0;
        }
        else if(IsKeyPressed(KEY_TWO)) {
            switchBullet = 1;
        }
        else if(IsKeyPressed(KEY_THREE)) {
            switchBullet = 2;
        }
        
        if(p1.ver1.x > SCREEN_WIDTH) p1.ver1.x = 0;
        if(p1.ver1.y > SCREEN_HEIGHT) p1.ver1.y = 20;
        if(p1.ver1.x < 0) p1.ver1.x = SCREEN_WIDTH;
        if(p1.ver1.y < 0) p1.ver1.y = SCREEN_HEIGHT;
        
        
        
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            if(bulletTicker > newbulletType[switchBullet].rate){
                if(addBulletCounter >= maxBullets){
                    addBulletCounter = 0;
                }
                bulletTicker = 0;
                entity* newBullet = arena_alloc(&bulletArena, sizeof(entity));
                newBullet->pos = (Vector2){.x = p1.ver1.x - (float)BULLET_SIZE / 2, .y = p1.ver1.y - (float)BULLET_SIZE / 2};
                newBullet->vec = (Vector2){.x = cosf(theta), .y = sinf(theta)};
                newBullet->speed = 700;
                newBullet->entityColor =  newbulletType[switchBullet].bulletColor;
                newBullet->render = 1;
                addBulletCounter++;
                bulletArray = bulletArena.memory;
            }
        }
        
        if(asteroidTicker > 1 / asteroidRate){
            if(addAsteroidCounter >= maxAsteroids - 1){
                addAsteroidCounter = 0;
            }
            asteroidTicker = 0;
            entity* newAsteroid = arena_alloc(&asteroidArena, sizeof(entity));
            float vx = (float) (randArray[randCounter % 30] % 750) / 1000;
            randCounter++;;
            float vy = (float) (randArray[randCounter % 30] % 500) / 1000;
            
            if(addAsteroidCounter % 4 == 0){
                newAsteroid->pos = (Vector2){.x = 0, .y = 0};
                newAsteroid->vec = (Vector2){.x =  0.70710678118, .y = 0.70710678118};
                //newAsteroid->vec = (Vector2){.x = + vx, .y = + vy};
                newAsteroid->entityColor = RED;
                even++;
            }
            /*else if(addAsteroidCounter % 4 == 1){
                newAsteroid->pos = (Vector2){.x = SCREEN_WIDTH, .y = SCREEN_HEIGHT};
                newAsteroid->vec = (Vector2){.x = - vx, .y = - vy};
                newAsteroid->entityColor = PINK;
                odd++;
            }
            else if(addAsteroidCounter % 4 == 2){
                newAsteroid->pos = (Vector2){.x = SCREEN_WIDTH, .y = 0};
                newAsteroid->vec = (Vector2){.x = - vx, .y = + vy};
                newAsteroid->entityColor = GREEN;
            }
            else{
                newAsteroid->pos = (Vector2){.x = 0, .y = SCREEN_HEIGHT};
                newAsteroid->vec = (Vector2){.x =  vx, .y = - vy};
                newAsteroid->entityColor = YELLOW;
            }*/
            
            newAsteroid->speed = 7 * SM;
            newAsteroid->render = 1;
            addAsteroidCounter++;
            asteroidArray = asteroidArena.memory;
            randCounter++;
            if(randCounter >= 30) randCounter = 0;
        }
        
        checkCollisionAsteroidBullet(&freeListArena, bulletArray, asteroidArray, addAsteroidCounter, addBulletCounter);
        checkCollisionAsteroidPlayer(&p1, asteroidArray, addAsteroidCounter);
        
        if(addAsteroidCounter >= 0){
            renderEntity(ASTEROID, &addAsteroidCounter, maxAsteroids, asteroidArray, dT);
        }
        
        if(addBulletCounter >= 0){
            renderEntity(BULLET, &addBulletCounter, maxBullets, bulletArray, dT);
        }
        
        DrawPlayer(&mousePos, &p1, theta);
        DrawFPS(20, 20);
        
#ifdef DEBUG
        float tempx = addAsteroidCounter;
        float scale = 1;
        DrawRectangleLines((CENTER_X * 0.5), CENTER_Y * 1.5, maxAsteroids * scale, 50, RED);
        DrawLine(CENTER_X * 0.5 + tempx * scale, CENTER_Y * 1.5, CENTER_X * 0.5 + tempx * scale, CENTER_Y * 1.5 + 50, YELLOW);
        
#endif
        EndDrawing();
    }
    arena_destroy(&bulletArena);
    return 0;
}
