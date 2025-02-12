#include <raylib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <sys/mman.h>

#define SCREEN_WIDTH 1240
#define SCREEN_HEIGHT 780
#define TILE_SIZE 40
#define BULLET_SIZE 4

typedef enum{
    ASTEROID,
    BULLET
}entityType;

typedef struct Arena{
    void* memory;
    size_t used;
    size_t capacity;
}Arena;

Arena arena_init(size_t size){
    void* memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    return (Arena){
	.memory = memory,
	.capacity = size,
	.used = 0
    };
}

void* arena_alloc(Arena* arena, size_t size){
    if(arena->used + size > arena->capacity){
	arena->used = 0;
    }
    void* ptr =  (char*)arena->memory + arena->used;
    arena->used += size;
    return ptr;
}

void arena_destroy(Arena* arena){
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
    int ttl;
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
	    assert("Not a correct direction");
    }
}

void 
DrawPlayer(Vector2* mousePos, player* p1, float theta)
{
    // DrawLine(p1->ver1.x , p1->ver1.y, mousePos->x, mousePos->y, RED);
    DrawCircle(p1->ver2.x, p1->ver2.y, 4, GREEN); 
    DrawCircle(p1->ver3.x, p1->ver3.y, 4, BLUE); 

    p1->ver2.x = p1->ver1.x - TILE_SIZE * cosf(theta - 1.0f * PI / 6.0f);
    p1->ver2.y = p1->ver1.y - TILE_SIZE * sinf(theta - 1.0f * PI / 6.0f);

    p1->ver3.x = p1->ver1.x - TILE_SIZE * cosf(theta + 1.0f * PI / 6.0f);
    p1->ver3.y = p1->ver1.y - TILE_SIZE * sinf(theta + 1.0f * PI / 6.0f);

    DrawTriangleLines(p1->ver1, p1->ver2, p1->ver3, RAYWHITE);
}

void 
DrawBullets(entity* b, int* subBulletCounter, float dT, int i)
{
    b->pos.x += b->vec.x * b->speed * dT;
    b->pos.y += b->vec.y * b->speed * dT;

    if(b->pos.x < 0 || b->pos.y < 0 || b->pos.x > SCREEN_WIDTH || b->pos.y > SCREEN_HEIGHT ){
	*subBulletCounter = i+1 ;
    }
    DrawRectangleV(b->pos, (Vector2){.x = BULLET_SIZE, .y = BULLET_SIZE}, b->entityColor);
}

void
DrawAsteroid(entity *a, int* subAsteroidCounter, float dT, int j)
{
    a->pos.x += a->vec.x * a->speed * dT;
    a->pos.y += a->vec.y * a->speed * dT;

    if(a->pos.x < 0 || a->pos.y < 0 || a->pos.x > SCREEN_WIDTH || a->pos.y > SCREEN_HEIGHT ){
	*subAsteroidCounter = j + 1;
    }
    int n = 6;
    float delta = 2 * PI / n;
    Vector2 Vertices[n];
    for(int i = 0; i < n; i++){
	Vertices[i].x = cosf(delta * i) * 30 + a->pos.x;
	Vertices[i].y = sinf(delta * i) * 30 + a->pos.y;
    }
    DrawLineStrip(Vertices, n, a->entityColor);
    DrawLine(Vertices[n-1].x, Vertices[n-1].y, Vertices[0].x, Vertices[0].y, a->entityColor);
}

void 
drawEntityArray(entityType testEntity, entity* entityArray, float dT, int i, int *subCounter)
{
    switch(testEntity){
	case ASTEROID:
	    DrawAsteroid(&entityArray[i], subCounter, dT, i);
	    break;
	case BULLET:
	    DrawBullets(&entityArray[i], subCounter, dT, i);
	    break;
    }
}

void 
renderEntity(entityType testEntity, int *addCounter, int *subCounter, int maxEntity, entity* entityArray, float dT)
{
    if (*addCounter == *subCounter) {
        return;
    }

    if (*addCounter < *subCounter) {
        // Handle wrap-around case
        for (size_t i = *subCounter; i < maxEntity; i++) {
            if (entityArray[i].render) {
                drawEntityArray(testEntity, entityArray, dT, i, subCounter);
            }
        }
        for (size_t i = 0; i < *addCounter; i++) {
            if (entityArray[i].render) {
                drawEntityArray(testEntity, entityArray, dT, i, subCounter);
            }
        }
    } else {
        // Normal case
        for (size_t i = *subCounter; i < *addCounter; i++) {
            if (entityArray[i].render) {
                drawEntityArray(testEntity, entityArray, dT, i, subCounter);
            }
        }
    }

    // Reset subCounter if it exceeds maxEntity
    if (*subCounter >= maxEntity) {
        *subCounter = 0;
    }
}

int main(void){
    float speed = 350;
    int maxBullets = 40;
    int maxAsteroids = 50;
    int SM = 100;
    float asteroidFallRate = 10;
    int even = 0, odd = 0;
    float bulletArrayDist = 0;

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
    int subBulletCounter = 0;
    int subAsteroidCounter = 0;
    int switchBullet = 0;
    float dT = 0;
    float bulletTicker = 0;
    float asteroidTicker = 0;


    entity* bulletArray;
    Arena bulletArena = arena_init(maxBullets * sizeof(entity));

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

	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
	    if(bulletTicker > newbulletType[switchBullet].rate){
		if(addBulletCounter >= maxBullets){
		    addBulletCounter = 0;
		}
		bulletTicker = 0;
		entity* newbullet = arena_alloc(&bulletArena, sizeof(entity));
		newbullet->pos = (Vector2){.x = p1.ver1.x - BULLET_SIZE / 2, .y = p1.ver1.y - BULLET_SIZE / 2};
		newbullet->vec = (Vector2){.x = cosf(theta), .y = sinf(theta)};
		newbullet->speed = 700;
		newbullet->entityColor = newbulletType[switchBullet].bulletColor;
		newbullet->render = 1;
		addBulletCounter++;
		bulletArray = bulletArena.memory;
	    }
	}

	if(asteroidTicker > 1 / asteroidFallRate ){
	    if(addAsteroidCounter >= maxAsteroids){
		addAsteroidCounter = 0;
	    }
	    asteroidTicker = 0;
	    entity* newAsteroid = arena_alloc(&asteroidArena, sizeof(entity));
	    if(addAsteroidCounter % 2 != 0){
		newAsteroid->pos = (Vector2){.x = 0, .y = 0};
		newAsteroid->vec = (Vector2){.x = 1 / sqrt(2), .y = 1 / sqrt(2)};
		newAsteroid->entityColor = RED;
		even++;
	    }
	    else{
		newAsteroid->pos = (Vector2){.x = SCREEN_WIDTH, .y = SCREEN_HEIGHT};
		newAsteroid->vec = (Vector2){.x = - 1 / sqrt(2), .y = - 1 / sqrt(2)};
		newAsteroid->entityColor = BLUE;
		odd++;
	    }
	    newAsteroid->speed = 7 * SM;
	    newAsteroid->render = 1;
	    addAsteroidCounter++;
	    asteroidArray = asteroidArena.memory;
	}

	if(addAsteroidCounter >= 0){
	    renderEntity(ASTEROID, &addAsteroidCounter, &subAsteroidCounter, maxAsteroids, asteroidArray, dT);
	}

	if(addBulletCounter >= 0){
	    renderEntity(BULLET, &addBulletCounter, &subBulletCounter, maxBullets, bulletArray, dT);
	}

	if(addBulletCounter > 2){
	    bulletArrayDist = ((bulletArray[0].pos.x - bulletArray[1].pos.x));
	    if(bulletArrayDist > 200){
	    }
	}

	DrawPlayer(&mousePos, &p1, theta);
	DrawFPS(20, 20);

	EndDrawing();
    }
    arena_destroy(&bulletArena);
    return 0;
}
