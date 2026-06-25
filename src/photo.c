#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "photo.h"
#include "bmp.h"
#include "utils.h"

#define MAX 5
#define W 800
#define H 480

static int cur = 0;

void photo_init(void)
{
    cur = 0;
}

void photo_show(int idx)
{
    char path[256];
    cur = idx;
    sprintf(path, "%s%d.bmp", PHOTO_PATH, idx + 1);
    showbmp(path);

    draw_btn(30, 440, 80, 35, 0x00FFFFFF, 0x000044FF, "PREV", 0x00FFFFFF);
    draw_btn(360, 440, 80, 35, 0x00FFFFFF, 0x00FF0000, "EXIT", 0x00FFFFFF);
    draw_btn(690, 440, 80, 35, 0x00FFFFFF, 0x0000FF00, "NEXT", 0x00FFFFFF);

    char info[32];
    sprintf(info, "PHOTO %d/%d", cur + 1, MAX);
    draw_str(460, 448, info, 0x00FFFFFF, -1);

    printf("[photo] show %d\n", idx + 1);
}

void photo_next(void)
{
    cur++;
    if (cur >= MAX) cur = 0;
    photo_show(cur);
}

void photo_prev(void)
{
    cur--;
    if (cur < 0) cur = MAX - 1;
    photo_show(cur);
}

int photo_getidx(void)
{
    return cur;
}

int photo_getcnt(void)
{
    return MAX;
}

void photo_loop(void)
{
    int x = 0, y = 0;
    int ts_fd = open("/dev/input/event0", O_RDONLY);
    struct input_event ev;
    int running = 1;

    if (ts_fd < 0) {
        perror("open touch");
        return;
    }

    photo_show(0);

    while (running) {
        read(ts_fd, &ev, sizeof(ev));

        if (ev.type == EV_ABS) {
            if (ev.code == ABS_X) x = ev.value * W / 1024;
            if (ev.code == ABS_Y) y = ev.value * H / 600;
        }

        if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0) {
            printf("[photo] touch (%d,%d)\n", x, y);

            if (x > 30 && x < 110 && y > 440 && y < 475) {
                photo_prev();
            } else if (x > 360 && x < 440 && y > 440 && y < 475) {
                running = 0;
                printf("[photo] exit\n");
            } else if (x > 690 && x < 770 && y > 440 && y < 475) {
                photo_next();
            }
        }
    }

    close(ts_fd);
}