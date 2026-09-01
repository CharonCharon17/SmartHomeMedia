# 智能家庭影音系统

基于 GEC6818 开发板（ARM Cortex-A53）与嵌入式 Linux 的智能影音系统，支持音乐播放、视频播放、图片相册浏览及钢琴游戏。

## 功能模块

- 音乐播放：调用 madplay 实现 MP3 解码播放，支持暂停/继续/切换
- 视频播放：调用 mplayer 实现 AVI 格式播放，通过命名管道（FIFO）控制快进/快退/音量
- 图片相册：基于 Framebuffer 显示 BMP 图片，支持上一张/下一张切换
- 钢琴游戏：多线程实现多音同时发声

## 目录结构

- src/main.c - 主程序入口
- src/menu.c/h - 主菜单界面
- src/music.c/h - 音乐播放模块
- src/photo.c/h - 图片相册模块
- src/video.c/h - 视频播放模块
- src/game.c/h - 钢琴游戏模块
- src/bmp.c/h - BMP图片解析与显示
- src/touch.c/h - 触摸屏驱动
- src/utils.c/h - 工具函数

## 技术栈

- 硬件平台：GEC6818 开发板（ARM Cortex-A53）
- 操作系统：嵌入式 Linux
- 开发语言：C语言
- 显示驱动：Framebuffer（LCD 显存映射）
- 输入系统：Linux 输入子系统（触摸坐标采集与校准）
- 多媒体解码：madplay、mplayer
- 进程控制：SIGSTOP/SIGCONT/SIGKILL 信号
- 进程间通信：命名管道（FIFO）

## 编译与运行

编译：
make clean && make

运行：
./shm

## 使用说明

1. 程序启动后进入主菜单，显示五个功能按钮
2. 触摸点击对应按钮进入相应功能模块
3. 各模块内通过按钮进行播放控制或浏览切换
4. 点击退出按钮返回主菜单或退出程序

## 备注

音乐、视频、图片素材请自行准备，放置于 resources/ 目录下。
