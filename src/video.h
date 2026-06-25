#ifndef VIDEO_H
#define VIDEO_H

void video_init(void);
void video_play(int idx);
void video_stop(void);
void video_cmd(char *cmd);
void video_prev(void);
void video_next(void);
void video_loop(void);

#endif