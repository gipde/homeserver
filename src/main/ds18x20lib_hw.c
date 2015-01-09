#include <avr/interrupt.h>
#include <avr/io.h>
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
    if (mode == HIGH)
        ONEWIRE_PORT |= 1 << ONEWIRE_PORTPIN;
    else
        ONEWIRE_PORT &= ~(1 << ONEWIRE_PORTPIN);
}

void direction(uint8_t dir)
{
    if (dir == INPUT)
        ONEWIRE_DDR &= ~(1 << ONEWIRE_PORTPIN);
    else
        ONEWIRE_DDR |= 1 << ONEWIRE_PORTPIN;
}

uint8_t read_pin()
{
    uint8_t bit = (ONEWIRE_PIN & (1 << ONEWIRE_PORTPIN)) >> ONEWIRE_PORTPIN;
    return bit;
}

