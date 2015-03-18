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
 * TODO:
 * 0.5 Schreibroutinte in bootloader
 * 0.6 Bootloader testen und entschlanken
 * 1. Main Programm als Hexfile rausschreiben
 * 2. Main Programm einlesen von atool
 * 3. Main Programm flashen
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
#include <util/crc16.h>
#include <avr/wdt.h>


#define TIMEOUT     3

#define ERROR       0
#define OK          1

#define MAGIC_START "BOOTLOADER_START"

#define ERR		"0"
#define WARN	"1"
#define INFO	"2"
#define DEBUG	"3"

// Debugging
#define DEB
#ifdef DEB
#define mlog(...)	mylog(__VA_ARGS__)
#else
#define mlog(...)
#endif

#define soft_reset()  do { wdt_enable(WDTO_15MS); for(;;) { } } while(0)
#define check(exp,text,...) if(exp) { mlog(text "\n" ,##__VA_ARGS__); return ERROR; }

int mylog(char const* s, ...)
{
    int retval;
    va_list args;
    va_start(args, s);

    retval = vprintf(s, args);
    va_end(args);
    return retval;
}


int uart_putc(unsigned char c)
{
    while (!(UCSRA & (1 << UDRE)))  {} /* warten bis Senden moeglich */

    UDR = c;                      /* sende Zeichen */
    return 0;
}

static FILE n_out = FDEV_SETUP_STREAM(uart_putc, NULL,
                                      _FDEV_SETUP_WRITE);

//TODO: das ist noch buggy, geht erst nach minicom start
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



uint8_t read_uart( uint8_t* buf, uint8_t len)
{
    uint8_t ptr = 0;

    mlog(DEBUG "Read from uart %d bytes\n", len);

    uint32_t timeout = F_CPU / 10;
    cli();

	if (UCSRA & (1 << DOR)) {
		mlog(ERR "Data Overrun!\n");
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

#ifdef DEB
	mlog(DEBUG "read uart (%d:%d): ",ptr,len);
    if (ptr==len) {
        for (int i = 0; i < len; i++) {
            mlog("%02x ", buf[i]);
        }
    }
    mlog(" timeout: %ld\n",timeout);
#endif

    return ptr==len?OK:ERROR;

}


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

uint8_t check_crc16(uint16_t crc16, uint8_t* page, uint8_t page_size)
{

	uint16_t own_crc16=0;
	for (int i=0;i<page_size;i++) {
			own_crc16=_crc16_update(own_crc16,page[i]);
	};

	mlog(DEBUG "Crc: %02x <> %02x Remote\n",own_crc16,crc16);

    return crc16==own_crc16;
}


uint8_t write_page(uint8_t* page, uint8_t page_num)
{
    mlog(INFO "WritePage %d: %p %d\n", page_num,page);

	uint8_t sreg = SREG;
	cli();
	eeprom_busy_wait();

	boot_page_erase(page_num);
	boot_spm_busy_wait();
    for (int i=0; i<SPM_PAGESIZE; i+=2)
    {
        /* Set up little-endian word. */
        uint16_t w = *page++;
        w += (*page++) << 8;
 
        boot_page_fill (page_num + i, w);
    }
    boot_page_write (page_num);     /* Store buffer in flash page.		*/
    boot_spm_busy_wait();       /* Wait until the memory is written.*/
 
    /* Reenable RWW-section again. We need this if we want to jump back */
    /* to the application after bootloading. */
    boot_rww_enable ();
 
    /* Re-enable interrupts (if they were ever enabled). */
    SREG = sreg;

    return OK;
}

uint8_t write_flash()
{

    uint16_t pages_all;
    uint16_t page_count = 0;
    uint16_t page_size;
	uint8_t page_num;
    uint8_t page[SPM_PAGESIZE];
    uint16_t crc16;

    const char magic[] = MAGIC_START;
    check(!check_uart((uint8_t*)magic, 16), ERR "No Begin found")
    check(!read_uart((uint8_t*)&pages_all, 2), ERR "No Page count transmitted")

    while (page_count < pages_all) {

		check(!read_uart(&page_num,1),ERR "No Page num")
        check(!read_uart((uint8_t*)&page_size, 2),
              ERR "No Page size for Page %d",page_num)
        check(!read_uart((uint8_t*)&crc16, 2), ERR "Incomplete SHA received")
        check(!read_uart(page, page_size), ERR "Incomplete Page received")

        if (check_crc16(crc16, page, page_size)) {
            if (!write_page(page, page_size))
                return ERROR;
        } else
            return ERROR;

		page_count++;
    }

	mlog("Everything is ok - ending write-flash\n");
    return OK;

}

int main(void)
{

    usart_init();
    mlog(INFO "Hallo hier ist der Bootloader\n");

    write_flash();
    usart_disable();

    mlog(INFO "Starting main...\n");

    /* vor Rücksprung eventuell benutzte Hardware deaktivieren
       und Interrupts global deaktivieren, da kein "echter" Reset erfolgt */


    // jmp to 0x0
    void (*start)( void ) = 0x0;
    start();

    return 0;
}



