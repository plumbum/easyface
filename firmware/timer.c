#include "timer.h"

#include <avr/interrupt.h>

uint32_t timer;

ISR(TIMER2_COMPA_vect)
{
    timer++;
}

void timerInit(void)
{
    timer = 0;

    OCR2A = F_CPU/1000/64; //125;
    TCCR2A = (2<<WGM00);
    TCCR2B = (0<<WGM02) | (3<<CS00);
    TIMSK2 |= (1<<OCIE2A);
}
