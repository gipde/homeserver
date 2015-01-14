#include "global.h"
#include <avr/io.h>

#include <stdio.h>
#include <stdlib.h>

//TODO: Precision selection

#define _TESTBUILD_
#define DEBUG

extern "C" {
#include "ds18x20lib.h"
#include "ds18x20lib_hw.h"
#include "delay.h"
#include "debug.h"
}



#define M_PINA  0x19
#define M_DDRA  0x1A
#define M_PORTA 0x1B

int main(void)
{
    one_wire_T ow = {  M_PORTA, M_PINA, M_DDRA, 4};
    sensorT sensor;
    debug("Starting Programm..");

    static char data[] = {0x0A, 0x99, 0xA6, 0x5A, 0x5A, 0x96, 0x59, 0x69, 0xA5, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x5A, 0x95, 0xA9, 0x66, 0x5A, 0x99, 0x95, 0x55, 0x66, 0xA5, 0x95, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x5A, 0x56, 0x09};
    static replay_T replay = {  data, 17 };

    set_replay_data(&replay);

    for (int j = 0; j < 1; j++) {
        reset_search();

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

            /*
            float temp = read_temp(&ow, &sensor);
            char s[8];
            dtostrf(temp, 0, 2, s);
            debug(" Temperature = %s Grad Celsius", s);
            */
        }

        debug("\n\r");
    }
}
