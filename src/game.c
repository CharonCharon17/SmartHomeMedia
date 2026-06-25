#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <time.h>
#include "game.h"
#include "bmp.h"

#define WIDTH  800
#define HEIGHT 480
#define LCD_SIZE (WIDTH * HEIGHT * 4)

extern int *fbuf;

int score = 0;
int game_over = 0;
int mole_x = -1, mole_y = -1;

// ========== 触摸检测：阻塞读取，获取X和Y后立即返回 ==========
int get_touch(int *x, int *y)
{
    int tsfd = open("/dev/input/event0", O_RDONLY);
    if (tsfd == -1) return -1;

    struct input_event event;
    int xpos = -1, ypos = -1;
    int got_x = 0, got_y = 0;

    while (1) {
        if (read(tsfd, &event, sizeof(event)) != sizeof(event)) {
            close(tsfd);
            return -1;
        }

        if (event.type == EV_ABS) {
            if (event.code == ABS_X) {
                xpos = event.value * 800 / 1024;
                got_x = 1;
            }
            if (event.code == ABS_Y) {
                ypos = event.value * 480 / 600;
                got_y = 1;
            }
            if (got_x && got_y) {
                *x = xpos;
                *y = ypos;
                close(tsfd);
                return 1;   // 返回1表示有效触摸
            }
        }
    }
    close(tsfd);
    return 0;
}

// ========== 绘图函数 ==========
void draw_rect(int x, int y, int w, int h, int color)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > WIDTH) w = WIDTH - x;
    if (y + h > HEIGHT) h = HEIGHT - y;
    if (w <= 0 || h <= 0) return;
    for (int i = 0; i < h; i++) {
        int idx = (y + i) * WIDTH + x;
        for (int j = 0; j < w; j++) {
            fbuf[idx + j] = color;
        }
    }
}

void draw_circle(int cx, int cy, int r, int color)
{
    int r2 = r * r;
    for (int i = -r; i <= r; i++) {
        for (int j = -r; j <= r; j++) {
            if (i*i + j*j < r2) {
                int dx = cx + j, dy = cy + i;
                if (dx >= 0 && dx < WIDTH && dy >= 0 && dy < HEIGHT) {
                    fbuf[dy * WIDTH + dx] = color;
                }
            }
        }
    }
}

void draw_number(int cx, int cy, int num, int color)
{
    char text[4];
    sprintf(text, "%d", num);
    draw_str(cx - 8, cy - 10, text, color, -1);
}

void draw_background(void)
{
    for (int i = 0; i < WIDTH * HEIGHT; i++)
        fbuf[i] = 0x00228B22;

    int cell_w = 800 / 3;
    int cell_h = 480 / 3;
    int hole_num = 1;

    // 打印洞坐标信息（调试）
    printf("\n[洞坐标信息]\n");
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int cx = col * cell_w + cell_w / 2;
            int cy = row * cell_h + cell_h / 2;
            int radius = 60;
            printf("洞 %d: 中心(%d, %d), 半径%d\n", hole_num, cx, cy, radius);
            // 画洞
            for (int y = -radius; y <= radius; y++) {
                for (int x = -radius; x <= radius; x++) {
                    if (x*x + y*y < radius*radius) {
                        int px = cx + x;
                        int py = cy + y;
                        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT)
                            fbuf[py * WIDTH + px] = 0x008B4513;
                    }
                }
            }
            draw_number(cx, cy, hole_num, 0x00FFFFFF);
            hole_num++;
        }
    }
    printf("========================\n\n");
}

void draw_mole(void)
{
    if (mole_x < 0 || mole_y < 0) return;

    int cell_w = 800 / 3;
    int cell_h = 480 / 3;
    int cx = mole_x * cell_w + cell_w / 2;
    int cy = mole_y * cell_h + cell_h / 2;
    int radius = 40;

    // 打印地鼠位置（调试）
    printf("[地鼠] 位置: 洞 %d 中心(%d, %d)\n", mole_y * 3 + mole_x + 1, cx, cy);

    // 画身体
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y < radius*radius) {
                int px = cx + x;
                int py = cy + y;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT)
                    fbuf[py * WIDTH + px] = 0x00A9A9A9;
            }
        }
    }

    // 眼睛
    int eye_radius = 8;
    int eye_pos[2][2] = {{-18, -10}, {18, -10}};
    for (int e = 0; e < 2; e++) {
        int ex = cx + eye_pos[e][0];
        int ey = cy + eye_pos[e][1];
        for (int y = -eye_radius; y <= eye_radius; y++) {
            for (int x = -eye_radius; x <= eye_radius; x++) {
                if (x*x + y*y < eye_radius*eye_radius) {
                    int px = ex + x;
                    int py = ey + y;
                    if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT)
                        fbuf[py * WIDTH + px] = 0x00FFFFFF;
                }
            }
        }
        int px = ex + 4;
        int py = ey + 0;
        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT)
            fbuf[py * WIDTH + px] = 0x00000000;
    }
}

void draw_score(void)
{
    char text[32];
    sprintf(text, "SCORE: %d", score);
    draw_rect(8, 8, 190, 40, 0x00000000);
    draw_rect(8, 8, 190, 40, 0x00FFFFFF);
    draw_rect(10, 10, 186, 36, 0x00000000);
    draw_str(20, 15, text, 0x00FFFFFF, -1);
}

void draw_time(int time_left)
{
    char text[32];
    sprintf(text, "TIME: %ds", time_left);
    draw_rect(8, 438, 110, 40, 0x00000000);
    draw_rect(8, 438, 110, 40, 0x00FFFFFF);
    draw_rect(10, 440, 106, 36, 0x00000000);
    draw_str(20, 445, text, 0x00FFFFFF, -1);
}

void draw_exit_button(void)
{
    draw_rect(718, 8, 76, 44, 0x00000000);
    draw_rect(720, 10, 70, 40, 0x00FFFFFF);
    draw_rect(722, 12, 66, 36, 0x00FF0000);
    draw_str(735, 18, "EXIT", 0x00FFFFFF, -1);
}

// ========== 游戏逻辑 ==========
void spawn_mole(void)
{
    mole_x = rand() % 3;
    mole_y = rand() % 3;
    int hole = mole_y * 3 + mole_x + 1;
    printf("[地鼠] 出现在 %d 号洞\n", hole);
}

int get_hole_number(int tx, int ty)
{
    int cell_w = 800 / 3;
    int cell_h = 480 / 3;
    int col = tx / cell_w;
    int row = ty / cell_h;
    if (col < 0 || col > 2 || row < 0 || row > 2) return -1;
    return row * 3 + col + 1;
}

int is_in_hole(int tx, int ty)
{
    int cell_w = 800 / 3;
    int cell_h = 480 / 3;
    int col = tx / cell_w;
    int row = ty / cell_h;
    if (col < 0 || col > 2 || row < 0 || row > 2) return 0;
    int cx = col * cell_w + cell_w / 2;
    int cy = row * cell_h + cell_h / 2;
    int dx = tx - cx;
    int dy = ty - cy;
    return (dx*dx + dy*dy < 60*60);
}

void game_loop(void)
{
    srand(time(NULL));

    while (1) {
        score = 0;
        game_over = 0;
        mole_x = -1;
        mole_y = -1;
        int exit_flag = 0;
        time_t start_time = time(NULL);
        int time_left = 30;

        draw_background();
        draw_score();
        draw_time(time_left);
        draw_exit_button();

        printf("\n========== 打地鼠游戏 ==========\n");
        printf("限时30秒，打中地鼠得分！\n");
        printf("洞编号: 1 2 3 / 4 5 6 / 7 8 9\n");
        printf("右上角 EXIT 退出\n");
        printf("================================\n\n");

        // 生成第一只地鼠
        spawn_mole();
        draw_background();
        draw_mole();
        draw_score();
        draw_time(time_left);
        draw_exit_button();

        while (!game_over && !exit_flag) {
            int elapsed = time(NULL) - start_time;
            time_left = 30 - elapsed;
            if (time_left <= 0) {
                game_over = 1;
                printf("[游戏] 时间到！游戏结束\n");
                break;
            }

            draw_time(time_left);

            int tx, ty;
            int touch_status = get_touch(&tx, &ty);

            if (touch_status == 1) {
                // 获取当前点击的洞编号和坐标范围
                int hole = get_hole_number(tx, ty);
                int in_hole = is_in_hole(tx, ty);
                int mole_hole = -1;
                if (mole_x >= 0 && mole_y >= 0) {
                    mole_hole = mole_y * 3 + mole_x + 1;
                }

                // 调试信息：显示点击位置和洞范围
                printf("[触摸] 坐标: (%d, %d)\n", tx, ty);
                printf("[调试] 点击洞: %d | 是否在洞内: %s | 地鼠在: %d号洞\n", 
                       hole, 
                       in_hole ? "是" : "否", 
                       mole_hole);

                // 检查退出按钮
                if (tx >= 720 && tx <= 790 && ty >= 10 && ty <= 50) {
                    printf("[退出] 点击退出按钮，返回主菜单\n");
                    exit_flag = 1;
                    break;
                }

                // 打地鼠判定
                if (hole >= 1 && hole <= 9) {
                    if (in_hole) {
                        if (mole_x >= 0 && mole_y >= 0) {
                            if (hole == mole_hole) {
                                printf("[打中] %d号洞，有地鼠！被打了！\n", hole);
                                score++;
                                // 立即生成下一只地鼠
                                spawn_mole();
                                draw_background();
                                draw_mole();
                                draw_score();
                                draw_time(time_left);
                                draw_exit_button();
                                printf("[得分] 当前得分: %d\n", score);
                            } else {
                                printf("[打空] %d号洞，地鼠在 %d 号洞\n", hole, mole_hole);
                            }
                        } else {
                            printf("[打空] %d号洞，没有地鼠\n", hole);
                        }
                    } else {
                        printf("[无效] 点击了 %d 号洞附近，但没点到洞\n", hole);
                    }
                } else {
                    printf("[无效] 点击了空白区域\n");
                }
            }

            usleep(30000);
        }

        if (exit_flag) break;

        // 游戏结束显示
        for (int i = 0; i < WIDTH * HEIGHT; i++)
            fbuf[i] = 0x00000000;
        printf("[游戏] 游戏结束！最终得分: %d\n", score);
        sleep(2);
    }
}