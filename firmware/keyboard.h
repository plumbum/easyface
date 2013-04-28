#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "config.h"

extern uint8_t kbd_scancode;

void kbdInit(void);

void kbdProcess(void);

void kbdSetMask(uint8_t mask);

#endif /* _KEYBOARD_H_ */

