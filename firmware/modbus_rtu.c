#include "modbus_rtu.h"

#include "uart.h"

uint16_t fcrc16i(uint16_t crc16, uint8_t data);


void modbusInit(modbus_t* mb, uint8_t device_id, systimer_t timeout)
{
    mb->done = 0;
    mb->crc = 0xFFFF;
    mb->timestamp = systimer;
    mb->device_id = device_id;
    mb->timeout = timeout;
    mb->state = mstWait;
}

static int8_t modbusReadByte(uint8_t* b, modbus_t* mb)
{
    int16_t c = uartGetRxTimeout(mb->timeout);
    if(c < 0) return c;
    mb->crc = fcrc16i(mb->crc, c);
    *b = (uint8_t)c;
    return 0;
}

static int8_t modbusReadWord(uint16_t* w, modbus_t* mb)
{
    uint16_t word;
    int16_t c;

    c = uartGetRxTimeout(mb->timeout);
    if(c < 0) return c;
    mb->crc = fcrc16i(mb->crc, c);
    word = ((uint8_t)c)<<8;

    c = uartGetRxTimeout(mb->timeout);
    if(c < 0) return c;
    mb->crc = fcrc16i(mb->crc, c);
    word |= (c & 0xFF);

    *w = word;
    return 0;
}

int8_t modbusReadPacket(modbus_t* mb)
{
    int8_t st;
    uint16_t dummy_crc;
    uint16_t i;
    uint16_t val;
    uint8_t len;

    mb->done = 0;
    mb->crc = 0xFFFF;

    /* Read First byte - device ID */
    if((st = modbusReadByte(&(mb->devid), mb)) < 0) return st;

    /* Read Second byte - function no */
    if((st = modbusReadByte(&(mb->funcno), mb)) < 0) return st;

    /* Read Word - register address */
    if((st = modbusReadWord(&(mb->regaddr), mb)) < 0) return st;

    switch(mb->funcno)
    {
        case 1: // (0x01) — чтение значений из нескольких регистров флагов (Read Coil Status)
            break;
        case 2: // (0x02) — чтение значений из нескольких дискретных входов (Read Discrete Inputs)
            break;
        case 3: // (0x03) — чтение значений из нескольких регистров хранения (Read Holding Registers)
        case 4: // (0x04) — чтение значений из нескольких регистров ввода (Read Input Registers)
            if((st = modbusReadWord(&(mb->regcnt), mb)) < 0) return st;
            break;
        case 5: // (0x05) — запись значения одного флага (Force Single Coil)
            break;
        case 6: // (0x06) — запись значения в один регистр хранения (Preset Single Register)
            /* Read one register value */
            if((st = modbusReadWord(&(mb->values[0]), mb)) < 0) return st;
            break;
        case 15: // (0x0F) — запись значений в несколько регистров флагов (Force Multiple Coils)
            break;
        case 16: // (0x10) — запись значений в несколько регистров хранения (Preset Multiple Registers)
            /* Read register count */
            if((st = modbusReadWord(&(mb->regcnt), mb)) < 0) return st;
            /* Read length in bytes */
            if((st = modbusReadByte(&len, mb)) < 0) return st;
            for(i=0; i<mb->regcnt; i++)
            {
                if((st = modbusReadWord(&val, mb)) < 0) return st;
                if(i<MODBUS_VALUES_SIZE)
                    mb->values[i] = val;
            }
            // TODO long packet error
            break;
    }

    /* Read CRC */
    if((st = modbusReadWord(&dummy_crc, mb)) < 0) return st;

    if(mb->crc != 0) /* CRC error */
    {
        return MODBUS_ERROR_CRC;
    }

    if((mb->devid != mb->device_id) /* Not device id */
        && (mb->devid != 0)) /* Not broadcast address */
    {
        return MODBUS_ERROR_DEVID;
    }

    return MODBUS_OK;
}

#if 0
static WORKING_AREA(modbus_thd_wa, 512);
static msg_t modbus_thd(void *arg)
{
    BaseChannel* chan = (BaseChannel *)arg;

    msg_t c;
    uint8_t devid;
    uint8_t funcno;
    uint16_t regaddr;
    uint16_t regcnt = 0;
    uint16_t values[VALUES_SIZE]; // TODO!!!

    uint8_t values_cnt;

    uint16_t crc;

    while(!chThdShouldTerminate())
    {
        
        c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
        if(c < 0) continue;
        devid = c;

        c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
        if(c < 0) continue;
        funcno = c;

        crc = fcrc16i(0xFFFF, devid);
        crc = fcrc16i(crc, funcno);

        c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
        if(c < 0) continue;
        regaddr = c<<8;
        crc = fcrc16i(crc, c);

        c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
        if(c < 0) continue;
        regaddr |= c&0xFF;
        crc = fcrc16i(crc, c);

        if(funcno == 0x06) // Preset Single Register
        {
            c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
            if(c < 0) continue;
            values[0] = c<<8;
            crc = fcrc16i(crc, c);

            c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
            if(c < 0) continue;
            values[0] |= c&0xFF;
            crc = fcrc16i(crc, c);
        }
        else if(funcno == 0x10) // Preset Multiple Registers
        {
            int i;

            /* Get register count */
            c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
            if(c < 0) continue;
            regcnt = c<<8;
            crc = fcrc16i(crc, c);

            c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
            if(c < 0) continue;
            regcnt |= c&0xFF;
            crc = fcrc16i(crc, c);

            /* Get lenght in bytes */
            c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
            if(c < 0) continue;
            values_cnt = c;
            crc = fcrc16i(crc, c);

            for(i=0; i<regcnt; i++)
            {
                c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
                if(c < 0) break;
                if(i<VALUES_SIZE)
                    values[i] = c<<8;
                crc = fcrc16i(crc, c);

                c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
                if(c < 0) break;;
                if(i<VALUES_SIZE)
                    values[i] |= c&0xFF;
                crc = fcrc16i(crc, c);
            }
            if(c < 0) continue;
            if(regcnt>VALUES_SIZE) regcnt = VALUES_SIZE;
        }
        else if(funcno == 0x04)
        {
            /* Get register count */
            c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
            if(c < 0) continue;
            regcnt = c<<8;
            crc = fcrc16i(crc, c);

            c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
            if(c < 0) continue;
            regcnt |= c&0xFF;
            crc = fcrc16i(crc, c);

        }

        /* Read CRC */
        c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
        if(c < 0) continue;
        crc = fcrc16i(crc, c);
        c = chIOGetTimeout(chan, MODBUS_TIMEOUT);
        if(c < 0) continue;
        crc = fcrc16i(crc, c);
        
        /*
        chprintf(&SD1, "Function %02X\n", funcno);
        chprintf(&SD1, "CRC calculated %04X, readed %04X\n\n", crc, frame_crc);
        */
        if(crc != 0) /* CRC error */
        {
            chprintf((BaseChannel*)&SD1, "CRC calculated %04X\n\n", crc);
            continue;
        }

        if((devid == modbusDevId()) || (devid == 0))
        {
            if(funcno == 0x06)
            {
                modbusTableSet(regaddr, values[0]);
                if(devid != 0)
                    modbusRegisterResponce(chan, devid, funcno, regaddr, values[0]);
            }
            else if(funcno == 0x10)
            {
                int i;
                for(i=0; i<regcnt; i++)
                {
                    modbusTableSet(regaddr+i, values[i]);
                }
                if(devid != 0)
                    modbusRegisterResponce(chan, devid, funcno, regaddr, regcnt);
            }
            else if(funcno == 0x04) /* Read Input Registers */
            {
                int i;
                for(i=0; i<regcnt; i++)
                {
                    values[i] = modbusTableGet(regaddr+i);
                }
                modbusReadRegister(chan, devid, funcno, values, regcnt);
            }
        }
    }
    return 0;
}

int modbusReadRegister(BaseChannel* chan, uint8_t devid, uint8_t funcno, uint16_t* values, uint16_t len)
{
    uint16_t crc;
    uint8_t hi, lo;
    int i;

    chIOPut(chan, devid);
    crc = fcrc16i(0xFFFF, devid);

    chIOPut(chan, funcno);
    crc = fcrc16i(crc, funcno);

    chIOPut(chan, len*2);
    crc = fcrc16i(crc, len*2);

    for(i=0; i<len; i++)
    {
        hi = values[i] >> 8;
        lo = values[i] & 0xFF;
        chIOPut(chan, hi);
        chIOPut(chan, lo);
        crc = fcrc16i(crc, hi);
        crc = fcrc16i(crc, lo);
    }

    chIOPut(chan, crc & 0xFF);
    chIOPut(chan, crc >> 8);

    chThdSleepMilliseconds(2);

    return 0;
}

int modbusRegisterResponce(BaseChannel* chan, uint8_t devid, uint8_t funcno, int32_t regaddr, int32_t datalen)
{
    uint16_t crc;
    uint8_t hi, lo;

    chIOPut(chan, devid);
    crc = fcrc16i(0xFFFF, devid);
    chIOPut(chan, funcno);
    crc = fcrc16i(crc, funcno);

    if(regaddr >= 0)
    {
        hi = regaddr >> 8;
        lo = regaddr & 0xFF;
        chIOPut(chan, hi);
        chIOPut(chan, lo);
        crc = fcrc16i(crc, hi);
        crc = fcrc16i(crc, lo);
        if(datalen >= 0)
        {
            hi = datalen >> 8;
            lo = datalen & 0xFF;
            chIOPut(chan, hi);
            chIOPut(chan, lo);
            crc = fcrc16i(crc, hi);
            crc = fcrc16i(crc, lo);
        }
    }
    chIOPut(chan, crc & 0xFF);
    chIOPut(chan, crc >> 8);

    chThdSleepMilliseconds(2);

    return 0;
}
#endif

