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

#include <stdio.h>
#include <stdlib.h>


#define DEBUG

extern "C" {
#include "ds18x20lib.h"
#include "delay.h"
#include "debug.h"
}

int main(void)
{
    one_wire_T ow = {  M_PORTA, M_PINA, M_DDRA, 4};
    sensorT sensor;
    debug("Starting Programm..");

    for (;;) {
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
}
