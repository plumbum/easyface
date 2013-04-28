//******************************************************************************
//*
//*     File: lcd.h
//*     declartions
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

#ifndef _LCD_H_
#define _LCD_H_

#include <inttypes.h>

#define LCD_ROWS 4
#define LCD_COLS 20

/*
 * controller commands
 */

#define LCD_CLEAR               0x01

#define LCD_RETURN_HOME         0x02

#define LCD_SET_INCREMENT_MODE  0x06

#define LCD_SET_DM              0x08
#define LCD_DM_DISPLAY_ON       4
#define LCD_DM_DISPLAY_OFF      0
#define LCD_DM_CURSOR_ON        2
#define LCD_DM_CURSOR_OFF       0
#define LCD_DM_BLINK_ON         1
#define LCD_DM_BLINK_OFF        0

#define LCD_CMD_INIT4           0x28
#define LCD_CMD_INIT8           0x38

#define LCD_SET_DDRAM_ADDRESS   0x80

/**
 * LCD initialization routine
 */
void lcdInit(void);

/**
 * Send direct command to LCD
 */
void lcdCmd(unsigned char cmd);

/**
 * Print char to display
 */
void lcdData(unsigned char cmd);

/**
 * Move cursor
 * @param row 
 * @param col 
 */
void lcdGoto(unsigned char row, unsigned char col);

/**
 * Cursor mode
 * @param cursor uint8_t : 0 - off, 1 - on, 2 - blink
 */
void lcdCursor(char cursor);

/**
 * Put char to terminal
 * @param c - one character to print
 */
void lcdPutc(char c);

/**
 * Put string to terminal
 * @param ASCIIZ string to print
 */
void lcdPuts(const char* str);
void lcdPuts_P(const char* str);

void lcdPutArr(const char* str, unsigned char len);
void lcdPutWords(const uint16_t* words, uint8_t len);

/*
 * Fake functions ;-)
 */
#define lcdClr() lcdCmd(LCD_CLEAR)
#define lcdHome() lcdCmd(LCD_RETURN_HOME)

/**
 * Alias for lcdData
 */
#define lcdPut(c) lcdData(c)

#define lcdBackLight(on) { if(on) PIN_SET(LIGHT); else PIN_CLR(LIGHT); }


#endif /* _LCD_H_ */

