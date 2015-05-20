/**
 * \mainpage Homeserver - Automation with ATMEL AVR
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1:
 *
 * etc...
 */

#include "global.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <stdio.h>
#include <stdlib.h>


#define DEBUG

extern "C" {
#include "ds18x20lib.h"
#include "delay.h"
#include "debug.h"
#include "eth-driver.h"
#include "hello-world.h"
#include "nip.h"
}


void write_test_packet()
{
    static uint8_t pkt1[84] = {
        0x45, 0x00, /* ......E. */
        0x00, 0x54, 0x55, 0x68, 0x40, 0x00, 0x40, 0x01, /* .TUh@.@. */
        0xe7, 0x3e, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x00, /* .>...... */
        0x00, 0x01, 0x08, 0x00, 0x82, 0xaf, 0x05, 0xb5, /* ........ */
        0x00, 0x01, 0x71, 0xf9, 0x18, 0x55, 0x00, 0x00, /* ..q..U.. */
        0x00, 0x00, 0x18, 0x79, 0x0e, 0x00, 0x00, 0x00, /* ...y.... */
        0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, /* ........ */
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, /* ........ */
        0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, /* .. !"#$% */
        0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, /* &'()*+,- */
        0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, /* ./012345 */
        0x36, 0x37                                      /* 67 */
    };
    uint8_t dst[] = {0x00, 0x90, 0xf5, 0xd7, 0x30, 0x6a};

    debug("Write Packet ...");
    write_packet(dst, 0x08, pkt1, 84);

}

inline void deb_c(uint8_t c)
{
	while ( !( UCSRA & (1<<UDRE)));
	UDR = c;
	while ( !( UCSRA & (1<<UDRE)));
	UDR = 0x0a;
}

int main(void)
{

    debug("Starting Programm..");

//    hello_world_init();

    sei();


    //Timer0 Interrupt konfigurieren
    TCCR0 = (1 << CS01);
    TIMSK |= (1 << TOIE0);

    //External INT2 konfigurieren
    GICR |= (1 << INTF2);
    MCUCSR &= ~(1 << ISC2); //activate on falling edge

	// Initialize ETH Hardware + IP-Stack
	eth_init_drv();
	ip_init();


	debug("Entering Endless While");
    while (1) {	}


    /*
    for (int i = 0; i < 8192; i++) {
        debugc("%02x ", read_buffer_memory());

        if (i % 20 == 0)
            debugnl();
    }
    */

    /*
    one_wire_T ow = {  M_PORTA, M_PINA, M_DDRA, 4};
    sensorT sensor;
    for (int i=0;i<10;i++) {
        while (search_slaves(&ow, &sensor) == TRUE) {
            debugn("ROM: ");

            for (int i = 0; i < 8; i++) {
                debugc("%x ", sensor.rom[i]);
            }

            switch (getType(&sensor)) {
            case DS1822:
                debugc("DS1822");
                break;

            case DS18B20:
                debugc("DS18B20");
                break;

            case DS18S20:
                debugc("DS18S20");
                break;

            default:
                debugnl("Other Device");
            }

            set_resolution(&ow, &sensor, RESOLUTION_HIGHEST);

            float temp = read_temp(&ow, &sensor);
            char s[8];
            dtostrf(temp, 0, 2, s);
            debugc(" Temperature = %s Grad Celsius", s);
            debugnl();
        }

        debug("\n\r");
    }
    */
}

// ISR

static uint16_t c;
ISR(TIMER0_OVF_vect)
{
    if (!++c) {
        debug("--MARK--");

        // check missed Interrupts
		eth_handle_intr();

    }

}

ISR(USART_RXC_vect)
{
    do {
		debug("Entered USART_RXC");
        wdt_enable(WDTO_15MS);

        for (;;) { }
    } while (0);
}

ISR(INT2_vect)
{
    eth_handle_intr();
}
