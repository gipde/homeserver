#define F_CPU 8000000
#define BAUD 9600

#include <util/setbaud.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include "OneWire.h"

void USART_Init( unsigned int baud )
{

    /* Enable Transmitter */
    UCSRB |= (1 << TXEN);

    // 8N1
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); // Asynchron 8N1

    // BAUD RATE
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;

}

int uart_putc(unsigned char c)
{
    while (!(UCSRA & (1 << UDRE)))  {} /* warten bis Senden moeglich */

    UDR = c;                      /* sende Zeichen */
    return 0;
}

void uart_puts (char* s)
{
    while (*s)  {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen(Terminator)" */
        uart_putc(*s);
        s++;
    }
}

int main (void)
{

    char s[7];
    unsigned char data = 60;
    USART_Init(9600);

    uart_puts(dtostrf(0.24, 6, 3, s));

    uart_putc(13);
    uart_putc(10);


    //OneWire  ds(10);

    for (;;)    {

        if (data > 'Z' || data < 'A')        {
            uart_putc(':');
            uart_puts(itoa(data, s, 10));
        } else {
            uart_putc(data);
        }

        data += 1;

        if (data > 100) {
            char s[7];
            data = 60;
            uart_puts(" werner ");
            uart_puts(itoa(data, s, 10));
            uart_putc(13);
            uart_putc(10);
        }

        _delay_ms(50);

    }

}

