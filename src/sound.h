#ifndef SOUND_H
#define SOUND_H

void sound_init(void);             /* azzera il SID e alza il volume */
void sound_beep(void);             /* click di conferma tasto */
void sound_jingle_to_break(void);  /* arpeggio: fine lavoro, si va in pausa */
void sound_jingle_to_work(void);   /* due note: fine pausa, si torna al lavoro */

#endif
