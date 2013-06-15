/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  Terminal bridge
 *
 *        Version:  1.0
 *        Created:  Fri Mar 14 17:43:49 MSK 2008
 *       Revision:  $Id$
 *       Compiler:  gcc
 *
 *         Author:  Ivan A-R <ivan@alferov.name>
 *
 *        License:  private
 *
 * =====================================================================================
 */

#include "config.h"

#include <string.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "helpers.h"
#include "systimer.h"
#include "uart.h"
#include "lcd.h"
#include "egpio.h"
#include "keyboard.h"
#include "modbus_rtu.h"


#define MODBUS_CMD_MASK  0xE000

#define MODBUS_CMD_LCD   0x0000
#define MODBUS_LCD_COL   0x007F
#define MODBUS_LCD_ROW   0x1F80
#define MODBUS_LCD_MASK  (MODBUS_LCD_COL | MODBUS_LCD_ROW)

#define MODBUS_CMD_LCDCTRL  0x2000

#define MODBUS_CMD_KEYBOARD             0x4000
#define MODBUS_CMD_KEYBOARD_BUF_SIZE    0x4000
#define MODBUS_CMD_KEYBOARD_BUF         0x4001

char sbuf[64];

void writeRegister(modbus_t* mb)
{
    switch(mb->regaddr & MODBUS_CMD_MASK)
    {
        case MODBUS_CMD_LCD:
            lcdGoto((mb->regaddr>>7)&0x3F, mb->regaddr&0x7F);
            lcdPutArr((char*)mb->values, mb->regcnt*2);
            break;
        case MODBUS_CMD_LCDCTRL:
            break;
        case MODBUS_CMD_KEYBOARD:
            break;
    }
}

void readRegister(modbus_t* mb)
{
    uint8_t i, v;
    switch(mb->regaddr & MODBUS_CMD_MASK)
    {
        case MODBUS_CMD_LCD:
            mb->values[0] = mb->regaddr;
            mb->values[1] = mb->regcnt;
            break;
        case MODBUS_CMD_LCDCTRL:
            break;
        case MODBUS_CMD_KEYBOARD:
            if(mb->funcno == MODBUS_FUNC_READ_INPUT_REGISTERS)
            {
                switch(mb->regaddr & 0x01)
                {
                    case 0x00:
                        mb->values[0] = fifoUsed(&kbd_scan);
                        break;
                    case 0x01:
                        for(i=0; i<mb->regcnt; i++)
                        {
                            if(!fifoEmpty(&kbd_scan))
                            {
                                mb->values[i] = fifoPop(&kbd_scan);
                            } else {
                                mb->values[i] = 0xFFFF;
                            }
                        }
                        break;
                }
            }
            break;
    }
}


static modbus_t modbus;
int main(void)
{
    uint16_t i;
    // uint8_t mcusr = MCUSR;
    // MCUSR = 0;

#ifdef WDT_EN
    wdt_disable();
    wdt_enable(WDTO_1S);
#endif

    SET_DDR();

    lcdInit();
    lcdBackLight(1);
    lcdClr();
    lcdHome();
    lcdCursor(2);

    uartInit(UART19200);

    sei();
    egpioInit();
    kbdInit();
    timerInit();

    modbusInit(&modbus, 0x01, 50);

    while(1)
    {
        wdr();

        int8_t r = modbusReadPacket(&modbus);
        if(r == MODBUS_OK)
        {
            switch(modbus.funcno)
            {
                case 3:
                    for(i=0; i<modbus.regcnt; i++)
                    {
                        modbus.values[i] = i*10;
                    }
                    modbusRegisterResponce(&modbus);
                    break;
                case 4:
                    readRegister(&modbus);
                    modbusRegisterResponce(&modbus);
                    break;
                case 5:
                case 15:
                    modbusRegisterResponce(&modbus);
                    break;
                case 6:
                case 16:
                    writeRegister(&modbus);
                    modbusRegisterResponce(&modbus);
                    break;
            }

        }
    }

    return 0;
}

// vim: sw=4:ts=4:si:et

