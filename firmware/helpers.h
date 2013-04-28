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

uint16_t fcrc16i(uint16_t crc16, uint8_t data);
uint16_t fcrc16(const uint8_t *buffer, uint16_t buffer_length);

char* uitoa(uint32_t val, char* dest);
char* itoa(int32_t val, char* dest);
char* uitox(uint32_t val, char* dest, int digits);
char* uctox(unsigned char val, char* dest);
int numericLength(int32_t v);

#endif /* _HELPERS_H_ */


