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

#define ERROR       0
#define OK          1

#define MAGIC_START "BOOTLOADER_START"

#define ERR     "0"
#define WARN    "1"
#define INFO    "2"
#define DEBUG   "3"

// Debugging
//#define DEB

#ifdef DEB

#define debug(...)   _log(DEBUG __VA_ARGS__)
#else
#define debug(...)
#endif

#define check(exp,text,...) if(exp) { _log(text "\n" ,##__VA_ARGS__); return ERROR; }

/**
 * log string to uart
 */
int _log(char const* s, ...)
{
    int retval;
    va_list args;
    va_start(args, s);

    retval = vprintf(s, args);
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
 */
uint8_t read_uart( uint8_t* buf, uint8_t len)
{
    uint8_t ptr = 0;

    debug("Read from uart %d bytes\n", len);

    uint32_t timeout = F_CPU / 5;
    cli();

    if (UCSRA & (1 << DOR)) {
        _log(ERR "Data Overrun!\n");
		buf[ptr++] = UDR;
        return ERROR;
    }

    while (ptr < len) {
        if (UCSRA & (1 << RXC)) {
            buf[ptr++] = UDR;
        }

        timeout--;

        if (! timeout) {
            break;
        }
    }

    sei();

    debug("read uart (%d:%d): ", ptr, len);

#ifdef DEB
    if (ptr == len) {
        for (int i = 0; i < len; i++) {
            _log("%02x ", buf[i]);
        }
    }
#endif

    debug(" timeout: %ld\n", timeout);

    return ptr == len ? OK : ERROR;

}

/**
 * read from usart and check, if read bytes equals to *bytes
 */
uint8_t check_uart( uint8_t* bytes, uint8_t len)
{
    uint8_t buf[len];
    uint8_t retval = OK;

    if (read_uart(buf, len) == OK) {
        if (strncmp((char*)buf, (char*)bytes, len) != 0)
            retval = ERROR;
    } else {
        retval = ERROR;
    }

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

    debug("Crc: %02x <> %02x Remote\n", own_crc16, crc16);

    return crc16 == own_crc16;
}

/**
 * write a page to flash ROM
 */
uint8_t write_page(uint8_t* page, uint16_t page_num)
{
	uint16_t page_addr = page_num*SPM_PAGESIZE;

	_log(INFO "Write Page %04d at 0x%04x ... ",page_num,page_addr);
#ifdef DEB
	for(int i=0;i<SPM_PAGESIZE;i++) {
			_log("%02x ",page[i]);
	}
#endif
	_log("\n");

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
uint8_t receive_flash()
{
    uint16_t pages_all;
    uint16_t page_count = 0;
    uint16_t page_size;
    uint16_t page_num;
    uint8_t page[SPM_PAGESIZE];
    uint16_t crc16;

    const char magic[] = MAGIC_START;
    check(!check_uart((uint8_t*)magic, 16), ERR "No Begin found")
    check(!read_uart((uint8_t*)&pages_all, 2),
          ERR "No Page count transmitted")

    while (page_count < pages_all) {

		//clear page
		for (int i=0;i<SPM_PAGESIZE;i++)
				page[i]=0x0;

		// receive page
        check(!read_uart((uint8_t*)&page_num, 2), ERR "No Page num")
        check(!read_uart((uint8_t*)&page_size, 2),
              ERR "No Page size for Page %d", page_num)
        check(!read_uart((uint8_t*)&crc16, 2), ERR "Incomplete SHA received")
        check(!read_uart(page, page_size), ERR "Incomplete Page received")

        if (check_crc16(crc16, page, page_size)) {
            if (!write_page(page, page_num))
                return ERROR;
        } else
            return ERROR;

        page_count++;
    }

    _log(INFO "Everything is ok - ending write-flash\n");
    return OK;

}

/**
 * bootloader main
 */
int main(void)
{
    usart_init();
    _log(INFO "Bootloader! - lookout for flashing ...\n");

    receive_flash();

    _log(INFO "start main...\n");

    usart_disable();

    // jmp to 0x0
    void (*start)( void ) = 0x0;
    start();

    return 0;
}



