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

#define MODBUS_CMD_KEYBOARD 0x4000

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
            egpioWriteReg(mb->regaddr & 0x03, mb->values[0]);
            break;
    }
}

void readRegister(modbus_t* mb)
{
    uint8_t i, j;
    switch(mb->regaddr & MODBUS_CMD_MASK)
    {
        case MODBUS_CMD_LCD:
            break;
        case MODBUS_CMD_LCDCTRL:
            break;
        case MODBUS_CMD_KEYBOARD:
            if(mb->funcno == 4)
            {
                i = mb->regaddr & 0xFF;
                for(j=0; j<mb->regcnt; j++)
                {
                    mb->values[j] = j+1;
                    i++;
                    if(i > 7) i = 0;
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


    /*
    uint8_t sr;
    sr = egpioWriteReg(3, 0);
    sr = egpioWriteReg(1, 0xAA);
    uint8_t val;
    sr = egpioReadReg(0, &val);
    */

    uartInit(UART19200);

    timerInit();
    egpioInit();
    //kbdInit();
    sei();

    twi_state_t st;
    uint8_t r;
    do {
        r = 3;
        egpioWriteReg(3, 0x00);
        while((st = egpioCheckStatus()) == twiStNone) wdr();
        if(st != twiStDone) break;
        for(i=0; i<100; i++) asm volatile(" nop ");
        r = 1;
        egpioWriteReg(1, 0xAA);
        while((st = egpioCheckStatus()) == twiStNone) wdr();
        if(st != twiStDone) break;
        for(i=0; i<100; i++) asm volatile(" nop ");
        r = 2;
        egpioReadReg(0);
        while((st = egpioCheckStatus()) == twiStNone) wdr();
        if(st != twiStDone) break;
    } while(0);

    lcdGoto(0, 0);
    switch(st)
    {
        case twiStDone:
            lcdPuts_P(PSTR("I2C done "));
            break;
        case twiStError:
            lcdPuts_P(PSTR("I2C error "));
            break;
        default:
            lcdPuts_P(PSTR("I2C unknow "));
            break;
    }
    uctox(r, sbuf);
    lcdPuts(sbuf);
    lcdPutc(' ');
    uctox(twi.status, sbuf);
    lcdPuts(sbuf);
    lcdPutc(' ');
    uctox(twi.reg_value, sbuf);
    lcdPuts(sbuf);

    while(1)
    {
        wdr();
    }

    modbusInit(&modbus, 0x01, 50);

    while(1)
    {
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

        kbdProcess();

        lcdGoto(0, 0);
        uctox(kbd_scancode, sbuf);
        lcdPuts(sbuf);
        /*
        for(i=0; i<8; i++)
        {
            uctox(kbd_buffer[i], sbuf);
            lcdPuts(sbuf);
            lcdPutc(':');
        }
        */
    }

    return 0;
}

// vim: sw=4:ts=4:si:et

