#include "../../main/delay.h"
#include "../cases/Ds18x20libTest.h"

void delay_us(uint16_t const us)
{
    EVENT(DELAY_US, us);
}

void delay_ms(uint16_t const ms)
{
    EVENT(DELAY_MS, ms);
}
