#ifndef _EGPIO_H_
#define _EGPIO_H_

#include "config.h"

#define EGPIO_INPUT     0
#define EGPIO_OUTPUT    1
#define EGPIO_POLARITY  2
#define EGPIO_CONFIG    3

typedef enum {
    twiStNone = 0,
    twiStError,
    twiStDone,
    twiStStart,
    twiStReStart,
    twiStSlaWr,
    twiStSlaRd,
    twiStWrite,
    twiStRead,
    twiStStop,
    twiStStop2,
} twi_state_t;

typedef enum {
    twiMacroNone = 0,
    twiMacroWriteReg,
    twiMacroReadReg,
} twi_macro_t;

typedef struct {
    uint8_t sl_addr;
    uint8_t reg_addr;
    uint8_t reg_value;
    volatile twi_state_t state;
    twi_macro_t macro;
    uint8_t nbyte;
    uint8_t status;
} twi_t;

extern twi_t twi;

void egpioInit(void);

void egpioWriteReg(uint8_t reg, uint8_t val);
void egpioReadReg(uint8_t reg);
void egpioCheckDevice(void);

twi_state_t egpioCheckStatus(void);


#endif /* _EGPIO_H_ */

// vim: sw=4:ts=4:si:et

