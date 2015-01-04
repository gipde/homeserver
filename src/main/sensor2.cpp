#include "global.h"
#include <avr/io.h>

#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

//TODO: Precision selection

#define DEBUG

extern "C" {
#include "ds18x20lib.h"
#include "debug.h"
}


int main(void)
{
    sensorT sensor;
    char hex[3];
    debug("Starting Programm..");

//    for (;;) {
    reset_search();

    while (search_slaves(&sensor) == TRUE) {
        debug("ROM: ");

        for (int i = 0; i < 8; i++) {
            sprintf(hex, "%x", sensor.rom[i]);
            debug("%s ", hex);
        }

        switch (getType(&sensor)) {
        case DS1822:
            debug("DS1822");
            break;

        case DS18B20:
            debug("DS18B20");
            break;

        case DS18S20:
            debug("DS18S20");
            break;

        default:
            debug("Other Device");
        }

        debug("\n\r");
        float temp = read_temp(&sensor);
        debug(" Temperature = ");
        debug(" %f", temp);
        debug(" Grad Celsius\n\r");
    }

    //_delay_ms(1000);

    debug("\n\r");
//    }
}
