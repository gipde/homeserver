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

    for (int j = 0; j < 1; j++) {
        while (search_slaves(&ow, &sensor) == TRUE) {
            debugn("ROM: ");

            for (int i = 0; i < 8; i++) {
                debugc("%x ", sensor.rom[i]);
            }

            switch (getType(&sensor)) {
            case DS1822:
                debugnl("DS1822");
                break;

            case DS18B20:
                debugnl("DS18B20");
                break;

            case DS18S20:
                debugnl("DS18S20");
                break;

            default:
                debugnl("Other Device");
            }

            //TODO: Precision selection

            float temp = read_temp(&ow, &sensor);
            char s[8];
            dtostrf(temp, 0, 2, s);
            debug(" Temperature = %s Grad Celsius", s);
        }

        debug("\n\r");
    }
}
