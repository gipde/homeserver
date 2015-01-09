#define DEBUG

#include "../../main/delay.h"
#include "../../main/debug.h"

#include "../cases/Ds18x20libTest.h"
#include "../cases/mock.h"

void delay_us(uint16_t const us)
{
    debug("delay %d us", us);
    EVENT(DELAY_US, us);
}

void delay_ms(uint16_t const ms)
{
    debug("delay %d ms", ms);
    EVENT(DELAY_MS, ms);
}
