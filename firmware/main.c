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


#define MB_SEGMENT_MASK         0xE000

#define MB_LCD_DATA             0x0000
#define MB_LCD_COL              0x007F
#define MB_LCD_ROW              0x1F80
#define MB_LCD_POS_MASK         (MB_LCD_COL | MB_LCD_ROW)

/* Configuration registers */
#define MB_CONFIG               0x2000
#define MB_CONFIG_LCD_POS       0x00
#define MB_CONFIG_LCD_CURSOR    0x01
#define MB_CONFIG_LCD_LIGHT     0x02
#define MB_CONFIG_LCD_CLEAR     0x03

/* Read keyboard scancode FIFO */
#define MB_KEYBOARD             0x2800
#define MB_KEYBOARD_BUF_SIZE    0x00
#define MB_KEYBOARD_BUF         0x01

void writeConfigRegister(uint16_t addr, uint16_t val)
{
    switch(addr)
    {
        case MB_CONFIG_LCD_POS:
            lcdGoto(val>>8, val&0xFF);
            break;
        case MB_CONFIG_LCD_CURSOR:
            lcdCursor(val & 0x03);
            break;
        case MB_CONFIG_LCD_LIGHT:
            lcdBackLight(val & 0xFF);
            break;
        case MB_CONFIG_LCD_CLEAR:
            lcdClr();
            break;
    }
}

void writeRegister(modbus_t* mb)
{
    switch(mb->regaddr & MB_SEGMENT_MASK)
    {
        case MB_LCD_DATA:
            {
                lcdGoto(((mb->regaddr>>7)&0x3F), (mb->regaddr&0x7F));
                lcdPutArr((char*)mb->values, mb->regcnt*2);
            }
            break;
        case MB_CONFIG:
            {
                uint8_t i;
                uint16_t addr = (mb->regaddr & 0x0FFF);
                for(i=0; i<mb->regcnt; i++)
                {
                    writeConfigRegister(addr, mb->values[i]);
                    addr++;
                }
            }
            break;
    }
}

void readRegister(modbus_t* mb)
{
    uint8_t i, v;
    switch(mb->regaddr & MB_SEGMENT_MASK)
    {
        case MB_LCD_DATA:
            if(mb->funcno == MODBUS_FUNC_READ_HOLDING_REGISTERS)
            {
            }
            break;
        case MB_CONFIG:
            break;
        case MB_KEYBOARD:
            if(mb->funcno == MODBUS_FUNC_READ_INPUT_REGISTERS)
            {
                switch(mb->regaddr & 0x0F)
                {
                    case MB_KEYBOARD_BUF_SIZE:
                        mb->values[0] = fifoUsed(&kbd_scan);
                        break;
                    case MB_KEYBOARD_BUF:
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
                case MODBUS_FUNC_READ_HOLDING_REGISTERS:
                case MODBUS_FUNC_READ_INPUT_REGISTERS:
                    readRegister(&modbus);
                    modbusRegisterResponce(&modbus);
                    break;
                case MODBUS_FUNC_FORCE_SINGLE_COIL:
                case MODBUS_FUNC_FORCE_MULTIPLE_COILS:
                    modbusRegisterResponce(&modbus);
                    break;
                case MODBUS_FUNC_PRESET_SINGLE_REGISTER:
                case MODBUS_FUNC_PRESET_MULTIPLE_REGISTERS:
                    writeRegister(&modbus);
                    modbusRegisterResponce(&modbus);
                    break;
            }

        }
    }

    return 0;
}

// vim: sw=4:ts=4:si:et

