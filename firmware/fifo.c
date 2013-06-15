/*
 * ===================================================================
 *
 *       Filename:  fifo.h
 *
 *    Description:  fifo buffer, headers
 *
 *        Version:  1.0
 *        Created:  Mon, 02 Nov 2009 13:59:42 +0300
 *       Revision:  none
 *       Compiler:  avr-gcc
 *
 *         Author:  Ivan A-R <ivan@tuxotronic.org>
 *        License:  LGPL
 *
 * ===================================================================
 */

#include "fifo.h"


fifo_bool_t fifoPush(fifo_struct_t* fifo, fifo_data_t value)
{
    if(fifoFull(fifo))
        return fifoTRUE;
    else
    {
        fifo->buf[fifo->head] = value;
        fifo->head = (fifo->head+1) & FIFO_MASK;
        fifo->empty = fifoFALSE;
        if(fifo->head == fifo->tail)
            fifo->full = fifoTRUE;
        return fifoFALSE;
    }
}

fifo_data_t fifoPop(fifo_struct_t* fifo)
{
    if(fifoEmpty(fifo))
        return 0;
    else
    {
        char value = fifo->buf[fifo->tail];
        fifo->tail = (fifo->tail+1) & FIFO_MASK;
        fifo->full = fifoFALSE;
        if(fifo->tail == fifo->head)
            fifo->empty = fifoTRUE;
        return value;
    }
}

fifo_data_t fifoTop(fifo_struct_t* fifo)
{
    if(fifoEmpty(fifo))
        return 0;
    else
        return fifo->buf[fifo->tail];
}

// vim: sw=4:ts=4:si:et

