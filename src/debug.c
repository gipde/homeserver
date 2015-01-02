#include <avr/io.h>
#include <stdlib.h>
#include "global.h"
#include "debug.h"

#ifdef DEBUG_UART
static uint8_t usart_inited=0;

void usart_init()
{
    /* Enable Transmitter */
    UCSRB |= (1 << TXEN);
    // 8N1
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); // Asynchron 8N1
    // BAUD RATE
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;
    usart_inited=1;
}

int uart_putc(unsigned char c)
{
    while (!(UCSRA & (1 << UDRE)))  {} /* warten bis Senden moeglich */

    UDR = c;                      /* sende Zeichen */
    return 0;
}

void uart_printf (char const* s, ...)
{
    //TODO:
    if (!usart_inited) 
        usart_init();
    while (*s)  {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen(Terminator)" */
        uart_putc(*s);
        s++;
    }
}
#endif

