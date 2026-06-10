/*
 * Cifre giganti in pseudo-grafica PETSCII, modalita' testo standard.
 * Ogni cifra e' una matrice 5x5 di bit raddoppiata in verticale (5x10
 * celle a schermo), disegnata con lo screen code 160 (spazio in reverse,
 * cella piena con qualunque charset).
 */

#include "bigfont.h"

#define SCREEN ((unsigned char *)0x0400)
#define COLRAM ((unsigned char *)0xD800)

#define BLOCK 160
#define BLANK 32

/* 5 righe per cifra, bit 4..0 = colonne da sinistra a destra */
static const unsigned char font[10][5] = {
    {0x1F, 0x11, 0x11, 0x11, 0x1F},  /* 0 */
    {0x04, 0x0C, 0x04, 0x04, 0x0E},  /* 1 */
    {0x1F, 0x01, 0x1F, 0x10, 0x1F},  /* 2 */
    {0x1F, 0x01, 0x0F, 0x01, 0x1F},  /* 3 */
    {0x11, 0x11, 0x1F, 0x01, 0x01},  /* 4 */
    {0x1F, 0x10, 0x1F, 0x01, 0x1F},  /* 5 */
    {0x1F, 0x10, 0x1F, 0x11, 0x1F},  /* 6 */
    {0x1F, 0x01, 0x02, 0x04, 0x04},  /* 7 */
    {0x1F, 0x11, 0x1F, 0x11, 0x1F},  /* 8 */
    {0x1F, 0x11, 0x1F, 0x01, 0x1F},  /* 9 */
};

void big_put(unsigned char x, unsigned char y, unsigned char code,
             unsigned char color)
{
    unsigned off = y * 40U + x;
    SCREEN[off] = code;
    COLRAM[off] = color;
}

static void draw_digit(unsigned char x, unsigned char y, unsigned char d,
                       unsigned char color)
{
    unsigned char row, col, bits;

    for (row = 0; row < 10; ++row) {
        bits = font[d][row >> 1];        /* ogni riga del font vale 2 righe */
        for (col = 0; col < 5; ++col) {
            big_put(x + col, y + row,
                    (bits & (0x10 >> col)) ? BLOCK : BLANK, color);
        }
    }
}

static void draw_colon(unsigned char x, unsigned char y, unsigned char color)
{
    big_put(x, y + 2, BLOCK, color);
    big_put(x, y + 3, BLOCK, color);
    big_put(x, y + 6, BLOCK, color);
    big_put(x, y + 7, BLOCK, color);
}

void big_time(unsigned char x, unsigned char y, unsigned char mm,
              unsigned char ss, unsigned char color)
{
    draw_digit(x,      y, mm / 10, color);
    draw_digit(x + 6,  y, mm % 10, color);
    draw_colon(x + 12, y, color);
    draw_digit(x + 15, y, ss / 10, color);
    draw_digit(x + 21, y, ss % 10, color);
}
