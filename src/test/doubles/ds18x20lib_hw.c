#include <avr/interrupt.h>
#include "../../main/ds18x20lib.h"
#include "../../main/ds18x20lib_hw.h"

#define DEBUG
#include "../../main/debug.h"

void interrupts()
{
    debug("enable Interrupt");
    sei();
}

void no_interrupts()
{
    debug("disable Interrupt");
    cli();
}

void power(uint8_t mode)
{
    no_interrupts();

    if (mode == HIGH)
        debug("setting Power HIGH");
    else
        debug("setting Power LOW");

    interrupts();
}

void direction(uint8_t dir)
{
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
    no_interrupts();
    debug("reading pin");
    interrupts();
    return 0;
}


