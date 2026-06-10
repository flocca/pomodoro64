#ifndef TOD_H
#define TOD_H

void tod_init(void);          /* configura 50/60Hz in base a PAL/NTSC */
void tod_reset(void);         /* azzera e (ri)avvia il cronometro */
void tod_pause(void);         /* congela il clock */
void tod_resume(void);        /* lo fa ripartire da dov'era */
unsigned tod_seconds(void);   /* secondi trascorsi dall'ultimo reset */

#endif
