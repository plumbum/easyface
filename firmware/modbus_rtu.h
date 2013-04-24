#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_

#include <inttypes.h>
#include "systimer.h"

#define MODBUS_VALUES_SIZE 64
#define MODBUS_TIMEOUT 20

#define MODBUS_OK UART_OK
#define MODBUS_ERROR_TIMEOUT UART_TIMEOUT
#define MODBUS_ERROR_DEVID  (-2)
#define MODBUS_ERROR_FUNCNO (-3)
#define MODBUS_ERROR_CRC    (-4)

typedef enum {
    mstWait = 0,
    mstFunctionNo,
    mstRegaddr1,
    mstRegaddr2
} modbus_state_t;

struct modbus_s {
    uint8_t devid;
    uint8_t funcno;
    uint16_t regaddr;
    uint16_t regcnt;
    uint16_t values[MODBUS_VALUES_SIZE];
    uint16_t crc;
    /* Flags */
    char done:1;
    /* Config */
    uint8_t device_id;
    systimer_t timeout;
    /* Internal state */
    systimer_t timestamp;
    modbus_state_t state;
};

typedef struct modbus_s modbus_t;

void modbusInit(modbus_t* mb, uint8_t device_id, systimer_t timeout);

int8_t modbusReadPacket(modbus_t* mb);

#endif /* _MODBUS_RTU_H_ */

