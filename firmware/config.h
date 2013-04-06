/*
 * =====================================================================================
 * 
 *       Filename:  config.h
 * 
 *    Description:  global configuration file
 * 
 *        Version:  1.0
 *        Created:  Fri Mar 14 17:46:40 MSK 2008
 *       Revision:  $Id$
 *       Compiler:  avr-gcc
 * 
 *         Author:  Ivan A-R <ivan@alferov.name>
 * 
 * =====================================================================================
 */

#ifndef  _CONFIG_H_
#define  _CONFIG_H_

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>

#define WDT_EN 1

#define WRAPEN 1

#define PORT_SET(port, bits) { (port) |=  (bits); }
#define PORT_CLR(port, bits) { (port) &= ~(bits); }

#define _PIN(port, bit) ((port) & (1<<(bit)))
#define PIN(x) _PIN(x)

#define _PIN_SET(port, bit) do { *(volatile uint8_t*)((&port)+2) |=  (1<<(bit)); } while(0)
#define PIN_SET(x) _PIN_SET(x)
#define _PIN_CLR(port, bit) do { *(volatile uint8_t*)((&port)+2) &= ~(1<<(bit)); } while(0)
#define PIN_CLR(x) _PIN_CLR(x)

#define _PIN_OUT(port, bit) do { *(volatile uint8_t*)((&port)+1) |=  (1<<(bit)); } while(0)
#define PIN_OUT(x) _PIN_OUT(x)
#define _PIN_IN(port, bit) do { *(volatile uint8_t*)((&port)+1) &= ~(1<<(bit)); } while(0)
#define PIN_IN(x) _PIN_IN(x)



#define _VAR_PIN_SET(port, bit) do { _##port[1] |=  (1<<(bit)); } while(0)
#define VAR_PIN_SET(x) _VAR_PIN_SET(x)

#define _VAR_PIN_CLR(port, bit) do { _##port[1] &= ~(1<<(bit)); } while(0)
#define VAR_PIN_CLR(x) _VAR_PIN_CLR(x)

#define _VAR_PIN_OUT(port, bit) do { _##port[0] |=  (1<<(bit)); } while(0)
#define VAR_PIN_OUT(x) _VAR_PIN_OUT(x)

#define _VAR_PIN_IN(port, bit)  do { _##port[0] &= ~(1<<(bit)); } while(0)
#define VAR_PIN_IN(x) _VAR_PIN_IN(x)

/*
 * Pinout configuration
 */

#define BUZZER  PIND, PD2

#define RXD     PIND, PD0
#define TXD     PIND, PD1
#define DE      PIND, PD2

#define LCD_RS      PINC, PC0
#define LCD_E       PINB, PB5
#define LCD_WR      PIND, PD4

#define LCD_D0      PINB, PB4
#define LCD_D1      PINB, PB3
#define LCD_D2      PINB, PB2
#define LCD_D3      PINB, PB1
#define LCD_D4      PINB, PB0
#define LCD_D5      PIND, PD7
#define LCD_D6      PIND, PD6
#define LCD_D7      PIND, PD5

#define LIGHT       PINC, PC3


inline static void SET_DDR(void)
{
    PORTB = 0xFF;
    DDRB  = 0;
    PORTC = 0xFF;
    DDRC  = 0;
    PORTD = 0xFF;
    DDRD  = 0;

    PIN_CLR(DE);
    PIN_OUT(DE);
    PIN_CLR(LIGHT);
    PIN_OUT(LIGHT);
    PIN_CLR(LCD_E);
    PIN_OUT(LCD_E);
    PIN_OUT(LCD_RS);
    PIN_OUT(LCD_WR);
    /*
    uint8_t _PINB[2] = {0, 0xFF};
    uint8_t _PINC[2] = {0, 0xFF};
    uint8_t _PIND[2] = {0, 0xFF};

    VAR_PIN_CLR(DE);
    VAR_PIN_OUT(DE);
    VAR_PIN_CLR(LIGHT);
    VAR_PIN_OUT(LIGHT);
    VAR_PIN_CLR(LCD_E);
    VAR_PIN_OUT(LCD_E);
    VAR_PIN_OUT(LCD_RS);
    VAR_PIN_OUT(LCD_WR);

    PORTB = _PINB[0];
    DDRB  = _PINB[1];
    PORTC = _PINC[0];
    DDRC  = _PINC[1];
    PORTD = _PIND[0];
    DDRD  = _PIND[1];
    */
}

#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	// cpu cycles per microsecond

#define _B(x) (1<<(x))

#define _SB(x) |= _B(x)
#define _RB(x) &= ~(_B(x))

#define _SET(x, val, msk) (x) = (val | ((x) & ~(msk)))

#define HI(x) (uint8_t)((x)>>8)
#define LO(x) (uint8_t)((x) & 0xFF)

#define NOP __asm__ __volatile__ (" nop ")

#ifdef WDT_EN
#   define wdr() wdt_reset()
#else
#   define wdr()
#endif


#endif   /* ----- #ifndef _CONFIG_H_  ----- */

