#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_

#include <ch.h>
#include <inttypes.h>

Thread* modbusRtuInit(BaseChannel* chan);

extern void modbusTableSet(uint16_t addr, uint16_t value);
extern uint16_t modbusTableGet(uint16_t addr);

#define modbusDevId() (0x77)

#endif /* _MODBUS_RTU_H_ */

