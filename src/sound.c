/*
 * Suoni sul SID ($D400), voce 1, scrivendo direttamente i registri
 * tramite la struct SID di c64.h.
 *
 * Le note sono bloccanti (busy wait sul raster): vanno suonate solo
 * nelle transizioni di stato, mai dentro il loop del timer.
 */

#include <c64.h>
#include "sound.h"

/* valore registro = Hz * 16777216 / 985248 (clock PAL);
   su NTSC l'intonazione sale del ~4%, irrilevante per dei beep */
#define NOTE_C5  8911U
#define NOTE_E5 11228U
#define NOTE_G5 13351U
#define NOTE_C6 17821U

static void wait_frames(unsigned char n)
{
    while (n--) {
        while (VIC.rasterline >= 128) ;
        while (VIC.rasterline < 128) ;
    }
}

void sound_init(void)
{
    unsigned char i;

    for (i = 0; i < 25; ++i) {
        ((unsigned char *)0xD400)[i] = 0;
    }
    SID.amp = 0x0F;            /* volume al massimo, filtro spento */
}

static void note(unsigned freq, unsigned char frames)
{
    SID.v1.freq = freq;
    SID.v1.pw   = 0x0800;      /* onda quadra, duty 50% */
    SID.v1.ad   = 0x08;        /* attack 0, decay medio */
    SID.v1.sr   = 0xA0;        /* sustain ~2/3, release 0 */
    SID.v1.ctrl = 0x41;        /* pulse + gate on */
    wait_frames(frames);
    SID.v1.ctrl = 0x40;        /* gate off */
    wait_frames(2);
}

void sound_beep(void)
{
    note(NOTE_C6, 2);
}

void sound_jingle_to_break(void)
{
    note(NOTE_C5, 8);
    note(NOTE_E5, 8);
    note(NOTE_G5, 8);
    note(NOTE_C6, 16);
}

void sound_jingle_to_work(void)
{
    note(NOTE_G5, 8);
    note(NOTE_C6, 16);
}
