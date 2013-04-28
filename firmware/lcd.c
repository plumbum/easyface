//******************************************************************************
//*
//*     File: lcd.c
//*     implementation
//*
//*     4-bit, 6 line access to lcd controller HD44780
//*     Support 2 and 4 line display
//*
//*
//*     Platform: AVR and other
//*
//*     Version: 0.1b
//*
//*     Copyright (c) 2009, Ivan A-R <ivan@tuxotronic.org>
//*
//*     Permission is hereby granted, free of charge, to any person 
//*     obtaining  a copy of this software and associated documentation 
//*     files (the "Software"), to deal in the Software without restriction, 
//*     including without limitation the rights to use, copy, modify, merge, 
//*     publish, distribute, sublicense, and/or sell copies of the Software, 
//*     and to permit persons to whom the Software is furnished to do so, 
//*     subject to the following conditions:
//*
//*     The above copyright notice and this permission notice shall be included 
//*     in all copies or substantial portions of the Software.
//*
//*     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
//*     EXPRESS  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
//*     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
//*     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
//*     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
//*     TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
//*     THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//*
//******************************************************************************

#include "lcd.h"

#include "config.h"

/**
 * @block Hardware depend part
 */
#include <util/delay.h>
#include <avr/pgmspace.h>


#define E_DELAY 10

#define LINE0 0x00
#define LINE1 0x40
#define LINE2 0x14
#define LINE3 0x54

/*
 * Elementary hardware depend commands
 */

//#define CMD_E_DELAY() { volatile unsigned char i; for(i=0; i<E_DELAY; i++) ; }
#define CMD_SLEEP_MS(ms) _delay_ms(ms)

#define CMD_E_MARK()    PIN_SET(LCD_E)
#define CMD_E_RELEASE() PIN_CLR(LCD_E)

#define CMD_RS0()       PIN_CLR(LCD_RS)
#define CMD_RS1()       PIN_SET(LCD_RS)

#define CMD_WRITE()     PIN_CLR(LCD_WR)
#define CMD_READ()      PIN_SET(LCD_WR)

static void lcdPrepareWrite(void)
{
    CMD_WRITE();
    PIN_OUT(LCD_D0);
    PIN_OUT(LCD_D1);
    PIN_OUT(LCD_D2);
    PIN_OUT(LCD_D3);
    PIN_OUT(LCD_D4);
    PIN_OUT(LCD_D5);
    PIN_OUT(LCD_D6);
    PIN_OUT(LCD_D7);
}

static void lcdPrepareRead(void)
{
    PIN_IN(LCD_D0);
    PIN_IN(LCD_D1);
    PIN_IN(LCD_D2);
    PIN_IN(LCD_D3);
    PIN_IN(LCD_D4);
    PIN_IN(LCD_D5);
    PIN_IN(LCD_D6);
    PIN_IN(LCD_D7);
    CMD_READ();
}

static void lcdWriteData(uint8_t x)
{
    if(x & (1<<0)) PIN_SET(LCD_D0); else PIN_CLR(LCD_D0);
    if(x & (1<<1)) PIN_SET(LCD_D1); else PIN_CLR(LCD_D1);
    if(x & (1<<2)) PIN_SET(LCD_D2); else PIN_CLR(LCD_D2);
    if(x & (1<<3)) PIN_SET(LCD_D3); else PIN_CLR(LCD_D3);
    if(x & (1<<4)) PIN_SET(LCD_D4); else PIN_CLR(LCD_D4);
    if(x & (1<<5)) PIN_SET(LCD_D5); else PIN_CLR(LCD_D5);
    if(x & (1<<6)) PIN_SET(LCD_D6); else PIN_CLR(LCD_D6);
    if(x & (1<<7)) PIN_SET(LCD_D7); else PIN_CLR(LCD_D7);
}

/*
static uint8_t lcdReadData(void)
{
    uint8_t r = 0;
    if(PIN(LCD_D0)) r |= (1<<0);
    if(PIN(LCD_D1)) r |= (1<<1);
    if(PIN(LCD_D2)) r |= (1<<2);
    if(PIN(LCD_D3)) r |= (1<<3);
    if(PIN(LCD_D4)) r |= (1<<4);
    if(PIN(LCD_D5)) r |= (1<<5);
    if(PIN(LCD_D6)) r |= (1<<6);
    if(PIN(LCD_D7)) r |= (1<<7);
    return r;
}
*/

#define CMD_CLEAN { LCD_GPIO &= ~(LCD_RS | LCD_E | LCD_DMASK); }

#define CMD_CONF_GPIO { DDRB |= LCD_DMASK | LCD_RS | LCD_E; }

static inline void CMD_E_DELAY(void)
{
    asm volatile (" nop ");
    asm volatile (" nop ");
    asm volatile (" nop ");
    /*
    volatile unsigned char i;
    for(i=0; i<E_DELAY; i++) ;
    */
}

static inline void pulse_e(void)
{
    CMD_E_MARK();
    CMD_E_DELAY();
    CMD_E_RELEASE();
    CMD_E_DELAY(); // ???
}

static void wait_ready(void)
{
    /*
    _delay_ms(1);
    return;
    */
    uint8_t flag;
    lcdPrepareRead();
    CMD_RS0();
    do
    {
        CMD_E_MARK();
        CMD_E_DELAY();
        flag = PIN(LCD_D7);
        CMD_E_RELEASE();
        CMD_E_DELAY();
        wdr();
    } while(flag);
    lcdPrepareWrite();
    /*
    uint16_t i;
    for(i=0; i<2000; i++) 
        asm volatile ("nop");
    */
}


static inline void _lcd_init(void)
{
}

void lcdPuts_P(const char* str)
{
    char c;
    while((c = pgm_read_byte(str++)) != 0)
        lcdPutc(c);
}

/**
 * @block hardware undepend part
 */


#ifdef WRAPEN
static uint8_t cur_col;
static uint8_t cur_row;
#endif

void lcdInit(void)
{
    _lcd_init();

    lcdPrepareWrite();
    CMD_SLEEP_MS(16); // 16 ms

    lcdWriteData(3); // 8bit mode
    pulse_e();
    CMD_SLEEP_MS(5); // 4992 us

    pulse_e(); // Repeat 8bit mode
    CMD_SLEEP_MS(1); // 64 us

    pulse_e(); // Third repeat
    CMD_SLEEP_MS(1); // 64 us

    // lcdWriteData(2); // Change to 4bit mode (0x20)
    // pulse_e();

    lcdCmd(LCD_CMD_INIT8);
    /*
    lcdCmd(0x24);
    lcdCmd(0x09);
    lcdCmd(0x20);
    */

    lcdCmd(0x08);
    lcdCmd(0x01);
    lcdCmd(0x06);
    lcdCmd(0x0C);

    lcdCmd(0x80);
    /*
    lcdCmd(0x02);
    */
    /*
    lcdCmd(LCD_SET_DM | LCD_DM_DISPLAY_ON);
    lcdCmd(LCD_SET_INCREMENT_MODE);
    */

#ifdef WRAPEN
    cur_col = 0;
    cur_row = 0;
#endif
}

void lcdCmd(unsigned char cmd)
{
    wait_ready();
    CMD_RS0(); // controll command
    lcdWriteData(cmd);
    pulse_e();
}

void lcdData(unsigned char cmd)
{
    wait_ready();
    CMD_RS1(); // data
    lcdWriteData(cmd);
    pulse_e();
}

void lcdCursor(char cursor)
{
    switch(cursor)
    {
        default:
            lcdCmd(LCD_SET_DM | LCD_DM_DISPLAY_ON);
            break;
        case 1:
            lcdCmd(LCD_SET_DM | LCD_DM_DISPLAY_ON | LCD_DM_CURSOR_ON);
            break;
        case 2:
            lcdCmd(LCD_SET_DM | LCD_DM_DISPLAY_ON | LCD_DM_CURSOR_ON | LCD_DM_BLINK_ON);
            break;
    }
}

void lcdGoto(unsigned char row, unsigned char col)
{
    char line_base;
    switch(row)
    {
        default:
            line_base = LINE0;
            break;
        case 1:
            line_base = LINE1;
            break;
#if (LCD_ROWS == 4)
        case 2:
            line_base = LINE2;
            break;
        case 3:
            line_base = LINE3;
            break;
#endif
    }
    lcdCmd((line_base + col) | LCD_SET_DDRAM_ADDRESS);
#ifdef WRAPEN
    cur_col = col;
    cur_row = row;
#endif
}

#ifdef WRAPEN
void lcdPutc(char c)
{
    if(c == '\n')
    {
        cur_col = 0;
        if(cur_row < LCD_ROWS-1)
            cur_row++;
        lcdGoto(cur_row, cur_col);
    }
    else
    {
        if(cur_col >= LCD_COLS)
        {
            cur_col = 0;
            if(cur_row < LCD_ROWS-1)
                cur_row++;
            lcdGoto(cur_row, cur_col);
        }
        lcdData(c);
        cur_col++;
    }
}

#else
void lcdPutc(char c)
{
    lcdData(c);
}
#endif

void lcdPutArr(const char* str, uint8_t len)
{
    char c;
    while(len--)
    {
        c = *str++;
        if(c) lcdPutc(c);
    }
}

void lcdPutWords(const uint16_t* words, uint8_t len)
{
    uint16_t w;
    char c;
    while(len--)
    {
        w = *words++;
        c = w>>8;
        if(c) lcdPutc(c);
        c = w & 0xFF;
        if(c) lcdPutc(c);
    }
}

void lcdPuts(const char* str)
{
    char c;
    while((c = *str++) != 0)
        lcdPutc(c);
}


