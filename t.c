#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/mman.h>

#define WIDTH  800
#define HEIGHT 480
#define LCD_SIZE (WIDTH * HEIGHT * 4)

int *lcd_buf;
int lcd_fd;

// 显示BMP图片
int show_bmp(const char *pathname)
{
    int pic = open(pathname, O_RDONLY);
    if (pic < 0)
    {
        perror("open bmp");
        return -1;
    }

    unsigned char header[54];
    read(pic, header, 54);
    lseek(pic, 54, SEEK_SET);

    int img_width = *(int *)&header[18];
    int img_height = *(int *)&header[22];
    if (img_height < 0)
        img_height = -img_height;

    int bpp = *(short *)&header[28];
    int pixel_bytes = bpp / 8;
    int row_size = ((img_width * pixel_bytes + 3) / 4) * 4;
    int data_size = row_size * img_height;

    unsigned char *bmp_data = malloc(data_size);
    if (!bmp_data)
    {
        close(pic);
        return -1;
    }

    int ret = read(pic, bmp_data, data_size);
    if (ret != data_size)
    {
        free(bmp_data);
        close(pic);
        return -1;
    }

    // 清屏
    memset(lcd_buf, 0, LCD_SIZE);

    // 居中显示
    int start_x = (WIDTH - img_width) / 2;
    int start_y = (HEIGHT - img_height) / 2;

    for (int y = 0; y < img_height && y + start_y < HEIGHT; y++)
    {
        for (int x = 0; x < img_width && x + start_x < WIDTH; x++)
        {
            // 翻转：读取最后一行作为第一行
            int src_y = img_height - 1 - y;
            int src_idx = src_y * row_size + x * pixel_bytes;
            int dst_idx = (y + start_y) * WIDTH + (x + start_x);

            if (pixel_bytes == 4)
            {
                int b = bmp_data[src_idx];
                int g = bmp_data[src_idx + 1];
                int r = bmp_data[src_idx + 2];
                lcd_buf[dst_idx] = (r << 16) | (g << 8) | b;
            }
            else
            {
                int b = bmp_data[src_idx];
                int g = bmp_data[src_idx + 1];
                int r = bmp_data[src_idx + 2];
                lcd_buf[dst_idx] = (r << 16) | (g << 8) | b;
            }
        }
    }

    free(bmp_data);
    close(pic);
    return 0;
}

// 获取触摸坐标
void get_touch(int *x, int *y)
{
    int ts_fd = open("/dev/input/event0", O_RDONLY);
    if (ts_fd == -1)
    {
        *x = -1;
        *y = -1;
        return;
    }

    struct input_event event;
    *x = -1;
    *y = -1;

    while (1)
    {
        read(ts_fd, &event, sizeof(event));

        if (event.type == EV_ABS)
        {
            if (event.code == ABS_X)
                *x = event.value * 800 / 1024;
            if (event.code == ABS_Y)
                *y = event.value * 480 / 600;
        }

        if (event.type == EV_KEY && event.code == BTN_TOUCH && event.value == 0)
        {
            break;
        }
    }

    close(ts_fd);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("用法: %s <BMP图片路径>\n", argv[0]);
        printf("示例: %s menu.bmp\n", argv[0]);
        return -1;
    }

    // 打开LCD
    lcd_fd = open("/dev/fb0", O_RDWR);
    if (lcd_fd < 0)
    {
        perror("open lcd");
        return -1;
    }

    lcd_buf = mmap(NULL, LCD_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (lcd_buf == MAP_FAILED)
    {
        perror("mmap");
        close(lcd_fd);
        return -1;
    }

    // 显示图片
    printf("正在显示: %s\n", argv[1]);
    show_bmp(argv[1]);

    printf("========================================\n");
    printf("  触摸屏幕任意位置，打印坐标\n");
    printf("  按 Ctrl+C 退出\n");
    printf("========================================\n");

    int x, y;
    int count = 0;

    while (1)
    {
        get_touch(&x, &y);
        if (x >= 0 && y >= 0)
        {
            count++;
            printf("[%d] 触摸坐标: (%d, %d)\n", count, x, y);
        }
        usleep(50000);
    }

    munmap(lcd_buf, LCD_SIZE);
    close(lcd_fd);

    return 0;
}
