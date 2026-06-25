#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/mman.h>

#define W 800
#define H 480
#define LSIZE (W * H * 4)

// 按钮坐标 (左上 + 右下)
#define M1 70
#define M2 170
#define M3 210
#define M4 285

#define P1 210
#define P2 170
#define P3 340
#define P4 285

#define V1 340
#define V2 170
#define V3 470
#define V4 285

#define G1 470
#define G2 170
#define G3 600
#define G4 285

#define E1 600
#define E2 170
#define E3 710
#define E4 285

int *fbuf;
int ffd;

int showbmp(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    unsigned char hdr[54];
    read(fd, hdr, 54);
    lseek(fd, 54, SEEK_SET);

    int iw = *(int *)&hdr[18];
    int ih = *(int *)&hdr[22];
    if (ih < 0) ih = -ih;

    int bpp = *(short *)&hdr[28];
    int pb = bpp / 8;
    int rs = ((iw * pb + 3) / 4) * 4;
    int ds = rs * ih;

    unsigned char *data = malloc(ds);
    if (!data) { close(fd); return -1; }

    read(fd, data, ds);
    memset(fbuf, 0, LSIZE);

    int sx = (W - iw) / 2;
    int sy = (H - ih) / 2;

    for (int y = 0; y < ih && y + sy < H; y++) {
        for (int x = 0; x < iw && x + sx < W; x++) {
            int sy2 = ih - 1 - y;
            int si = sy2 * rs + x * pb;
            int di = (y + sy) * W + (x + sx);
            int b = data[si];
            int g = data[si + 1];
            int r = data[si + 2];
            fbuf[di] = (r << 16) | (g << 8) | b;
        }
    }

    free(data);
    close(fd);
    return 0;
}

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

int chkbtn(int x, int y)
{
    if (x >= M1 && x <= M3 && y >= M2 && y <= M4) return 1;
    if (x >= P1 && x <= P3 && y >= P2 && y <= P4) return 2;
    if (x >= V1 && x <= V3 && y >= V2 && y <= V4) return 3;
    if (x >= G1 && x <= G3 && y >= G2 && y <= G4) return 4;
    if (x >= E1 && x <= E3 && y >= E2 && y <= E4) return 5;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("use: %s <bmp>\n", argv[0]);
        return -1;
    }

    ffd = open("/dev/fb0", O_RDWR);
    if (ffd < 0) { perror("open lcd"); return -1; }

    fbuf = mmap(NULL, LSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ffd, 0);
    if (fbuf == MAP_FAILED) { perror("mmap"); close(ffd); return -1; }

    showbmp(argv[1]);

    printf("touch screen...\n");

    int x, y;
    while (1) {
        getxy(&x, &y);
        if (x >= 0 && y >= 0) {
            int btn = chkbtn(x, y);
            printf("(%d,%d) btn=%d\n", x, y, btn);
        }
    }

    munmap(fbuf, LSIZE);
    close(ffd);
    return 0;
}
