#if 0
		    if(bulletArray[i].pos.y > SCREEN_HEIGHT){
			bulletArray[i].pos.y = 0;
		    }
		    if(bulletArray[i].pos.x > SCREEN_WIDTH){
			bulletArray[i].pos.x = 0;
		    }
		    if(bulletArray[i].pos.x < 0){
			bulletArray[i].pos.x = SCREEN_WIDTH;
		    }
		    if(bulletArray[i].pos.y < 0){
			bulletArray[i].pos.y = SCREEN_HEIGHT;
		    }
#endif
#include <raylib.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MAX_BULLETS 10
#define BULLET_SIZE 4


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

typedef struct bullet{
    Vector2 pos;
    Vector2 vec;
    float speed;
    float ttl;
} bullet;

void movDir(player* p, Direction d, float scale, float dT){
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
    float speed = 350;
    player p1 = {.ver1 = {.x = 300, .y = 300},
	.ver2 = {.x = 300 - 0.5 * TILE_SIZE, .y = 300 + 0.86602540378 * TILE_SIZE},
	.ver3 = {.x = 300 + 0.5 * TILE_SIZE, .y = 300 + 0.86602540378 * TILE_SIZE}
    };
    float bulletTicker = 0;
    int addCounter = 0;
    int subCounter = 0;
    bullet* bulletArray;
    Arena bulletArena = arena_init(MAX_BULLETS * sizeof(bullet));
    float dT = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Init Window");
    while(!WindowShouldClose()){
	BeginDrawing(); 
	ClearBackground(BLACK);
	dT = GetFrameTime();
	bulletTicker += dT;
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
	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
	    if(bulletTicker > 0.1){
		if(addCounter >= MAX_BULLETS){
		    addCounter = 0;
		}
		bulletTicker = 0;
		bullet* newbullet = arena_alloc(&bulletArena, sizeof(bullet));
		newbullet->pos = (Vector2){.x = p1.ver1.x - BULLET_SIZE / 2, .y = p1.ver1.y - BULLET_SIZE / 2};
		newbullet->vec = (Vector2){.x = cosf(theta), .y = sinf(theta)};
		newbullet->speed = 700;
		newbullet->ttl = 1;
		addCounter++;
		bulletArray = bulletArena.memory;
		if(addCounter > MAX_BULLETS) addCounter = 0;
	    }
	}

	printf("dishum\n");

	if(addCounter > 0){
	    if(addCounter < subCounter){
		for(size_t i = subCounter; i < MAX_BULLETS; i++){
		    bulletArray[i].ttl -= dT;
		    if(bulletArray[i].ttl < 0){
			subCounter = i+1;
			continue; // lol claude
		    }
		    bulletArray[i].pos.x += bulletArray[i].vec.x * bulletArray[i].speed * dT;
		    bulletArray[i].pos.y += bulletArray[i].vec.y * bulletArray[i].speed * dT;
		    DrawRectangleV(bulletArray[i].pos, (Vector2){.x = BULLET_SIZE, .y = BULLET_SIZE}, WHITE);
		    if(subCounter > MAX_BULLETS) subCounter = 0;
		}
		for(size_t i = 0; i < addCounter; i++){
		    bulletArray[i].ttl -= dT;
		    if(bulletArray[i].ttl < 0){
			subCounter = i+1;
		    }
		    bulletArray[i].pos.x += bulletArray[i].vec.x * bulletArray[i].speed * dT;
		    bulletArray[i].pos.y += bulletArray[i].vec.y * bulletArray[i].speed * dT;
		    DrawRectangleV(bulletArray[i].pos, (Vector2){.x = BULLET_SIZE, .y = BULLET_SIZE}, WHITE);
		}
	    }
	    else{
		for(size_t i = subCounter; i < addCounter; i++){
		    bulletArray[i].ttl -= dT;
		    if(bulletArray[i].ttl < 0){
			subCounter = i+1;
			continue; // lol claude
		    }
		    bulletArray[i].pos.x += bulletArray[i].vec.x * bulletArray[i].speed * dT;
		    bulletArray[i].pos.y += bulletArray[i].vec.y * bulletArray[i].speed * dT;
		    DrawRectangleV(bulletArray[i].pos, (Vector2){.x = BULLET_SIZE, .y = BULLET_SIZE}, WHITE);
		}
	    }
	    printf("%d, %d\n", addCounter, subCounter);
	}

	DrawPlayer(&mousePos, &p1, theta);
	DrawFPS(20, 20);
	EndDrawing();
    }
    arena_destroy(&bulletArena);
    return 0;
}
