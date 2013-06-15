#include "systimer.h"

#include <avr/interrupt.h>

#define TIMER_CLOCK 1000
#define TIMER_DIVIDER 64
#define TIMER_COEF (F_CPU/TIMER_DIVIDER/TIMER_CLOCK)

#if (TIMER_COEF > 255)
#error Overflow TIMER_DIVIDER
#endif

#if (TIMER_DIVIDER == 1)
#define TIMER_CS 1
#elif (TIMER_DIVIDER == 8)
#define TIMER_CS 2
#elif (TIMER_DIVIDER == 32)
#define TIMER_CS 3
#elif (TIMER_DIVIDER == 64)
#define TIMER_CS 4
#elif (TIMER_DIVIDER == 128)
#define TIMER_CS 5
#elif (TIMER_DIVIDER == 256)
#define TIMER_CS 6
#elif (TIMER_DIVIDER == 1024)
#define TIMER_CS 7
#else
#error Invalid TIMER_DIVIDER
#endif

volatile systimer_t systimer;

ISR(TIMER2_COMPA_vect)
{
    systimer++;
    kbd_systimer();
}

void timerInit(void)
{
    systimer = 0;

    OCR2A = TIMER_COEF;
    TCCR2A = (2<<WGM00);
    TCCR2B = (0<<WGM02) | (TIMER_CS<<CS00);
    TIMSK2 |= (1<<OCIE2A);
}

// vim: sw=4:ts=4:si:et

