#ifndef BMP_H
#define BMP_H

int showbmp(const char *path);
void draw_char(int x, int y, char ch, int color, int bg);
void draw_str(int x, int y, char *str, int color, int bg);
void draw_btn(int x, int y, int w, int h, int border, int fill, char *text, int text_color);

#endif