#include "keyboard.h"
#include "egpio.h"

uint8_t kbd_scancode;

static uint8_t _kbd_mask;
static uint8_t _kbd_index;

void kbd_systimer(void)
{
}

void kbdInit(void)
{
    _kbd_mask = 0x10;
    _kbd_index = 0;
    kbd_scancode = 0;
    egpioWriteReg(3, 0x0F);
    egpioWriteReg(1, ~_kbd_mask);
}

/*
static uint8_t nextMask(void)
{
    if(kbd_mask == 0) return 0;
    do
    {
        _kbd_mask <<= 1;
        _kbd_index++;
        if(_kbd_mask == 0)
        {
            _kbd_mask = 1;
            _kbd_index = 0;
        }
    }
    while((kbd_mask & _kbd_mask) == 0);
    return _kbd_mask;
}

void kbdSetMask(uint8_t mask)
{
    uint8_t i;
    for(i=0; i<8; i++) kbd_buffer[i] = 0;
    kbd_mask = mask;
    _kbd_mask = 0;
    egpioWriteReg(1, 0xFF);
    egpioWriteReg(3, ~mask);
    nextMask();
    egpioWriteReg(1, ~_kbd_mask);
}
*/

void kbdProcess(void)
{
    uint8_t tmp;
    uint8_t scan;
    // egpioReadReg(0, &tmp);
    tmp = ~tmp & 0x0F;

    _kbd_index++;
    _kbd_mask <<= 1;
    if(_kbd_mask == 0)
    {
        _kbd_mask = 0x10;
        _kbd_index = 0;
        kbd_scancode = scan;
        scan = 0;
    }

    egpioWriteReg(1, ~_kbd_mask);

    if(tmp & 0x01)
        scan = 0x80 | _kbd_index;
    else if(tmp & 0x02)
        scan = 0x84 | _kbd_index;
    else if(tmp & 0x04)
        scan = 0x88 | _kbd_index;
    else if(tmp & 0x08)
        scan = 0x8C | _kbd_index;
}

