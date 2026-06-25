#ifndef MUSIC_H
#define MUSIC_H

void music_init(void);
void music_play(int idx);
void music_stop(void);
void music_pause(void);
void music_resume(void);
void music_prev(void);
void music_next(void);
int music_getidx(void);
int music_getcnt(void);
void music_loop(void);

#endif