/*
 * ===================================================================
 *
 *       Filename:  helpers.c
 *
 *    Description:  some functions, implements
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

#ifndef _HELPERS_H_
#define _HELPERS_H_

#include "config.h"

void delay_ms(unsigned int ms);
void ERROR(uint8_t no, uint8_t code);

#endif /* _HELPERS_H_ */


