#include <util/delay.h>

void delay_us(uint16_t const us)
{
    for (int i = 0; i < us; i++)
        _delay_us(1);
}

void delay_ms(uint16_t const ms)
{
    for (int i = 0; i < us; i++)
        _delay_ms(1);
}

