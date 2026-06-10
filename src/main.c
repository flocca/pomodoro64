/*
 * pomodoro64 - Pomodoro timer per Commodore 64
 *
 * Macchina a stati: menu -> (lavoro -> pausa)* -> menu
 * Ogni 4 pomodori completati la pausa e' lunga.
 *
 * Tempo:  TOD del CIA #1 (tod.c) - hardware, non deriva
 * Video:  cifre giganti PETSCII in modalita' testo (bigfont.c)
 * Audio:  beep e jingle sulla voce 1 del SID (sound.c)
 *
 * Nota sulle stringhe: tutte minuscole. cc65 le converte in PETSCII
 * e nel charset maiuscolo di default appaiono come maiuscole; ASCII
 * maiuscolo diventerebbe invece simboli grafici.
 */

#include <conio.h>
#include <c64.h>
#include "tod.h"
#include "bigfont.h"
#include "sound.h"

/* fasi */
#define PH_WORK  0
#define PH_SHORT 1
#define PH_LONG  2

/* esito di una fase */
#define RES_DONE  0
#define RES_SKIP  1
#define RES_RESET 2

#define BAR_W         36
#define POMOS_PER_SET 4

/* durate correnti in minuti, indice = fase */
static unsigned char minutes[3] = {25, 5, 15};

static const unsigned char work_presets[]  = {15, 20, 25, 30, 45, 50};
static const unsigned char short_presets[] = {3, 5, 10, 15};
static const unsigned char long_presets[]  = {10, 15, 20, 30};
static const unsigned char *const presets[3] = {
    work_presets, short_presets, long_presets
};
static const unsigned char preset_len[3] = {6, 4, 4};
static unsigned char preset_idx[3] = {2, 1, 1};

static const char *const phase_name[3] = {
    "lavoro", "pausa breve", "pausa lunga"
};
static const unsigned char phase_fg[3] = {
    COLOR_WHITE, COLOR_LIGHTGREEN, COLOR_CYAN
};
static const unsigned char phase_bd[3] = {
    COLOR_RED, COLOR_GREEN, COLOR_BLUE
};

static unsigned char done_count;   /* pomodori completati nella sessione */

/* ---------------------------------------------------------------- UI -- */

static void center(unsigned char y, const char *s)
{
    unsigned char len = 0;
    const char *p = s;

    while (*p++) {
        ++len;
    }
    cputsxy((40 - len) / 2, y, s);
}

static void title_bar(void)
{
    unsigned char i;

    revers(1);
    textcolor(COLOR_WHITE);
    gotoxy(0, 0);
    for (i = 0; i < 40; ++i) {
        cputc(' ');
    }
    cputsxy(14, 0, "pomodoro 64");
    revers(0);
}

static void draw_pomodoros(unsigned char filled)
{
    unsigned char i;

    for (i = 0; i < POMOS_PER_SET; ++i) {
        /* screen code 81/87 = cerchio pieno/vuoto */
        big_put(15 + i * 3, 17,
                (i < filled) ? 81 : 87,
                (i < filled) ? COLOR_RED : COLOR_GRAY1);
    }
}

static void draw_bar(unsigned char filled, unsigned char color)
{
    unsigned char i;

    textcolor(color);
    gotoxy(1, 15);
    cputc('[');
    for (i = 0; i < BAR_W; ++i) {
        if (i < filled) {
            revers(1);
            cputc(' ');
        } else {
            revers(0);
            cputc('.');
        }
    }
    revers(0);
    cputc(']');
}

/* -------------------------------------------------------------- menu -- */

static void cycle_preset(unsigned char ph)
{
    preset_idx[ph] = (preset_idx[ph] + 1) % preset_len[ph];
    minutes[ph] = presets[ph][preset_idx[ph]];
}

static void menu_values(void)
{
    textcolor(COLOR_WHITE);
    gotoxy(7, 8);
    cprintf("f1  lavoro       %2d min", minutes[PH_WORK]);
    gotoxy(7, 11);
    cprintf("f3  pausa breve  %2d min", minutes[PH_SHORT]);
    gotoxy(7, 14);
    cprintf("f5  pausa lunga  %2d min", minutes[PH_LONG]);
}

static void show_menu(void)
{
    bordercolor(COLOR_GRAY1);
    bgcolor(COLOR_BLACK);
    clrscr();
    cursor(0);
    title_bar();
    textcolor(COLOR_ORANGE);
    center(4, "un timer pomodoro per c64");
    menu_values();
    textcolor(COLOR_GRAY2);
    center(19, "f1/f3/f5 cambiano le durate");
    center(21, "spazio per iniziare");

    for (;;) {
        switch (cgetc()) {
        case CH_F1: cycle_preset(PH_WORK);  break;
        case CH_F3: cycle_preset(PH_SHORT); break;
        case CH_F5: cycle_preset(PH_LONG);  break;
        case ' ':
            sound_beep();
            return;
        default:
            continue;
        }
        sound_beep();
        menu_values();
    }
}

/* -------------------------------------------------------------- fasi -- */

static unsigned char run_phase(unsigned char ph)
{
    unsigned total = minutes[ph] * 60U;
    unsigned el;
    unsigned last = 0xFFFFU;
    unsigned rem;
    unsigned char paused = 0;
    unsigned char filled = done_count % POMOS_PER_SET;

    /* durante la pausa lunga il set appena chiuso resta pieno */
    if (ph == PH_LONG && filled == 0 && done_count > 0) {
        filled = POMOS_PER_SET;
    }

    bordercolor(phase_bd[ph]);
    bgcolor(COLOR_BLACK);
    clrscr();
    title_bar();
    textcolor(phase_fg[ph]);
    center(2, phase_name[ph]);
    draw_pomodoros(filled);
    textcolor(COLOR_GRAY2);
    gotoxy(8, 19);
    cprintf("pomodori completati: %d", done_count);
    center(23, "spazio=pausa  s=salta  r=menu");

    /* stato iniziale disegnato subito: se una pausa scatta al primo giro
       di loop il timer deve essere gia' visibile */
    big_time(7, 4, minutes[ph], 0, phase_fg[ph]);
    draw_bar(0, phase_fg[ph]);

    tod_reset();
    for (;;) {
        if (kbhit()) {
            switch (cgetc()) {
            case ' ':
                sound_beep();
                if (paused) {
                    tod_resume();
                    paused = 0;
                    cclearxy(0, 21, 40);
                } else {
                    tod_pause();
                    paused = 1;
                    textcolor(COLOR_YELLOW);
                    center(21, "- in pausa -");
                }
                break;
            case 's':
                sound_beep();
                return RES_SKIP;
            case 'r':
                sound_beep();
                return RES_RESET;
            }
        }
        if (paused) {
            continue;
        }

        el = tod_seconds();
        if (el != last) {
            last = el;
            rem = (el >= total) ? 0 : total - el;
            big_time(7, 4, rem / 60, rem % 60, phase_fg[ph]);
            draw_bar((unsigned char)((unsigned long)el * BAR_W / total),
                     phase_fg[ph]);
        }
        if (el >= total) {
            return RES_DONE;
        }
    }
}

static unsigned char wait_continue(unsigned char next_ph)
{
    textcolor(COLOR_YELLOW);
    center(21, "fase completata! spazio per continuare");
    if (next_ph == PH_WORK) {
        sound_jingle_to_work();
    } else {
        sound_jingle_to_break();
    }
    for (;;) {
        switch (cgetc()) {
        case ' ': return RES_DONE;
        case 'r': return RES_RESET;
        }
    }
}

/* -------------------------------------------------------------- main -- */

int main(void)
{
    unsigned char ph;
    unsigned char res;

    /* charset maiuscolo/grafica ($D018): i cerchi PETSCII dei pomodori
       (screen code $51/$57) esistono solo qui; nel charset minuscolo
       sarebbero le lettere q/w */
    VIC.addr = 0x15;

    sound_init();
    tod_init();

    for (;;) {
        done_count = 0;
        show_menu();
        ph = PH_WORK;
        for (;;) {
            res = run_phase(ph);
            if (res == RES_RESET) {
                break;
            }
            if (ph == PH_WORK) {
                if (res == RES_DONE) {
                    ++done_count;
                }
                ph = (res == RES_DONE && done_count % POMOS_PER_SET == 0)
                     ? PH_LONG : PH_SHORT;
            } else {
                ph = PH_WORK;
            }
            if (res == RES_DONE && wait_continue(ph) == RES_RESET) {
                break;
            }
        }
    }
    return 0;
}
