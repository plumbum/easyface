#ifndef _SYSTIMER_H_
#define _SYSTIMER_H_

#include "config.h"

typedef uint16_t systimer_t;
extern volatile systimer_t systimer;

void timerInit(void);

void kbd_systimer(void);

#endif /* _SYSTIMER_H_ */

// vim: sw=4:ts=4:si:et

