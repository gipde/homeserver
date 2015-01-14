
#include "global.h"
#include <avr/io.h>

#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

//TODO: Precision selection


extern "C" {
#include "uart.h"
#include "ds18x20lib.h"
}


int main(void)
{
    usart_init();
    sensorT sensor;
    char hex[3];
    uart_puts("Starting Programm..");

    for (;;) {
        reset_search();

        while (search_slaves(&sensor) == TRUE) {
            uart_puts("ROM: ");

            for (int i = 0; i < 8; i++) {
                sprintf(hex, "%x", sensor.rom[i]);
                uart_puts(hex);
                uart_puts(" ");
            }

            switch (getType(&sensor)) {
            case DS1822:
                uart_puts("DS1822");
                break;

            case DS18B20:
                uart_puts("DS18B20");
                break;

            case DS18S20:
                uart_puts("DS18S20");
                break;

            default:
                uart_puts("Other Device");
            }

            uart_puts("\n\r");
            float temp = read_temp(&sensor);
            uart_puts(" Temperature = ");
            uart_putd(temp);
            uart_puts(" Grad Celsius\n\r");
        }

        //_delay_ms(1000);

        uart_puts("\n\r");
    }
}
