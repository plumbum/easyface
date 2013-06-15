#include "keyboard.h"
#include "egpio.h"

/* Must be power of 2 */
#define KBD_SCAN_SIZE 4

typedef enum {
    ksNone = 0,
    ksSelectRow,
    ksWaitSelectRow,
    ksGetValue,
    ksWaitGetValue,
} kbd_state_t;

static kbd_state_t _kbd_state = ksNone;

static uint8_t _kbd_row_index;

static uint8_t _kbd_scan_prev[KBD_SCAN_SIZE];

fifo_struct_t kbd_scan;

void kbd_push_scancode(uint8_t scancode)
{
    fifoPush(&kbd_scan, scancode);
}

void kbd_systimer(void)
{
    twi_state_t st;
    switch(_kbd_state)
    {
        case ksSelectRow:
            egpioWriteReg(EGPIO_CONFIG, ~(1<<_kbd_row_index)); // Select row
            _kbd_state = ksWaitSelectRow;
            break;
        case ksWaitSelectRow:
            if((st = egpioCheckStatus()) != twiStNone)
            {
                _kbd_state = ksGetValue;
            }
            break;
        case ksGetValue:
            egpioReadReg(EGPIO_INPUT);
            _kbd_state = ksWaitGetValue;
            break;
        case ksWaitGetValue:
            if((st = egpioCheckStatus()) != twiStNone)
            {
                uint8_t now = (twi.reg_value>>4) | (((~PINC & 0x06)>>1)<<4);
                uint8_t change = _kbd_scan_prev[_kbd_row_index] ^ now;
                if(change) /* Have change keys */
                {
                    _kbd_scan_prev[_kbd_row_index] = now;
                    uint8_t i;
                    for(i=0; i<6; i++)
                    {
                        if(change & 1)
                        {
                            kbd_push_scancode(
                                ((now&1)?0:0x80) |
                                _kbd_row_index |
                                (i<<2)
                            );
                        }
                        change >>= 1;
                        now >>= 1;
                    }
                }

                _kbd_row_index++;
                if(_kbd_row_index >= KBD_SCAN_SIZE)
                { /* Process scancode */
                    _kbd_row_index = 0;
                }
                _kbd_state = ksSelectRow;
            }
            break;
        default: // Empty
            break;
    }
}

void kbdInit(void)
{
    uint8_t i;
    fifoInit(&kbd_scan);
    for(i=0; i<KBD_SCAN_SIZE; i++)
    {
        _kbd_scan_prev[i] = 0;
    }
    twi_state_t st;
    _kbd_state = ksSelectRow;
    _kbd_row_index = 0;

    egpioWriteReg(EGPIO_CONFIG, 0xFF); // All inputs
    while((st = egpioCheckStatus()) == twiStNone) wdr();
    delay();
    egpioWriteReg(EGPIO_POLARITY, 0xFF); // All inverted
    while((st = egpioCheckStatus()) == twiStNone) wdr();
    delay();
    egpioWriteReg(EGPIO_OUTPUT, 0); // Prepare output to keyboard scan
    while((st = egpioCheckStatus()) == twiStNone) wdr();
    delay();
}

// vim: sw=4:ts=4:si:et

