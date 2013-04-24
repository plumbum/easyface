#include "modbus_rtu.h"

#include "uart.h"

uint16_t fcrc16i(uint16_t crc16, uint8_t data);

int modbusRegisterResponce(BaseChannel* chan, uint8_t devid, uint8_t funcno, int32_t regaddr, int32_t datalen);
int modbusReadRegister(BaseChannel* chan, uint8_t devid, uint8_t funcno, uint16_t* values, uint16_t len);


void modbusInit(modbus_state_t* mb, uint8_t device_id, systimer_t timeout)
{
    mb->done = 0;
    mb->crc = 0xFFFF;
    mb->timestamp = systimer;
    mb->device_id = device_id;
    mb->timeout = timeout;
    mb->mb_state = mstWait;
}

char modbusProcess(modbus_state_t* mb)
{
    if(!uartGetReady()) // No data
    {
        if((uint16_t)(mb->systimer - mb->timestamp) > mb->timeout)
        {
            // Timeout -> reset state machine
            mb->mb_state = mstWait;
            mb->timestamp = systimer;
            return 0;
        } else {
            return 0;
        }
    }

    uint8_t c;
    c = uartGetRx();

    switch(mb->mb_state)
    {
        case mstWait:
            break;

}

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


