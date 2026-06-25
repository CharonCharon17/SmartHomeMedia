#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "bmp.h"
#include "touch.h"
#include "menu.h"
#include "music.h"
#include "photo.h"
#include "video.h"
#include "game.h"
#include "utils.h"

#define W 800
#define H 480
#define LSIZE (W * H * 4)

int *fbuf;
int ffd;

void lcd_clear(void)
{
    memset(fbuf, 0, LSIZE);
}

int main(void)
{
    int x, y, btn;

    ffd = open("/dev/fb0", O_RDWR);
    if (ffd < 0) { perror("open lcd"); return -1; }

    fbuf = mmap(NULL, LSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ffd, 0);
    if (fbuf == MAP_FAILED) { perror("mmap"); close(ffd); return -1; }

    music_init();
    photo_init();

    while (1) {
        showmenu();
        printf("[main] menu\n");

        while (1) {
            getxy(&x, &y);
            if (x < 0 || y < 0) continue;

            btn = chkbtn(x, y);
            if (btn == 0) continue;

            printf("[main] btn=%d at (%d,%d)\n", btn, x, y);
            break;
        }

        switch (btn) {
            case 1:
                music_loop();
                break;

            case 2:
                photo_loop();
                break;

            case 3:
                video_loop();
                break;

            case 4:
                printf("[main] 启动 Piano 钢琴\n");
                system("killall madplay mplayer 2>/dev/null");
                system("cd /mnt/udisk/shixun/shm && ./piano");
                break;

            case 5:
                music_stop();
                video_stop();
                lcd_clear();
                munmap(fbuf, LSIZE);
                close(ffd);
                printf("[main] exit\n");
                return 0;
        }
    }

    return 0;
}