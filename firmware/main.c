/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  Terminal bridge
 *
 *        Version:  1.0
 *        Created:  Fri Mar 14 17:43:49 MSK 2008
 *       Revision:  $Id$
 *       Compiler:  gcc
 *
 *         Author:  Ivan A-R <ivan@alferov.name>
 *
 *        License:  private
 *
 * =====================================================================================
 */

#include "config.h"

#include <string.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "helpers.h"
#include "systimer.h"
#include "uart.h"
#include "lcd.h"

int main(void)
{
    // uint8_t mcusr = MCUSR;
    // MCUSR = 0;

#ifdef WDT_EN
    wdt_disable();
    wdt_enable(WDTO_1S);
#endif

    SET_DDR();

    timerInit();

    lcdInit();
    lcdBackLight(1);
    lcdClr();
    lcdHome();
    lcdCursor(2);

    _delay_ms(100);
    wdr();
    _delay_ms(100);

    uartInit(UART19200);

    sei();

    uartPuts("Hello world!\r\n");
    uartPuts("Hello\r\n");

    int cnt = 0;
    char str[32];
    for(;;)
    {
        wdr();
        itoa(cnt++, str);
        uartPuts(str);
        uartPuts(" : ");
        itoa(systimer, str);
        uartPuts(str);
        uartPuts("\r\n");
        _delay_ms(500);
    }

    for(;;)
    {
        wdr();
        char c = uartGetRx();
        switch(c) {
            case '`':
                lcdHome();
                break;
            case '~':
                lcdClr();
                break;
            default:
                lcdPutc(c);
                break;
        }
    }
    return 0;
}

// vim: sw=4:ts=4:si:et
