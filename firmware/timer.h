#ifndef _TIMER_H_
#define _TIMER_H_

#include "config.h"

typedef uint16_t systimer_t;
extern volatile systimer_t systimer;

void timerInit(void);

#endif /* _TIMER_H_ */

