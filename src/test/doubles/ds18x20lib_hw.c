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
    sei();
}

void no_interrupts()
{
    EVENT(NO_INTERRUPTS);
    cli();
}

void power(uint8_t mode)
{
    EVENT(POWER, mode);
    no_interrupts();

    if (mode == HIGH)
        debug("setting Power HIGH");
    else
        debug("setting Power LOW");

    interrupts();
}

void direction(uint8_t dir)
{
    EVENT(DIRECTION, dir);
    power(LOW);
    no_interrupts();

    if (dir == INPUT)
        debug("setting Direction INPUT");
    else
        debug("setting Direction INPUT");

    interrupts();
}

uint8_t read_pin()
{
    EVENT(READ_PIN);
    no_interrupts();
    debug("reading pin");
    interrupts();
    return 0;
}


