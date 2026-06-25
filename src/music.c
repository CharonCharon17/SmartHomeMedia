#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "music.h"
#include "bmp.h"
#include "utils.h"

#define MAX 3
#define W 800
#define H 480

static int i = 0;
static int paused = 0;
static char *song[] = {"1.mp3", "2.mp3", "3.mp3"};

void music_init(void)
{
    i = 0;
    paused = 0;
    system("killall madplay 2>/dev/null");
}

void music_play(int idx)
{
    char cmd[256];
    i = idx;
    sprintf(cmd, "madplay %s%s &", MUSIC_PATH, song[idx]);
    system(cmd);
    paused = 0;
    printf("[music] play %s\n", song[idx]);
}

void music_stop(void)
{
    system("killall -KILL madplay 2>/dev/null");
    paused = 0;
    printf("[music] stop\n");
}

void music_pause(void)
{
    system("killall -STOP madplay 2>/dev/null");
    paused = 1;
    printf("[music] pause\n");
}

void music_resume(void)
{
    system("killall -CONT madplay 2>/dev/null");
    paused = 0;
    printf("[music] resume\n");
}

void music_prev(void)
{
    music_stop();
    i--;
    if (i < 0) i = MAX - 1;
    printf("[music] prev: %s\n", song[i]);
    music_play(i);
}

void music_next(void)
{
    music_stop();
    i++;
    if (i >= MAX) i = 0;
    printf("[music] next: %s\n", song[i]);
    music_play(i);
}

int music_getidx(void)
{
    return i;
}

int music_getcnt(void)
{
    return MAX;
}

void music_loop(void)
{
    int x = 0, y = 0;
    int ts_fd = open("/dev/input/event0", O_RDONLY);
    struct input_event ev;
    int running = 1;

    if (ts_fd < 0) {
        perror("open touch");
        return;
    }

    showbmp(IMG_PATH "music.bmp");
    music_play(0);

    while (running) {
        read(ts_fd, &ev, sizeof(ev));

        if (ev.type == EV_ABS) {
            if (ev.code == ABS_X) x = ev.value * W / 1024;
            if (ev.code == ABS_Y) y = ev.value * H / 600;
        }

        if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0) {
            printf("[music] touch (%d,%d)\n", x, y);

            if (x < 100 && y < 96) {
                music_stop();
                running = 0;
                printf("[music] exit\n");
            } else if (x > 700 && y < 96) {
                music_stop();
                i = 0;
                music_play(i);
            } else if (x > 700 && y > 96 && y < 192) {
                music_resume();
            } else if (x > 700 && y > 192 && y < 288) {
                music_pause();
            } else if (x > 700 && y > 288 && y < 384) {
                music_next();
            } else if (x > 700 && y > 384) {
                music_prev();
            }
        }
    }

    close(ts_fd);
}