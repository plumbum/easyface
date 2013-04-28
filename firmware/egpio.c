#include "egpio.h"
#include "config.h"
#include <util/twi.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "helpers.h"

#define I2C_ADDR 0x40

#define F_SCL (100000UL)

#define TWI_PRESCALER 1

#define TWI_BR() ((F_CPU/F_SCL-16)/(2*TWI_PRESCALER))

twi_t twi;

static void twiInit(void)
{
    TWBR = TWI_BR();
    TWSR = 0;
    TWCR = 0;
    twi.state = twiStNone;
    twi.macro = twiMacroNone;
    twi.sl_addr = I2C_ADDR;
}

static void twiStart(void)
{
    TWCR = (1<<TWSTA) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
}

static void twiStop(void)
{
    TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
}

static void twiAddrWr(uint8_t addr)
{
    TWDR = addr | 0x00;
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
}

static void twiAddrRd(uint8_t addr)
{
    TWDR = addr | 0x01;
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
}

static void twiWrite(uint8_t b)
{
    TWDR = b;
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
}

static void twiReadAck(void)
{
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
}

static void twiReadNack(void)
{
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
}

static void twiIsrMacroNone(uint8_t sr)
{
    switch(twi.state)
    {
        case twiStStart:
            if(sr != TW_START)
            {
                twi.state = twiStError;
                twiStop();
            } else {
                twiAddrWr(twi.sl_addr);
                twi.state = twiStSlaWr;
            }
            break;
        case twiStSlaWr:
            if(sr != TW_MT_SLA_ACK)
            {
                twi.state = twiStError;
            } else {
                twi.state = twiStDone;
            }
            twiStop();
            break;
        default:
            twi.state = twiStError;
            twiStop();
            break;
    }
}

static void twiIsrMacroWrite(uint8_t sr)
{
    switch(twi.state)
    {
        case twiStStart:
            if(sr != TW_START)
            {
                twi.state = twiStError;
                twiStop();
            } else {
                twiAddrWr(twi.sl_addr);
                twi.state = twiStSlaWr;
            }
            break;
        case twiStSlaWr:
            if(sr != TW_MT_SLA_ACK)
            {
                twi.state = twiStError;
                twiStop();
            } else {
                twi.state = twiStWrite;
                twiWrite(twi.reg_addr);
            }
            break;
        case twiStWrite:
            if(sr != TW_MT_DATA_ACK)
            {
                twi.state = twiStError;
                twiStop();
            } else {
                twiWrite(twi.reg_value);
                twi.state = twiStStop;
            }
            break;
        case twiStStop:
            if(sr != TW_MT_DATA_ACK)
            {
                twi.state = twiStError;
            } else {
                twi.state = twiStDone;
            }
            twiStop();
            break;
        default:
            twi.state = twiStError;
            twiStop();
            break;
    }
}

static void twiIsrMacroRead(uint8_t sr)
{
    switch(twi.state)
    {
        case twiStStart:
            if(sr != TW_START) {
                twi.state = twiStError;
                twiStop();
            } else {
                twiAddrWr(twi.sl_addr);
                twi.state = twiStSlaWr;
            }
            break;
        case twiStSlaWr:
            if(sr != TW_MT_SLA_ACK) {
                twi.state = twiStError;
                twiStop();
            } else {
                twi.state = twiStWrite;
                twiWrite(twi.reg_addr);
            }
            break;
        case twiStWrite:
            if(sr != TW_MT_DATA_ACK) {
                twi.state = twiStError;
                twiStop();
            } else {
                twi.state = twiStReStart;
                twiStart();
            }
            break;
        case twiStReStart:
            if(sr != TW_REP_START) {
                twi.state = twiStError;
                twiStop();
            } else {
                twi.state = twiStSlaRd;
                twiAddrRd(twi.sl_addr);
            }
            break;
        case twiStSlaRd:
            if(sr != TW_MR_SLA_ACK) {
                twi.state = twiStError;
                twiStop();
            } else {
                twi.state = twiStRead;
                twiReadNack();
            }
            break;
        case twiStRead:
            if(sr != TW_MR_DATA_NACK) {
                twi.state = twiStError;
            } else {
                twi.reg_value = TWDR;
                twi.state = twiStDone;
            }
            twiStop();
            break;
        default:
            twi.state = twiStError;
            twiStop();
            break;
    }
}

ISR(TWI_vect)
{
    uint8_t sr = TW_STATUS;
    twi.status = sr;
    switch(twi.macro)
    {
        case twiMacroNone:
            twiIsrMacroNone(sr);
            break;
        case twiMacroWriteReg:
            twiIsrMacroWrite(sr);
            break;
        case twiMacroReadReg:
            twiIsrMacroRead(sr);
            break;
    }
}


void egpioWriteReg(uint8_t reg, uint8_t val)
{
    twi.state = twiStStart;
    twi.macro = twiMacroWriteReg;
    twi.reg_addr = reg;
    twi.reg_value = val;
    twiStart();
    /*
    uint8_t sr;
    do {
        if((sr = twiStart()) != 0) break;
        if((sr = twiAddrWr(I2C_ADDR)) != 0) break;
        if((sr = twiWrite(reg)) != 0) break;
        if((sr = twiWrite(val)) != 0) break;
    } while(0);
    twiStop();
    return sr;
    */
}

void egpioReadReg(uint8_t reg)
{
    twi.state = twiStStart;
    twi.macro = twiMacroReadReg;
    twi.reg_addr = reg;
    twi.reg_value = 0;
    twiStart();
    /*
    uint8_t sr;
    do {
        if((sr = twiStart()) != 0) break;
        if((sr = twiAddrWr(I2C_ADDR)) != 0) break;
        if((sr = twiWrite(reg)) != 0) break;
        if((sr = twiStart()) != 0) break;
        if((sr = twiAddrRd(I2C_ADDR)) != 0) break;
        if((sr = twiRead(val, 1)) != 0) break;
    } while(0);
    twiStop();
    return sr;
    */
}

void egpioCheckDevice(void)
{
    twi.state = twiStStart;
    twi.macro = twiMacroNone;
    twiStart();
}

twi_state_t egpioCheckStatus(void)
{
    if((twi.state == twiStDone) || (twi.state == twiStError))
        return twi.state;
    else
        return twiStNone;
}

void egpioInit(void)
{
    twiInit();

    /*
    uint8_t sr;
    do {
        // if((sr = egpioWriteReg(3, 0xFF)) != 0) break;
        // if((sr = egpioWriteReg(1, 0xFF)) != 0) break;
        // if((sr = egpioWriteReg(2, 0x00)) != 0) break;
    } while(0);
    if(sr)
    {
        char sbuf[8];
        lcdPuts_P(PSTR("Expander fail "));
        uctox(sr & TW_STATUS_MASK, sbuf);
        lcdPuts(sbuf);
        while(1) wdr();
    }
    */
}

