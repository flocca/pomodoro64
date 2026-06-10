/*
 * Cronometro basato sul Time-of-Day clock del CIA #1 ($DC08-$DC0B).
 * Il TOD e' agganciato ai 50/60Hz: non deriva e non dipende dagli IRQ.
 *
 * Particolarita' hardware sfruttate qui:
 *  - scrivere il registro delle ore FERMA il clock; scrivere i decimi
 *    lo riavvia (e' cosi' che implementiamo pausa/riprendi)
 *  - leggere le ore congela (latch) i registri di lettura; leggere i
 *    decimi li sblocca: l'ordine di lettura e' sempre ore -> decimi
 *  - i valori sono in BCD
 */

#include <c64.h>
#include "tod.h"

static unsigned char saved_tenths;

static unsigned char bcd2bin(unsigned char b)
{
    return (b >> 4) * 10 + (b & 0x0F);
}

/* PAL ha 312 linee raster, NTSC al massimo 263: se entro qualche frame
   il contatore a 9 bit (bit 8 in ctrl1, bit 7) supera 271 siamo su PAL */
static unsigned char is_pal(void)
{
    unsigned i;

    for (i = 0; i < 8000U; ++i) {
        if ((VIC.ctrl1 & 0x80) && VIC.rasterline >= 0x10) {
            return 1;
        }
    }
    return 0;
}

void tod_init(void)
{
    /* TODIN (bit 7 di CRA): 1 = rete a 50Hz (PAL), 0 = 60Hz (NTSC) */
    if (is_pal()) {
        CIA1.cra |= 0x80;
    } else {
        CIA1.cra &= 0x7F;
    }
    /* bit 7 di CRB a 0: le scritture impostano il clock, non la sveglia */
    CIA1.crb &= 0x7F;
}

void tod_reset(void)
{
    CIA1.tod_hour = 0;   /* ferma il clock */
    CIA1.tod_min  = 0;
    CIA1.tod_sec  = 0;
    CIA1.tod_10   = 0;   /* lo riavvia */
}

void tod_pause(void)
{
    unsigned char h = CIA1.tod_hour;   /* latch */
    unsigned char m = CIA1.tod_min;
    unsigned char s = CIA1.tod_sec;

    saved_tenths = CIA1.tod_10;        /* sblocca il latch */
    CIA1.tod_hour = h;                 /* riscrivere le ore ferma il clock */
    CIA1.tod_min  = m;
    CIA1.tod_sec  = s;
}

void tod_resume(void)
{
    CIA1.tod_10 = saved_tenths;        /* scrivere i decimi riavvia il clock */
}

unsigned tod_seconds(void)
{
    unsigned char h = CIA1.tod_hour;   /* latch */
    unsigned char m = CIA1.tod_min;
    unsigned char s = CIA1.tod_sec;
    unsigned char t = CIA1.tod_10;     /* sblocca il latch */

    (void)t;
    h = bcd2bin(h & 0x1F);             /* via il flag AM/PM */
    return h * 3600U + bcd2bin(m) * 60U + bcd2bin(s);
}
