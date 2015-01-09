#ifndef _DS18X20LIB_HW_H
#define _DS18X20LIB_HW_H

void interrupts();
void no_interrupts();
void power(uint8_t);
void direction(uint8_t);
uint8_t read_pin();

//-----------------------------------------
// Set Ports
//-----------------------------------------
#define ONEWIRE_PORTPIN PA4
#define ONEWIRE_PIN     PINA
#define ONEWIRE_PORT    PORTA
#define ONEWIRE_DDR     DDRA

#endif
