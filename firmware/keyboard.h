#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "config.h"
#include "fifo.h"

extern fifo_struct_t kbd_scan;

void kbdInit(void);

void kbdProcess(void);

void kbdSetMask(uint8_t mask);

#endif /* _KEYBOARD_H_ */

// vim: sw=4:ts=4:si:et

