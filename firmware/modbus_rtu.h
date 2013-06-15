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


// (0x01) — чтение значений из нескольких регистров флагов (Read Coil Status)
#define MODBUS_FUNC_READ_COIL_STATUS         1

// (0x02) — чтение значений из нескольких дискретных входов (Read Discrete Inputs)
#define MODBUS_FUNC_READ_DISCRETE_INPUTS     2

// (0x03) — чтение значений из нескольких регистров хранения (Read Holding Registers)
#define MODBUS_FUNC_READ_HOLDING_REGISTERS   3

// (0x04) — чтение значений из нескольких регистров ввода (Read Input Registers)
#define MODBUS_FUNC_READ_INPUT_REGISTERS     4

// (0x05) — запись значения одного флага (Force Single Coil)
#define MODBUS_FUNC_FORCE_SINGLE_COIL        5

// (0x06) — запись значения в один регистр хранения (Preset Single Register)
#define MODBUS_FUNC_PRESET_SINGLE_REGISTER   6

// (0x0F) — запись значений в несколько регистров флагов (Force Multiple Coils)
#define MODBUS_FUNC_FORCE_MULTIPLE_COILS     15

// (0x10) — запись значений в несколько регистров хранения (Preset Multiple Registers)
#define MODBUS_FUNC_PRESET_MULTIPLE_REGISTERS    16


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
    /* Config */
    uint8_t device_id;
    systimer_t timeout;
    /* Internal state */
    systimer_t timestamp;
    modbus_state_t state;
    /* Flags */
    char done:1;
};

typedef struct modbus_s modbus_t;

void modbusInit(modbus_t* mb, uint8_t device_id, systimer_t timeout);

int8_t modbusReadPacket(modbus_t* mb);

void modbusRegisterResponce(modbus_t* mb);

#endif /* _MODBUS_RTU_H_ */

// vim: sw=4:ts=4:si:et

