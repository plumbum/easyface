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

static void modbusWriteByte(uint8_t b, modbus_t* mb)
{
    uartPutTx(b);
    mb->crc = fcrc16i(mb->crc, b);
}

static void modbusWriteWord(uint16_t w, modbus_t* mb)
{
    uint8_t b;
    b = w>>8;
    uartPutTx(b);
    mb->crc = fcrc16i(mb->crc, b);
    b = w & 0xFF;
    uartPutTx(b);
    mb->crc = fcrc16i(mb->crc, b);
}

static void modbusWriteCrc(modbus_t* mb)
{
    uartPutTx(mb->crc & 0xFF);
    uartPutTx(mb->crc >> 8);
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
            mb->regcnt = 1;
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

    mb->done = -1;
    return MODBUS_OK;
}

void modbusRegisterResponce(modbus_t* mb)
{
    uint16_t i;
    mb->crc = 0xFFFF;

    modbusWriteByte(mb->devid, mb);
    modbusWriteByte(mb->funcno, mb);

    switch(mb->funcno)
    {
        case 1: // (0x01) — чтение значений из нескольких регистров флагов (Read Coil Status)
            break;
        case 2: // (0x02) — чтение значений из нескольких дискретных входов (Read Discrete Inputs)
            break;
        case 3: // (0x03) — чтение значений из нескольких регистров хранения (Read Holding Registers)
        case 4: // (0x04) — чтение значений из нескольких регистров ввода (Read Input Registers)
            modbusWriteByte(mb->regcnt*2, mb);
            for(i=0; i<mb->regcnt; i++)
            {
                modbusWriteWord(mb->values[i], mb);
            }
            break;
        case 5: // (0x05) — запись значения одного флага (Force Single Coil)
        case 6: // (0x06) — запись значения в один регистр хранения (Preset Single Register)
            modbusWriteWord(mb->regaddr, mb);
            break;
        case 15: // (0x0F) — запись значений в несколько регистров флагов (Force Multiple Coils)
        case 16: // (0x10) — запись значений в несколько регистров хранения (Preset Multiple Registers)
            modbusWriteWord(mb->regaddr, mb);
            modbusWriteWord(mb->regcnt, mb);
            break;
    }

    modbusWriteCrc(mb);
}

