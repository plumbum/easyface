/*
 * =================================================================
 *
 *       Filename:  uart.c
 *
 *    Description:  UART routines, implementation
 *
 *        Version:  1.0
 *        Created:  03.11.2007 16:30:06 MSK
 *       Revision:  none
 *       Compiler:  avr-gcc
 *
 *         Author:  Ivan A-R <ivan@tuxotronic.org>
 *        License:  LGPL
 *
 * =================================================================
 */

#include "uart.h"

#include <inttypes.h>
#include <avr/interrupt.h>

#include "fifo.h"

#include "timer.h"

fifo_struct_t rx;
fifo_struct_t tx;

uart_flags_t uartFlags;

void uartInit(uint16_t baud)
{
    uartFlags.tx = 0;

    fifoInit(&rx);
    fifoInit(&tx);

    UCSR0B = 0;
    /* Set baud rate */
    UBRR0H = (unsigned char)(baud>>8);
    UBRR0L = (unsigned char)baud;

    UCSR0A = (1<<TXC0) | (1<<RXC0) | (1<<UDRE0);
    /* Set frame format: 8data, 2stopbit */
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);
    /* Enable receiver and transmit */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)/*|(1<<UDRIE0)|(1<<TXCIE0)*/;
}

/*
 * UART get routines
 */

ISR(USART_RX_vect)
{
    char data = UDR0;

    // FE0 - frame error
    // DOR0 - data overrun
    // UPE0 - parity
    if(!(UCSR0A & ((1<<FE0) | (1<<DOR0) | (1<<UPE0))))
    {
        fifoPush(&rx, data);
    }
}

char uartGetReady(void)
{
    return !fifoEmpty(&rx);
}

int16_t uartGetRx(void)
{
    int16_t data;
    while(fifoEmpty(&rx))
        wdr();
    cli();
    data = fifoPop(&rx);
    sei();
    return data;
}

int16_t uartGetRxTimeout(uint32_t timeout_ms)
{
    uint32_t timestampt = timer;
    int16_t data;
    while(fifoEmpty(&rx))
    {
        wdr();
        if((timer - timestampt) > timeout_ms)
            return UART_TIMEOUT;
    }
    cli();
    data = fifoPop(&rx);
    sei();
    return data;
}

int16_t uartGetTopRx(void)
{
    while(fifoEmpty(&rx))
        wdr();
    return fifoTop(&rx);
}


/*
 * UART put routines
 */
ISR(USART_UDRE_vect)
{
    if(!fifoEmpty(&tx))
    {
        UDR0 = fifoPop(&tx);
    } else {
        UCSR0B &= ~(1<<UDRIE0);
        UCSR0B |= (1<<TXCIE0);
    }
}

ISR(USART_TX_vect)
{
    if(fifoEmpty(&tx))
    {
        PIN_CLR(DE);
        uartFlags.tx = 0;
    }
    UCSR0B &= ~(1<<TXCIE0);
}

char uartTxFull(void)
{
    return fifoFull(&tx);
}

void uartPutTx(uint8_t data)
{
    if(fifoEmpty(&tx) && (UCSR0A & (1<<UDRE0)))
    { // If can - write direct to USART
        uartFlags.tx = 1;
        PIN_SET(DE);
        UDR0 = data;
        UCSR0B |= (1<<UDRIE0);
    }
    else
    {
        while(fifoFull(&tx))
            wdr(); // Wait for space in buffer
        fifoPush(&tx, data);
    }
}

void uartPuts(char* str)
{
    char c;
    while((c = *str++) != 0)
        uartPutTx((uint8_t)c);
}

