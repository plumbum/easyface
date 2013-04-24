#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_

#include <ch.h>
#include <inttypes.h>

#define MODBUS_VALUES_SIZE 64
#define MODBUS_TIMEOUT 20

#define MODBUS_FUNC_SETSINGLE 0x06
#define MODBUS_FUNC_SETMULTI 0x10

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
    uint16_t values[MODBUS_VALUES_SIZE]; // TODO!!!
    uint16_t crc;
    /* Flags */
    char done:1;
    /* Config */
    uint8_t device_id;
    systime_t timeout;
    /* Internal state */
    systimer_t timestamp;
    modbus_state_t mb_state;
};

typedef struct modbus_s modbus_t;

void modbusInit(modbus_state_t* mb, uint8_t device_id, systimer_t timeout);

char modbusProcess(modbus_state_t* mb);

#endif /* _MODBUS_RTU_H_ */

