#include "global.h"
#include <avr/io.h>

#include <stdio.h>
#include <stdlib.h>

//TODO: Precision selection

#define DEBUG

extern "C" {
#include "ds18x20lib.h"
#include "delay.h"
#include "debug.h"
}


void startseq()
{
    debug("Startseq");

    for (int i = 0; i < 10; i++) {
        PORTA |= 1 << PA4;
        _delay_us(10);
        PORTA &= ~(1 << PA4);
        _delay_us( 5);
        PORTA |= 1 << PA4;
        _delay_us(25);
        PORTA &= ~(1 << PA4);
        _delay_us( 5);
    }
}



#define M_PINA  0x19
#define M_DDRA  0x1A
#define M_PORTA 0x1B

int main(void)
{
    one_wire_T n = {  M_PORTA, M_PINA, M_DDRA, 4};
    sensorT sensor;
    char hex[3];
    debug("Starting Programm..");

    startseq();

    reset_search();

    while (search_slaves(&n, &sensor) == TRUE) {
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
        float temp = read_temp(&n, &sensor);
        debug(" Temperature = ");
        debug(" %f", temp);
        debug(" Grad Celsius\n\r");
    }

    debug("\n\r");
}
