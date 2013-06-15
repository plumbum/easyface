/*
 * =====================================================================================
 * 
 *       Filename:  uart.h
 * 
 *    Description:  UART routines, declartions
 * 
 *        Version:  1.0
 *        Created:  03.11.2007 16:35:24 MSK
 *       Revision:  none
 *       Compiler:  avr-gcc
 * 
 *         Author:  Ivan A-R <ivan@tuxotronic.org>
 *        License:  LGPL
 * 
 * =====================================================================================
 */

#ifndef  _UART_H_
#define  _UART_H_

#include "config.h"

#define UART_OK         (0)
#define UART_TIMEOUT    (-1)

// UART constant
#define UARTN(x) (uint16_t)((F_CPU)/((x)*16l)-1)

#define UART1200 UARTN(1200)
#define UART2400 UARTN(2400)
#define UART4800 UARTN(4800)
#define UART9600 UARTN(9600)
#define UART19200 UARTN(19200)
#define UART38400 UARTN(38400)
#define UART57600 UARTN(57600)
#define UART115200 UARTN(115200)
#define UART230400 UARTN(230400)
#define UART460800 UARTN(460800)

typedef struct uart_flags_s {
    volatile uint8_t tx:1;
} uart_flags_t;

extern uart_flags_t uartFlags;

// Functions
#define uartSTATE ( UCSRA & (1<<RXC) )

void uartInit(uint16_t baud);

char uartGetReady(void);
int16_t uartGetRx(void);
int16_t uartGetTopRx(void);

int16_t uartGetRxTimeout(uint16_t timeout_ms);

char uartTxFull(void);
void uartPutTx(uint8_t data);
#define uartPut(data) uartPutTx(data)

void uartPuts(char* str);

#endif   /* ----- #ifndef _UART_H_  ----- */

// vim: sw=4:ts=4:si:et

