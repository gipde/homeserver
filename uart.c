/*
 * CFile1.c
 *
 * Created: 23.12.2014 22:27:29
 *  Author: werner
 */

#include "uart.h"
#include <stdlib.h>

void usart_init()
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

void uart_puts (char const* s)
{
    while (*s)  {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen(Terminator)" */
        uart_putc(*s);
        s++;
    }
}

void uart_puti(int i)
{
    char s[8];
    itoa(i, s, 10);
    uart_puts(s);
}

void uart_putd(double d)
{
    char s[8];
    dtostrf(d, 0, 2, s);
    uart_puts(s);
}
