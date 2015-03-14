/*
 * Der Dev-Rechner muss wohl auch byteweise per UART das Flash übertragen, da sonst so viel
 * Code benötigt wird, wenn man z.b. via Netzwerk übertragen will
 *
 * Test mit RXCIE Interrupt-Hanlder im Hauptprogramm wenn char über USART empfangen wurde
 * damit müsste es gehen, im laufenden betrieb, das Flash zu programmieren
 * 0. Beim Empfang der korrekten Signatur
 * 1. Reset des uc
 * 2. Empfang boot signatur
 * 3. flash
 * 4. start des anwendungsprogrammes
 *
 *
 */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdint.h>

#define TIMEOUT     3

#define ERROR       0
#define OK          1

#define PAGESIZE    128
#define MAGIC_START "BOOTLOADER_START"

int uart_putc(unsigned char c)
{
    while (!(UCSRA & (1 << UDRE)))  {} /* warten bis Senden moeglich */

    UDR = c;                      /* sende Zeichen */
    return 0;
}

static FILE n_out = FDEV_SETUP_STREAM(uart_putc, NULL,
                                      _FDEV_SETUP_WRITE);

int debug (char const* s, ...)
{
    int retval;
    va_list args;
    va_start(args, s);

    retval = vprintf(s, args);
    va_end(args);
    return retval;
}


void usart_init()
{
    //redirect stdout to uart_putc
    stdout = &n_out;

    // Enable Transmitter and Receiver
    UCSRB |= (1 << TXEN) | (1 << RXEN);
    // 8N1
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); // Asynchron 8N1
    // BAUD RATE
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;
}

void usart_disable()
{
    UCSRB |= (0 << TXEN) | (0 << RXEN);
}


void programPage(uint16_t page, uint8_t* buf)
{

    cli(); // Disable Interrupts

    debug("* %d,%p", page, buf);

    sei();

}

uint8_t page[PAGESIZE];
uint8_t page_ptr = 0;

uint8_t read_uart( uint8_t* buf, uint8_t len)
{
    uint8_t ptr = 0;

    debug("Read from uart %d bytes\n", len);

    uint32_t t1 = F_CPU / 10;
    //uint32_t t1 = UINT32_MAX;
    cli();

    while (ptr < len) {
        if (UCSRA & (1 << RXC)) {
            buf[ptr++] = UDR;
        }

        t1 = t1 - 1;

        if (! t1) {
            break;
        }
    }

    sei();

    if (ptr > 0) {
        for (int i = 0; i < len; i++) {
            debug("%c", buf[i]);
        }
    }

    debug("\n");

    return OK;

}


uint8_t check_uart( uint8_t* bytes, uint8_t len)
{
    uint8_t buf[len];
    uint8_t retval = OK;

    if (read_uart(buf, len) != ERROR) {
        if (strncmp((char*)buf, (char*)bytes, len) != 1)
            retval = ERROR;
    } else {
        retval = ERROR;
    }

    return retval;
}

uint8_t check_sha(uint8_t* sha, uint8_t* page, uint8_t page_size)
{
    debug("%p %p %d", sha, page, page_size);
    return OK;
}

uint8_t write_page(uint8_t* page, uint8_t page_size)
{
    debug("%p %d", page, page_size);
    return OK;
}

#define check(exp,text,...) if(exp) { debug(text "\n" ,##__VA_ARGS__); return ERROR; }
uint8_t write_flash()
{

    uint16_t pages_all;
    uint16_t page_count = 0;
    uint16_t page_size;
    uint8_t page[PAGESIZE];
    uint8_t sha[20];

    const char magic[] = MAGIC_START;
    check(!check_uart((uint8_t*)magic, 16), "No Begin found")
    check(!read_uart((uint8_t*)&pages_all, 2), "No Page count transmitted")

    while (page_count < pages_all) {

        check(!read_uart((uint8_t*)&page_size, 2),
              "No Page size for Page %d")
        check(!read_uart(sha, 20), "Incomplete SHA received")
        check(!read_uart(page, page_size), "Incomplete Page received")

        if (check_sha(sha, page, page_size)) {
            if (!write_page(page, page_size))
                return ERROR;
        } else
            return ERROR;
    }

    return OK;

}

int main(void)
{

    usart_init();
    debug("Hallo hier ist der Bootloader\n");

    write_flash();
    usart_disable();

    debug("Starting main...\n");

    /* vor Rücksprung eventuell benutzte Hardware deaktivieren
       und Interrupts global deaktivieren, da kein "echter" Reset erfolgt */


    // jmp to 0x0
    void (*start)( void ) = 0x0;
    start();

    return 0;
}



