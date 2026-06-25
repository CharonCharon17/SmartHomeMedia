#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include "touch.h"

#define W 800
#define H 480

void getxy(int *x, int *y)
{
    int fd = open("/dev/input/event0", O_RDONLY);
    if (fd < 0) { *x = -1; *y = -1; return; }

    struct input_event ev;
    *x = -1;
    *y = -1;

    while (1) {
        read(fd, &ev, sizeof(ev));
        if (ev.type == EV_ABS) {
            if (ev.code == ABS_X) *x = ev.value * W / 1024;
            if (ev.code == ABS_Y) *y = ev.value * H / 600;
        }
        if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0) break;
    }
    close(fd);
}