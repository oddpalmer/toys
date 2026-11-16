#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


#define GAME_NAME "tetris"
#define GAME_FPS 300

#define COL 11
#define ROW 30
#define BUFFER_ROW 4
#define MONITOR_ROW 5

#define BLOCK_WIDTH 25
#define EDGE_WIDTH 3

#define TOP_PADDING 30
#define BOTTOM_PADDING 30
#define LEFT_PADDING 30
#define RIGHT_PADDING 30
#define MID_GAP 60

#define MAIN_WIDTH (COL * BLOCK_WIDTH + EDGE_WIDTH)
#define MAIN_HEIGHT ((ROW - BUFFER_ROW) * BLOCK_WIDTH + EDGE_WIDTH)
#define MONITOR_WIDTH (MONITOR_ROW * BLOCK_WIDTH + EDGE_WIDTH)
#define WINDOW_WIDTH  (MAIN_WIDTH + LEFT_PADDING + RIGHT_PADDING + MID_GAP + MONITOR_WIDTH)
#define WINDOW_HEIGHT (MAIN_HEIGHT + TOP_PADDING + BOTTOM_PADDING)

#define EDGE_COLOR (BLACK)
#define BG_COLOR (WHITE)
#define STATIC_BLOCK_COLOR (BLACK)
#define DYNAMIC_BLOCK_COLOR (RED)

#define max(a,b) ((a > b) ? (a) : (b))
#define min(a,b) ((a < b) ? (a) : (b))

const int blocks[7][BUFFER_ROW][BUFFER_ROW] = {
    {{1,0,0,0},{1,0,0,0},{1,1,0,0},{0,0,0,0}}, // L
    {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}, // 反向 L
    {{0,0,0,0},{1,1,0,0},{1,1,0,0},{0,0,0,0}}, // O
    {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, // Z
    {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, // S
    {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, // I
    {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, // T
};

void init();
void check_input();
void check_kill();
bool can_fall();
void update_map();
void update_gen_block();
void update_move(int input, int dir);
void update_fall();
void update_rotate();
void update_clear_row();
void render_game();
void render_edge(int pos_x, int pos_y, int row, int col, int width, int height, Color color);
void render_block(int pos_x, int pos_y, Color color);
void render_map_block();
void render_active_block();



struct BLOCK{
    int pos[4][2];
}block;

int game_map[ROW][COL];
int TYPE, KILL_LINE = BUFFER_ROW;
bool IS_GAME_RUNNING = true;
double UPDATE_FALL_INTERVAL = 0.2;


int main(){
    init();
    bool can_gen_block = 1;
    double dt = 0;
    while(!WindowShouldClose() && IS_GAME_RUNNING){
        dt += GetFrameTime();

        if(can_gen_block) {
            update_gen_block();
            can_gen_block = 0;
        }

        check_input();
        if(dt >= UPDATE_FALL_INTERVAL){
            dt -= UPDATE_FALL_INTERVAL;
            if(can_fall()) update_fall();
            else can_gen_block = 1;
        }

        if(can_gen_block){
            update_map();
            update_clear_row();
            check_kill();
        }

        BeginDrawing();
        ClearBackground(BG_COLOR);
        render_game();
        EndDrawing();
    }
}





void init(){
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME);
    SetTargetFPS(GAME_FPS);
    srand((int)time(NULL));
    memset(game_map, 0, sizeof(game_map));
}

void check_input(){
    if(IsKeyPressed(KEY_A)) update_move(KEY_A, -1);
    if(IsKeyPressed(KEY_D)) update_move(KEY_D, +1);
    if(IsKeyDown(KEY_SPACE)&& can_fall()) update_fall();
    if(IsKeyPressedRepeat(KEY_S)&& can_fall()) update_fall();
    if(IsKeyPressed(KEY_W)) update_rotate();
}

void update_rotate(){
    if(TYPE == 2) return;
    int min_row = block.pos[0][0], max_row = block.pos[0][0];
    int min_col = block.pos[0][1], max_col = block.pos[0][1];

    for(int i = 1; i < 4; i++){
        min_row = min(min_row, block.pos[i][0]);
        max_row = max(max_row, block.pos[i][0]);
        min_col = min(min_col, block.pos[i][1]);
        max_col = max(max_col, block.pos[i][1]);
    }

    int cy = (min_row + max_row) / 2;
    int cx = (min_col + max_col) / 2;
    int new_pos[4][2];

    for(int i = 0; i < 4; i++){
        int x = block.pos[i][1] - cx;   // 列偏移
        int y = block.pos[i][0] - cy;   // 行偏移

        int nx =  y;    // x' = y
        int ny = -x;    // y' = -x

        new_pos[i][0] = ny + cy;   // 新 row
        new_pos[i][1] = nx + cx;   // 新 col
    }

    for(int i = 0; i < 4; i++){
        int r = new_pos[i][0];
        int c = new_pos[i][1];
        if(r < 0 || r >= ROW || c < 0 || c >= COL) return;
        if(game_map[r][c] == 2) return;
    }

    memcpy(block.pos, new_pos, sizeof(new_pos));
}

bool can_fall(){
    for(int i = 0; i < 4; i++){
        int row = block.pos[i][0];
        int col = block.pos[i][1];
        if(row + 1 >= ROW) return false;
        if(game_map[row+1][col] == 2) return false;
    }
    return true;
}

void update_gen_block(){
    TYPE = rand() % 7; // TYPE 0 ~ 6
    int spawn_col = rand() % (COL - BUFFER_ROW + 1);
    int cnt = 0;
    for(int i = 0; i < BUFFER_ROW; i++){
        for(int j = 0; j < BUFFER_ROW; j++){
            int nj = j + spawn_col;
            if(blocks[TYPE][i][j] == 1){
                block.pos[cnt][0] = i;
                block.pos[cnt][1] = nj;
                cnt++;
            }
        }
    }
}

void update_move(int input, int dir){
    int row,col;
    for(int i = 0; i < 4; i++){
        row = block.pos[i][0];
        col = block.pos[i][1];
        if(input == KEY_A && (col - 1 < 0 || game_map[row][col - 1] == 2)) return;
        if(input == KEY_D && (col + 1 >= COL || game_map[row][col + 1] == 2)) return;
    }
    for(int i = 0; i < 4; i++){
        block.pos[i][1] += dir;
    }
}

void update_fall(){
    for(int i = 0; i < 4; i++){
        block.pos[i][0]++;
    }
}

void update_clear_row(){
    int new_map[ROW][COL];
    memset(new_map, 0, sizeof(new_map));
    int new_row = ROW - 1;
    for(int i = ROW - 1; i >= BUFFER_ROW; i--){
        for(int j = 0; j < COL; j++){
            if(!game_map[i][j]){
                memcpy(new_map[new_row--], game_map[i], sizeof(game_map[i]));
                break;
            }
        }
    }
    memcpy(game_map, new_map, sizeof(game_map));
}

void update_map(){
    for(int i = 0; i < 4; i++){
        int row = block.pos[i][0];
        int col = block.pos[i][1];
        game_map[row][col] = 2;
    }
}

void render_game(){
    DrawFPS(0, 0);
    render_edge(LEFT_PADDING, TOP_PADDING, ROW - BUFFER_ROW, COL, MAIN_WIDTH, MAIN_HEIGHT, EDGE_COLOR); // main
    render_edge(LEFT_PADDING + MAIN_WIDTH + MID_GAP, TOP_PADDING, MONITOR_ROW, MONITOR_ROW, MONITOR_WIDTH, MONITOR_WIDTH, EDGE_COLOR);
    render_map_block();
    render_active_block();
}

void render_block(int pos_x, int pos_y, Color color){
    DrawRectangle(pos_x, pos_y, BLOCK_WIDTH, BLOCK_WIDTH, color);
}

void render_map_block(){
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            if(game_map[i][j]){
                int pos_x = LEFT_PADDING + j * BLOCK_WIDTH;
                int pos_y = TOP_PADDING + (i - BUFFER_ROW) * BLOCK_WIDTH;
                if(i >= BUFFER_ROW) render_block(pos_x, pos_y,STATIC_BLOCK_COLOR);
            }
        }
    }
}

void render_active_block(){
    for(int i = 0; i < 4; i++){
        int row = block.pos[i][0];
        int col = block.pos[i][1];
        int pos_x = LEFT_PADDING + col * BLOCK_WIDTH;
        int pos_y = TOP_PADDING + (row - BUFFER_ROW) * BLOCK_WIDTH;
        if(row >= BUFFER_ROW) render_block(pos_x, pos_y,DYNAMIC_BLOCK_COLOR);
    }
}

void render_edge(int pos_x, int pos_y, int row, int col, int width, int height, Color color){
    for(int i = 0; i <= row; i++) DrawRectangle(pos_x, pos_y + i * BLOCK_WIDTH, width, EDGE_WIDTH, color);
    for(int j = 0; j <= col; j++) DrawRectangle(pos_x + j * BLOCK_WIDTH, pos_y, EDGE_WIDTH, height, color);
}

void check_kill(){
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            if(game_map[i][j] && i < KILL_LINE) {
                IS_GAME_RUNNING = false;
                return;
            }
        }
    }
}
