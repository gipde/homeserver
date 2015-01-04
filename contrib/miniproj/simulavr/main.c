#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define special_output_port (*((volatile char *)0x20))
void debug_puts(const char str)
{
    special_output_port = str;
}

static FILE mystdout = FDEV_SETUP_STREAM(debug_puts, NULL,
                       _FDEV_SETUP_WRITE);

int add(int a, int b)
{
    return a + b;
}


int main (void)
{

    stdout = &mystdout;
    printf("Setting Port B to Input %d\n", 1);

    PORTB &= ~(_BV(PB4));

    DDRB |= _BV(PB4);
    DDRB &= ~(_BV(PB4));

    printf("Waiting...\n");
    printf("End\n");

    return 0;
}
