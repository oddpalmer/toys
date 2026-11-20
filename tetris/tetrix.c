#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define GAME_NAME "tetris"
#define GAME_FPS 300

#define COL 12
#define ROW 30
#define BUFFER_ROW 4
#define MONITOR_ROW 5

#define BLOCK_WIDTH 30
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
#define FULL_COLOR GREEN

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
bool be_killed(int kill_line);
void check_full_row();
bool can_fall();
void update_map();
void update_gen_block(int type);
void update_move(int input, int dir);
void update_fall();
void update_rotate();
void update_clear_row();
void update_restart();
void render_game(int score, int type);
void render_edge(int pos_x, int pos_y, int row, int col, int width, int height, Color color);
void render_block(int pos_x, int pos_y, Color color);
void render_map_block();
void render_active_block();
void render_next_block();
void render_game_over();
void branch_game_running();
void branch_game_over();


struct BLOCK{
    int pos[4][2];
}block;


int game_map[ROW][COL], full_row[ROW];

int main(){
    init();
    bool can_gen_block = 1;
    bool is_game_end = 0;
    int type = rand() % 7;
    int next_type = rand() % 7;
    int kill_line = BUFFER_ROW, score = 0;
    double UPDATE_FALL_INTERVAL = 0.2, dt = 0;
    while(!WindowShouldClose()){
        if(!is_game_end){
            dt += GetFrameTime();
            if(!can_gen_block && IsKeyPressed(KEY_A)) update_move(KEY_A, -1);
            if(!can_gen_block && IsKeyPressed(KEY_D)) update_move(KEY_D, +1);
            if(!can_gen_block && IsKeyDown(KEY_SPACE)&& can_fall()) update_fall();
            if(!can_gen_block && IsKeyPressedRepeat(KEY_S)&& can_fall()) update_fall();
            if(!can_gen_block && IsKeyPressed(KEY_W) && type != 2) update_rotate();

            if(can_gen_block) {
                update_gen_block(type);
                score += 400;
                can_gen_block = 0;
            }

            if(dt >= UPDATE_FALL_INTERVAL){
                dt -= UPDATE_FALL_INTERVAL;
                if(can_fall()) update_fall();
                else can_gen_block = 1;
            }

            if(can_gen_block){
                type = next_type;
                next_type = rand() % 7;
                update_map();
                check_full_row();
                update_clear_row();
                is_game_end = be_killed(kill_line);
            }
        }

        if(is_game_end){
            dt = 0;
            if(IsKeyPressed(KEY_R)) {
                memset(game_map, 0, sizeof(game_map));
                memset(block.pos,0, sizeof(block.pos));
                can_gen_block = 1;
                score = 0;
                dt = 0;
                is_game_end = 0;
            }
        }

        BeginDrawing();
        ClearBackground(BG_COLOR);
        if(!is_game_end) render_game(score, next_type);
        if(is_game_end) render_game_over();
        EndDrawing();
    }
}

void init(){
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME);
    SetTargetFPS(GAME_FPS);
    srand((int)time(NULL));
    memset(game_map, 0, sizeof(game_map));
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

void check_full_row(){
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            if(!game_map[i][j]){
                full_row[i] = 0;
                break;
            }
            if(j == COL - 1) full_row[i] = 1;
        }
    }
}

void update_rotate(){
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

void update_gen_block(int type){
    int spawn_col = rand() % (COL - BUFFER_ROW + 1);
    int cnt = 0;
    for(int i = 0; i < BUFFER_ROW; i++){
        for(int j = 0; j < BUFFER_ROW; j++){
            int nj = j + spawn_col;
            if(blocks[type][i][j] == 1){
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
    for(int i = ROW - 1; i >= 0; i--){
        if(!full_row[i]) memcpy(new_map[new_row--], game_map[i], sizeof(game_map[i]));
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

void render_game(int score, int next_type){
    DrawFPS(0, 0);
    DrawText(TextFormat("SCORE: %d",score),LEFT_PADDING + MAIN_WIDTH + MID_GAP, TOP_PADDING + MONITOR_WIDTH + 2 * BLOCK_WIDTH, BLOCK_WIDTH - 10, BLACK);

    render_edge(LEFT_PADDING, TOP_PADDING, ROW - BUFFER_ROW, COL, MAIN_WIDTH, MAIN_HEIGHT, EDGE_COLOR); // main
    render_edge(LEFT_PADDING + MAIN_WIDTH + MID_GAP, TOP_PADDING, MONITOR_ROW, MONITOR_ROW, MONITOR_WIDTH, MONITOR_WIDTH, EDGE_COLOR);

    // render static block on map
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            if(game_map[i][j]){
                int pos_x = LEFT_PADDING + j * BLOCK_WIDTH;
                int pos_y = TOP_PADDING + (i - BUFFER_ROW) * BLOCK_WIDTH;
                if(i >= BUFFER_ROW) render_block(pos_x, pos_y,STATIC_BLOCK_COLOR);
            }
        }
    }
    // render active block
    for(int i = 0; i < 4; i++){
        int row = block.pos[i][0];
        int col = block.pos[i][1];
        int pos_x = LEFT_PADDING + col * BLOCK_WIDTH;
        int pos_y = TOP_PADDING + (row - BUFFER_ROW) * BLOCK_WIDTH;
        if(row >= BUFFER_ROW) render_block(pos_x, pos_y,DYNAMIC_BLOCK_COLOR);
    }

    // render next block on monitor
    for(int i = 0; i < BUFFER_ROW; i++){
        for(int j = 0; j < BUFFER_ROW; j++){
            if(blocks[next_type][i][j]){
                render_block(LEFT_PADDING + MAIN_WIDTH + MID_GAP + (j + 1) * BLOCK_WIDTH, TOP_PADDING + (i + 1) * BLOCK_WIDTH, STATIC_BLOCK_COLOR);
            }
        }
    }
}

void render_block(int pos_x, int pos_y, Color color){
    DrawRectangle(pos_x, pos_y, BLOCK_WIDTH, BLOCK_WIDTH, color);
}

void render_edge(int pos_x, int pos_y, int row, int col, int width, int height, Color color){
    for(int i = 0; i <= row; i++) DrawRectangle(pos_x, pos_y + i * BLOCK_WIDTH, width, EDGE_WIDTH, color);
    for(int j = 0; j <= col; j++) DrawRectangle(pos_x + j * BLOCK_WIDTH, pos_y, EDGE_WIDTH, height, color);
}

bool be_killed(int kill_line){
    for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            if(game_map[i][j] && i < kill_line) {
                return true;
            }
        }
    }
    return false;
}

void render_game_over(){
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(WHITE,1));

    const char* main_text = "GAME OVER";
    const int main_size = 60;
    int main_width = MeasureText(main_text, main_size);
    int main_x = (WINDOW_WIDTH - main_width) / 2;
    int main_y = (WINDOW_HEIGHT - main_size) / 2;

    const int prompt_size = 30;
    const char* restart = "Prees R to Restart";
    const char* exit = "Press Esc to Exit";

    int restart_width = MeasureText(restart, prompt_size);
    int exit_width = MeasureText(exit, prompt_size);
    int restart_x = (WINDOW_WIDTH - restart_width) / 2;
    int exit_x = (WINDOW_WIDTH - exit_width) / 2;

    DrawText(main_text,main_x, main_y - 4 * prompt_size, main_size, BLACK);
    DrawText(restart, restart_x, main_y + 2 * prompt_size, prompt_size, BLACK);
    DrawText(exit, exit_x, main_y + 4 * prompt_size, prompt_size, BLACK);
}
