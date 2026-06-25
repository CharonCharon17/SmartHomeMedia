#include <stdio.h>
#include "menu.h"
#include "bmp.h"
#include "utils.h"

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

int chkbtn(int x, int y)
{
    if (x >= M1 && x <= M3 && y >= M2 && y <= M4) return 1;
    if (x >= P1 && x <= P3 && y >= P2 && y <= P4) return 2;
    if (x >= V1 && x <= V3 && y >= V2 && y <= V4) return 3;
    if (x >= G1 && x <= G3 && y >= G2 && y <= G4) return 4;
    if (x >= E1 && x <= E3 && y >= E2 && y <= E4) return 5;
    return 0;
}

int showmenu(void)
{
    return showbmp(MENU_BMP);
}