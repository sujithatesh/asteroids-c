#include <raylib.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MEGABYTE 1024 * 1024

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
    void* ptr =  arena->memory + arena->used;
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
} entity;

entity* shootBullet(Arena* arena,player* p, float theta){
    entity* bullet = arena_alloc(arena, sizeof(entity));
    bullet->pos = (Vector2){.x = p->ver1.x, .y = p->ver1.y };
    bullet->vec = (Vector2){.x = sin(PI/2 - theta), .y = cos(PI/2 - theta)};
    bullet->speed = 0.1;
    return bullet;
}

void movDir(player* p, Direction d, float scale){
    switch (d) {
	case W:
	    p->ver1.y -= scale;
	    p->ver2.y -= scale;
	    p->ver3.y -= scale;
	    break;
	case A:
	    p->ver1.x -= scale;
	    p->ver2.x -= scale;
	    p->ver3.x -= scale;
	    break;
	case S:
	    p->ver1.y += scale;
	    p->ver2.y += scale;
	    p->ver3.y += scale;
	    break;
	case D:
	    p->ver1.x += scale;
	    p->ver2.x += scale;
	    p->ver3.x += scale;
	    break;
	default:
	    assert("Not a correct direction");
    }
}

void DrawPlayer(Vector2* mousePos, player* p1, float theta){
    DrawLine(p1->ver1.x , p1->ver1.y, mousePos->x, mousePos->y, RED);
    DrawCircle(p1->ver2.x, p1->ver2.y, 4, GREEN); 
    DrawCircle(p1->ver3.x, p1->ver3.y, 4, BLUE); 

    p1->ver2.x = p1->ver1.x - TILE_SIZE * cosf(theta - 1.0f * PI / 6.0f);
    p1->ver2.y = p1->ver1.y - TILE_SIZE * sinf(theta - 1.0f * PI / 6.0f);

    p1->ver3.x = p1->ver1.x - TILE_SIZE * cosf(theta + 1.0f * PI / 6.0f);
    p1->ver3.y = p1->ver1.y - TILE_SIZE * sinf(theta + 1.0f * PI / 6.0f);

    DrawTriangleLines(p1->ver1, p1->ver2, p1->ver3, RAYWHITE);
}

int main(void){
    float speed = 0.1;
    player p1 = {.ver1 = {.x = 300, .y = 300},
	.ver2 = {.x = 300 - 0.5 * TILE_SIZE, .y = 300 + 0.86602540378 * TILE_SIZE},
	.ver3 = {.x = 300 + 0.5 * TILE_SIZE, .y = 300 + 0.86602540378 * TILE_SIZE}
    };
    float bulletTicker = 0;
    int bulletCounter = 0;
    entity* bulletArray;
    Arena bullet_arena = arena_init(MEGABYTE * sizeof(entity));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Init Window");
    while(!WindowShouldClose()){
	BeginDrawing(); 
	ClearBackground(BLACK);
	bulletTicker += GetFrameTime();
	Vector2 mousePos = GetMousePosition();
	float theta = atan2f((mousePos.y - p1.ver1.y), (mousePos.x - p1.ver1.x));

	if(IsKeyDown(KEY_W)) {
	    movDir(&p1, W, speed);
	}
	if(IsKeyDown(KEY_A)) {
	    movDir(&p1, A, speed);
	}
	if(IsKeyDown(KEY_S)) {
	    movDir(&p1, S, speed);
	}
	if(IsKeyDown(KEY_D)) {
	    movDir(&p1, D, speed);
	}
	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
	    while(bulletTicker > 0.1){
		if(bulletCounter >= MEGABYTE / sizeof(entity)){
		    bulletCounter = 0;
		    printf("exceeded\n");
		}
		bulletTicker = 0;
		shootBullet(&bullet_arena, &p1, theta);
		bulletCounter++;
	    }
	    bulletArray = bullet_arena.memory;
	    printf("bullet: %d\n", bulletCounter);
	}

	if(bulletCounter > 0){
	    for(size_t i = 0; i < bulletCounter; i++){
		bulletArray[i].pos.x += bulletArray[i].vec.x * bulletArray[i].speed;
		bulletArray[i].pos.y += bulletArray[i].vec.y * bulletArray[i].speed;
		DrawRectangleV(bulletArray[i].pos, (Vector2){.x = 5, .y = 5}, WHITE);
	    }
	}

	DrawPlayer(&mousePos, &p1, theta);
	EndDrawing();
    }
    arena_destroy(&bullet_arena);
    return 0;
}
