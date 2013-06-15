/*
 * ===================================================================
 *
 *       Filename:  fifo.h
 *
 *    Description:  simple fifo buffer, headers
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

#ifndef _FIFO_H_
#define _FIFO_H_

#include <inttypes.h>

/**
 * FIFO_SIZE must be power of two
 */
#ifndef FIFO_SIZE
#define FIFO_SIZE 64
#endif

typedef enum {fifoFALSE=0, fifoTRUE=!fifoFALSE} fifo_bool_t;

typedef uint8_t fifo_idx_t;
typedef uint8_t fifo_data_t;

typedef struct {
    volatile fifo_idx_t head;
    volatile fifo_idx_t tail;
    volatile fifo_bool_t empty:1;
    volatile fifo_bool_t full:1;
    fifo_data_t buf[FIFO_SIZE];
} fifo_struct_t;

#define FIFO_MASK (FIFO_SIZE-1)

#if (FIFO_SIZE & FIFO_MASK) != 0
#error FIFO_SIZE must be power of two
#endif

inline static void fifoInit(fifo_struct_t* fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
    fifo->empty = fifoTRUE;
    fifo->full = fifoFALSE;
}

inline static fifo_bool_t fifoEmpty(fifo_struct_t* fifo)
{
    return fifo->empty;
}

inline static fifo_bool_t fifoFull(fifo_struct_t* fifo)
{
    return fifo->full;
}

inline static fifo_idx_t fifoFree(fifo_struct_t* fifo)
{
    return (fifo->tail - fifo->head) & FIFO_MASK;
}

inline static fifo_idx_t fifoUsed(fifo_struct_t* fifo)
{
    return (fifo->head - fifo->tail) & FIFO_MASK;
}


fifo_bool_t fifoPush(fifo_struct_t* fifo, fifo_data_t value);
fifo_data_t fifoPop(fifo_struct_t* fifo);

fifo_data_t fifoTop(fifo_struct_t* fifo);


#endif /* _FIFO_H_ */

// vim: sw=4:ts=4:si:et

