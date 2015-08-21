/*
 * Bootloader der von USART Daten empfängt und in das Flash schreibt
 *
 * TODO:
 *  vprintf ersetzen (da es zu gross ist)
 *  allgemein verschlanken (aktuell 2588 Bytes -> das sollte auch in 512 reingehen)
 *
 */


#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <util/setbaud.h>
#include <util/crc16.h>

// Return Codes
#define TIMEOUT     -1
#define ERROR       0
#define OK          1

#define MAGIC_START "BOOTLOADER_START"

#define ERR     "0"
#define WARN    "1"
#define INFO    "2"
#define DEBUG   "3"

// Debugging
//#define DEB
//#define TRACE

#ifdef DEB
#define debug(...)   _log(DEBUG __VA_ARGS__)
#else
#define debug(...)
#endif

#define check(exp,text,...) ret = exp; if(ret<1) { _log(text  ,##__VA_ARGS__); return ret; }

/**
 * log string to uart
 */
int _log(const char* s, ...)
{
    int retval;
    va_list args;
    va_start(args, s);

    retval = vprintf(s, args);
    printf("\n");
    va_end(args);
    return retval;
}

int uart_putc(unsigned char c);

/* redirect vprintf */
static FILE n_out = FDEV_SETUP_STREAM(uart_putc, NULL,
                                      _FDEV_SETUP_WRITE);
/**
 * initialize uart
 */
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
    debug("USART inited :)");
}

/**
 * disable uart
 */
void usart_disable()
{
    UCSRB |= (0 << TXEN) | (0 << RXEN);
}

/**
 *  put character on USART
 */
int uart_putc(unsigned char c)
{
    while (!(UCSRA & (1 << UDRE)))  {} /* warten bis Senden moeglich */

    UDR = c;                      /* sende Zeichen */
    return 0;
}


/*
 * read from usart and exit, if not read len bytes in a specific time
 * returns only Return Codes
 */
int8_t read_uart( uint8_t* buf, uint8_t len)
{
    uint8_t ptr = 0;
    int8_t retval = OK;

    debug("Read from uart %d bytes", len);

    uint32_t timeout = F_CPU / 5;
    cli();

    while (ptr < len) {

        while (UCSRA & (1 << DOR)) {
            buf[ptr] = UDR;
            retval = ERROR;
        }

        if (retval == ERROR) {
            _log(ERR "Data Overrun %d !", UCSRA & (1 << DOR));
            break;
        }

        if (UCSRA & (1 << RXC)) {
            buf[ptr++] = UDR;
        }

        timeout--;

        if (timeout < 1) {
            retval = TIMEOUT;
            debug("returning timeout exeeds %d -> %d", timeout, retval);
            break;
        }
    }

    // check length
    if (retval == OK) {
        debug("check length %d-%d (%d)", ptr, len, retval);

        if ( ptr != len) {
            retval = ERROR;
        }
    }

    sei();


#ifdef TRACE
    debug("read uart (%d:%d): ", ptr, len);

    if (ptr == len) {
        for (int i = 0; i < len; i++) {
            debug(" %02x ", buf[i]);
        }
    }

    debug(" timeout: %ld", timeout);
#endif



    debug("returning %d", retval);
    return retval;

}

/**
 * read from usart and check, if read bytes equals to *bytes
 */
int8_t check_magic( uint8_t* bytes, uint8_t len)
{
    uint8_t buf[len];

    for (int i = 0; i < len; i++)
        buf[i] = 0x0;

    // read the magic string
    int8_t retval = read_uart(buf, len);


    if (retval > 0) {

        debug("buf[0]=0x%x", buf[0]);

        if (buf[0] == 'x') {
            // prefixed with x - we have to read one more
            memcpy(buf, buf + 1, len - 1);
            retval = read_uart(buf + len - 1, 1);
        }

        // compare
        debug("%d bytes : <<%16s>> <> <<%16s>>", len, buf, bytes);

        if (strncmp((char*)buf, (char*)bytes, len) != 0)
            retval = ERROR;
    }

    debug( "check_uart returning %d", retval);
    return retval;
}

/**
 * check crc16
 */
uint8_t check_crc16(uint16_t crc16, uint8_t* page, uint8_t page_size)
{

    uint16_t own_crc16 = 0;

    for (int i = 0; i < page_size; i++) {
        own_crc16 = _crc16_update(own_crc16, page[i]);
    };

    debug("Crc: %02x <> %02x Remote", own_crc16, crc16);

    return crc16 == own_crc16;
}

/**
 * write a page to flash ROM
 */
uint8_t write_page(uint8_t* page, uint16_t page_num)
{
    uint16_t page_addr = page_num * SPM_PAGESIZE;

    _log(INFO "Write Page %03d at 0x%04x... ", page_num, page_addr);
#ifdef TRACE

    for (int i = 0; i < SPM_PAGESIZE; i++) {
        debug("%02x ", page[i]);
    }

#endif

    uint8_t sreg = SREG;
    cli();
    eeprom_busy_wait();

    boot_page_erase(page_addr);
    boot_spm_busy_wait();

    for (int i = 0; i < SPM_PAGESIZE; i += 2) {
        /* Set up little-endian word. */
        uint16_t w = *page++;
        w += (*page++) << 8;

        boot_page_fill (page_addr + i, w);
    }

    boot_page_write (page_addr);     /* Store buffer in flash page.      */
    boot_spm_busy_wait();       /* Wait until the memory is written.*/

    /* Reenable RWW-section again. We need this if we want to jump back */
    /* to the application after bootloading. */
    boot_rww_enable ();

    /* Re-enable interrupts (if they were ever enabled). */
    SREG = sreg;
    sei();

    return OK;
}

/**
 * receives the flash over usart
 */
int8_t receive_flash()
{
    uint16_t pages_all;
    uint16_t page_count = 0;
    uint16_t page_size;
    uint16_t page_num;
    uint8_t page[SPM_PAGESIZE];
    uint16_t crc16;

    int8_t ret;

    const char magic[] = MAGIC_START;
    check(check_magic((uint8_t*)magic, 16), ERR "No Begin found")
    check(read_uart((uint8_t*)&pages_all, 2),
          ERR "No Page count")
    _log(INFO "Pages: %d", pages_all);

    while (page_count < pages_all) {

        //clear page
        for (int i = 0; i < SPM_PAGESIZE; i++)
            page[i] = 0x0;

        // receive page
        check(read_uart((uint8_t*)&page_num, 2), ERR "No Page num");
        debug( "Page num: %d", page_num);

        check(read_uart((uint8_t*)&page_size, 2),
              ERR "No Page size for Page %d", page_num);
        debug( "Page size: %d", page_size);

        check(read_uart((uint8_t*)&crc16, 2), ERR "Incomplete SHA");
        debug( "Page CRC16: %d", crc16);

        check(read_uart(page, page_size), ERR "Incomplete Page");

        if (check_crc16(crc16, page, page_size)) {
            if (!write_page(page, page_num))
                return ERROR;
        } else
            return ERROR;

        page_count++;
    }

    _log(INFO "Write Flash ok");
    return OK;

}

/**
 * bootloader main
 */
int main(void)

{
    usart_init();
    _log(INFO "BOOTLOADER");

    int8_t ret;

    while ((ret = receive_flash()) == ERROR) {
        usart_init();
        _log(ERR "Error during receive, starting again ...");
    }

    void (*start)( void ) = 0x0;

    if (ret == TIMEOUT ) {
        _log(INFO "Timeout reached");
    }

    _log(INFO "starting main...");

    usart_disable();

    start();

    return 0;
}



