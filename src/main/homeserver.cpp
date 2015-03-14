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

#include <stdio.h>
#include <stdlib.h>


#define DEBUG

extern "C" {
#include "ds18x20lib.h"
#include "delay.h"
#include "debug.h"
#include "enc28j60.h"
#include "hello-world.h"
}

int main(void)
{
    debug("Starting Programm..");
    hello_world_init();

    //Timer0 Interrupt einschalten
    TCCR0 = (1 << CS01);
    TIMSK |= (1 << TOIE0);

    sei();

    enc28j60_init();

    for (int i = 0; i < 8192; i++) {
        debugc("%02x ", read_buffer_memory());

        if (i % 20 == 0)
            debugnl();
    }

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

static uint8_t c;

ISR (TIMER0_OVF_vect)
{
    if (!++c)
        debug("ich bin im Interrupt");

}
