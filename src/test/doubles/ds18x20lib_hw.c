#include <avr/interrupt.h>
#include "../../main/ds18x20lib.h"
#include "../../main/ds18x20lib_hw.h"

#define DEBUG
#include "../../main/debug.h"
#include "../cases/Ds18x20libTest.h"
#include "../cases/mock.h"


void interrupts()
{
    EVENT(INTERRUPTS);
    debug("interrupts allowed");
    sei();
}

void no_interrupts()
{
    EVENT(NO_INTERRUPTS);
    debug("no interrupts");
    cli();
}

void power(uint8_t mode)
{
    EVENT(POWER, mode);

    if (mode == HIGH)
        debug("setting Power HIGH");
    else
        debug("setting Power LOW");
}

void direction(uint8_t dir)
{
    EVENT(DIRECTION, dir);

    if (dir == INPUT)
        debug("setting Direction INPUT");
    else
        debug("setting Direction INPUT");
}

uint8_t read_pin()
{
    EVENT(READ_PIN);
    debug("reading pin");
    return 0;
}


