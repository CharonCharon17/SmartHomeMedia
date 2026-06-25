#ifndef PHOTO_H
#define PHOTO_H

void photo_init(void);
void photo_show(int idx);
void photo_next(void);
void photo_prev(void);
int photo_getidx(void);
int photo_getcnt(void);
void photo_loop(void);

#endif