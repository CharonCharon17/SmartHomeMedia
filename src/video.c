#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "video.h"
#include "bmp.h"
#include "utils.h"
#include <sys/stat.h>

#define W 800
#define H 480
#define MAX_VIDEO 2

static int video_idx = 0;
static int pipe_fd = -1;

// ========== 完全照抄 vp.c ==========
int init_fifo(void)
{
    int ret;
    if (access("/tmp/fifo", F_OK) == -1) {
        ret = mkfifo("/tmp/fifo", 0777);
        if (ret == -1) {
            printf("mkfifo error!\n");
            return -1;
        }
    }
    pipe_fd = open("/tmp/fifo", O_RDWR);
    if (pipe_fd < 0) {
        printf("open fifo error!\n");
        return -1;
    }
    printf("管道初始化成功\n");
    return 0;
}

// ========== 完全照抄 vp.c ==========
void write_cmd_to_fifo(char *cmd)
{
    write(pipe_fd, cmd, strlen(cmd));
}

void video_stop(void)
{
    system("killall mplayer 2>/dev/null");
    printf("[video] stop\n");
}

void video_prev(void)
{
    video_stop();
    video_idx--;
    if (video_idx < 0) video_idx = MAX_VIDEO - 1;
    printf("[video] prev: %d.avi\n", video_idx + 1);
}

void video_next(void)
{
    video_stop();
    video_idx++;
    if (video_idx >= MAX_VIDEO) video_idx = 0;
    printf("[video] next: %d.avi\n", video_idx + 1);
}

// ========== 完全照抄 vp.c 的 main 结构 ==========
void video_loop(void)
{
    int x = 0, y = 0;
    char cmd[128];
    int running = 1;

    // 1. 显示背景
    showbmp(IMG_PATH "video.bmp");

    // 显示当前视频编号
    char info[32];
    sprintf(info, "V:%d/%d", video_idx + 1, MAX_VIDEO);
    draw_str(10, 10, info, 0x00FFFFFF, -1);

    // 2. 先杀死旧进程
    system("killall mplayer 2>/dev/null");

    // 3. 启动 mplayer（完全照抄 vp.c）
    sprintf(cmd, "./mplayer -slave -quiet -input file=/tmp/fifo -zoom -idx -x 640 -y 480 %s%d.avi &", VIDEO_PATH, video_idx + 1);
    system(cmd);
    printf("[video] play %d.avi\n", video_idx + 1);

    // 4. 等待 mplayer 启动，然后初始化管道（vp.c 中在这步）
    usleep(500000);
    init_fifo();

    // 5. 打开触摸屏（完全照抄 vp.c）
    struct input_event buf;
    int ts = open("/dev/input/event0", O_RDWR);
    if (ts < 0) {
        printf("open touch error!\n");
        return;
    }

    // 6. 循环监听触摸（完全照抄 vp.c 的结构）
    while (running) {
        read(ts, &buf, sizeof(buf));

        if (buf.type == EV_ABS && buf.code == ABS_X) {
            x = buf.value * W / 1024;
        }
        if (buf.type == EV_ABS && buf.code == ABS_Y) {
            y = buf.value * H / 600;
        }

        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0) {
            printf("[video] touch (%d,%d)\n", x, y);

            // ====== 左边区域 (x < 710) ======
            // 对应 vp.c 的四个按钮，只是 x 范围从 >640 改为 <710
            if (x < 710 && y < 120) {
                printf("快进 10秒\n");
                write_cmd_to_fifo("seek 10\n");
            } else if (x < 710 && y > 120 && y < 240) {
                printf("快退 10秒\n");
                write_cmd_to_fifo("seek -10\n");
            } else if (x < 710 && y > 240 && y < 360) {
                printf("音量 +10\n");
                write_cmd_to_fifo("volume +10\n");
            } else if (x < 710 && y > 360) {
                printf("音量 -10\n");
                write_cmd_to_fifo("volume -10\n");
            }
            // ====== 右边区域 (x > 740) ======
            else if (x > 740 && y < 120) {
                printf("上一首\n");
                video_prev();
                // 重启 mplayer
                system("killall mplayer 2>/dev/null");
                sprintf(cmd, "./mplayer -slave -quiet -input file=/tmp/fifo -zoom -idx -x 640 -y 480 %s%d.avi &", VIDEO_PATH, video_idx + 1);
                system(cmd);
                usleep(500000);
                init_fifo();
                sprintf(info, "V:%d/%d", video_idx + 1, MAX_VIDEO);
                draw_str(10, 10, info, 0x00FFFFFF, -1);
            } else if (x > 740 && y > 120 && y < 240) {
                printf("暂停/继续\n");
                write_cmd_to_fifo("pause\n");
            } else if (x > 740 && y > 240 && y < 360) {
                printf("下一首\n");
                video_next();
                system("killall mplayer 2>/dev/null");
                sprintf(cmd, "./mplayer -slave -quiet -input file=/tmp/fifo -zoom -idx -x 640 -y 480 %s%d.avi &", VIDEO_PATH, video_idx + 1);
                system(cmd);
                usleep(500000);
                init_fifo();
                sprintf(info, "V:%d/%d", video_idx + 1, MAX_VIDEO);
                draw_str(10, 10, info, 0x00FFFFFF, -1);
            } else if (x > 740 && y > 360) {
                printf("退出播放\n");
                video_stop();
                running = 0;
            }
        }
    }

    close(ts);
    close(pipe_fd);
}