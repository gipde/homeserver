#define DEBUG
#include "debug.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef DEBUG_UART
#include <avr/io.h>
#include <stdlib.h>
#include "global.h"
#include <util/setbaud.h>
void usart_init()
{
    /* Enable Transmitter */
    UCSRB |= (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);
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
static FILE n_out = FDEV_SETUP_STREAM(uart_putc, NULL,
                                      _FDEV_SETUP_WRITE);
#endif

#ifdef DEBUG_SIMUL
#define special_output_port (*((volatile char *)0x20))
void debug_puts(const char str)
{
    special_output_port = str;
}
static FILE n_out = FDEV_SETUP_STREAM(debug_puts, NULL,
                                      _FDEV_SETUP_WRITE);
#endif

static uint8_t debug_inited = 0;
void debug_init()
{
    stdout = &n_out;
    debug_inited = 1;
}

int debug_printf (char const* s, ...)
{
    int retval;
    va_list args;
    va_start(args, s);

    if (!debug_inited) {
        debug_init();
#ifdef DEBUG_UART
        usart_init();
#endif
    }

    retval = vprintf(s, args);
    va_end(args);
    return retval;
}


// dummy Impl -> Warning: it also produces asm code !
void debug_default(char const* s, ...)
{
    (void)(s);
}
