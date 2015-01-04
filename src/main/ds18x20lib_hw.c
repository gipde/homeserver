#include <avr/interrupt.h>
#include "ds18x20lib.h"
#include "ds18x20lib_hw.h"

#define DEBUG
#include "debug.h"

void interrupts()
{
    sei();
}

void no_interrupts()
{
    cli();
}

void power(uint8_t mode)
{
    no_interrupts();

    if (mode == HIGH)
        ONEWIRE_PORT |= 1 << ONEWIRE_PORTPIN;
    else
        ONEWIRE_PORT &= ~(1 << ONEWIRE_PORTPIN);

    interrupts();
}

void direction(uint8_t dir)
{
    power(LOW);
    no_interrupts();

    if (dir == INPUT)
        ONEWIRE_DDR &= ~(1 << ONEWIRE_PORTPIN);
    else
        ONEWIRE_DDR |= 1 << ONEWIRE_PORTPIN;

    interrupts();
}

uint8_t read_pin()
{
    no_interrupts();
    uint8_t bit = (ONEWIRE_PIN & (1 << ONEWIRE_PORTPIN)) >> ONEWIRE_PORTPIN;
    interrupts();
    return bit;
}

