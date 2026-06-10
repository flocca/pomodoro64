#ifndef BIGFONT_H
#define BIGFONT_H

/* Scrive uno screen code e il suo colore direttamente in memoria video
   ($0400/$D800), senza passare da conio. */
void big_put(unsigned char x, unsigned char y, unsigned char code,
             unsigned char color);

/* Disegna MM:SS con cifre 5x10 a blocchi; occupa 26 colonne e 10 righe
   a partire da (x, y). */
void big_time(unsigned char x, unsigned char y, unsigned char mm,
              unsigned char ss, unsigned char color);

#endif
