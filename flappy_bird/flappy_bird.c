#include <raylib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define PIPE_WIDTH 150
#define MINIMUM_HEIGHT 100
#define PIPE_GAP 200

#define BIRD_IMAGE_PATH "./flappy_bird/bird.png"
#define BIRD_IMAGE_SCALE 0.1f

#define GRAVITY 1200
#define CACHE_SIZE 20

typedef struct{
    bool is_alive;
    Rectangle top_rec;
    Rectangle bottom_rec;
}PIPE;

typedef struct BIRD{
    Rectangle body;
    float width;
    float height;
    float vy;
    Image image;
    Texture2D texture;
}BIRD;

PIPE cache[CACHE_SIZE];
BIRD bird;

void gen_pipe(){
    PIPE pipe;
    int top_height = MINIMUM_HEIGHT + rand() % (WINDOW_HEIGHT - PIPE_GAP - 2 * MINIMUM_HEIGHT);
    int bottom_y = top_height + PIPE_GAP;
    int bottom_height = WINDOW_HEIGHT - top_height - PIPE_GAP;
    pipe.is_alive = true;
    pipe.top_rec = (Rectangle){WINDOW_WIDTH, 0 - 5, PIPE_WIDTH, top_height + 5};
    pipe.bottom_rec = (Rectangle){WINDOW_WIDTH, bottom_y, PIPE_WIDTH, bottom_height + 5};

    for(int i = 0; i < CACHE_SIZE; i++){
        if(!cache[i].is_alive){
            cache[i] = pipe;
            return;
        }
    }
}

void move_pipes(float delta){
    for(int i = 0; i < CACHE_SIZE; i++){
        if(cache[i].is_alive){
            cache[i].top_rec.x -= delta * 200;
            cache[i].bottom_rec.x -= delta * 200;
        }
    }
}

void draw_pipes(){
    for(int i = 0; i < CACHE_SIZE; i++){
        if(cache[i].is_alive){
            DrawRectangleRec(cache[i].top_rec, GREEN);
            DrawRectangleRec(cache[i].bottom_rec, GREEN);
            DrawRectangleLinesEx(cache[i].top_rec, 5, BLACK);
            DrawRectangleLinesEx(cache[i].bottom_rec, 5, BLACK);

        }
    }
}

void remove_pipe(){
    for(int i = 0; i < CACHE_SIZE; i++){
        if(cache[i].is_alive && cache[i].top_rec.x + PIPE_WIDTH <= 0){
            cache[i].is_alive = false;
        }
    }
}

void init_bird(){
    bird.image = LoadImage(BIRD_IMAGE_PATH);
    ImageAlphaCrop(&bird.image, 0.1f);
    bird.texture = LoadTextureFromImage(bird.image);

    bird.width = bird.image.width * BIRD_IMAGE_SCALE;
    bird.height = bird.image.height * BIRD_IMAGE_SCALE;
    float pos_x = (WINDOW_WIDTH - bird.width) / 2.0;
    float pos_y = (WINDOW_HEIGHT - bird.height) / 2.0;
    bird.body = (Rectangle){pos_x, pos_y, bird.width, bird.height};
}

void draw_bird(){
    DrawTextureEx(bird.texture, (Vector2){bird.body.x, bird.body.y}, 0, BIRD_IMAGE_SCALE ,WHITE);
    // DrawRectangleLinesEx(bird.body, 5, BLACK); // hit-box
}

void fall_bird(float delta){
    if(bird.body.y + bird.vy * delta >= WINDOW_HEIGHT - bird.height){
        bird.vy = 0 * delta;
    } else {
        bird.vy += GRAVITY * delta;
    }
    bird.body.y += bird.vy * delta;
}

void jump_bird(){
    bird.vy = -500;
}

bool is_hit(){
    for(int i = 0; i < CACHE_SIZE; i++){
        if(cache[i].is_alive){
            if(CheckCollisionRecs(bird.body, cache[i].bottom_rec)) return true;
            if(CheckCollisionRecs(bird.body, cache[i].top_rec)) return true;
        }
    }
    return false;
}


int main(){
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flappy Bird");
    SetTargetFPS(300);
    srand((unsigned int)time(NULL));
    memset(cache, 0, sizeof(cache));
    init_bird();

    float pipe_dt = 0, delta;

    while(!WindowShouldClose()){
        delta = GetFrameTime();
        pipe_dt += delta;

        if(pipe_dt >= 2){
            pipe_dt -= 2;
            gen_pipe();
        }

        if(IsKeyPressed(KEY_SPACE)) jump_bird();
        // if(is_hit()) break;

        move_pipes(delta);
        remove_pipe();
        fall_bird(delta);

        BeginDrawing();
        ClearBackground(WHITE);
        draw_pipes();
        draw_bird();
        DrawFPS(0, 0);
        EndDrawing();
    }
    UnloadImage(bird.image);
    UnloadTexture(bird.texture);
    CloseWindow();
    return 0;
}
