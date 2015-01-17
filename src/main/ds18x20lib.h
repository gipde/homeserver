/*! \file
 * \brief DS18x20 OneWire Interface Driver.
 * This Driver connects to the DS18x20 Sensors on a OneWire Bus
 * \addtogroup ds18x20
 * @{
 */

#ifndef _DS18X20LIB_h_
#define _DS18X20LIB_h_

// OneWire Transaction Commands
#define SEARCH_ROM      0xF0
#define READ_SINGLE_ROM 0x33
#define MATCH_ROM       0x55
#define SKIP_ROM        0xCC
#define ALARM_SEARCH    0xEC

// OneWire FunctionCommands
#define CONVERT_T           0x44
#define WRITE_SCRATCHPAD    0x4E
#define READ_SCRATCHPAD     0xBE
#define COPY_SCRATCHPAD     0x48
#define EE_RECALL           0xB8
#define READ_POWER          0xB4

#define RESOLUTION_HIGHEST  0x60
#define RESOLUTION_HIGH     0x40
#define RESOLUTION_MEDIUM   0x20
#define RESOLUTION_LOW      0x00
#define RESOLUTION_UNKNOWN  0xFF

#define CONV_TIME_HIGHEST   750
#define CONV_TIME_HIGH      CONV_TIME_HIGHEST/2
#define CONV_TIME_MEDIUM    CONV_TIME_HIGHEST/4
#define CONV_TIME_LOW       CONV_TIME_HIGHEST/8

#define DS18S20             0x00
#define DS18B20             0x01
#define DS1822              0x02
#define OTHER               0xFF

#define LOW                 0x00
#define HIGH                0x01
#define UNCHANGED           0xFF

#define INPUT               0x00
#define OUTPUT              0x01

#define M_PINA  0x19
#define M_DDRA  0x1A
#define M_PORTA 0x1B

typedef struct {
    uint8_t port;
    uint8_t pin;
    uint8_t ddr;
    uint8_t port_pin;
} one_wire_T;

/*!
 * \brief sensorT holds all relevant data of a sensor
 *
 * the most notable info is the rom-address
 */
struct sensorT {
    uint8_t rom[8];
    uint8_t resolution;
};

uint8_t reset(one_wire_T*);
float read_temp(one_wire_T*, struct sensorT*);
uint8_t search_slaves(one_wire_T*, struct sensorT*);
uint8_t getType(struct sensorT*);
void set_precision(one_wire_T*, struct sensorT*, uint8_t);


/* macros */
#ifdef _TESTBUILD_
#define DELEGATE1(fn)       fn()
#define DELEGATE2(fn,arg1)  fn(arg1)
typedef struct {
    char* data;
    int size;
} replay_T;

void power(uint8_t);
void direction(uint8_t);
uint8_t read();
void set_replay_data(replay_T*);
void interrupts();
void no_interrupts();
void delay_hook_us(uint16_t);
void delay_hook_ms(uint16_t);
#define OW_READ(ow)     read()
#else
#define DELEGATE1(fn)
#define DELEGATE2(fn,arg1)
#define OW_READ(ow)			(IO(ow->pin) & (1 << ow->port_pin)) >> ow->port_pin
#endif //_TESTBUILD_

#define IO(port)            (*(volatile uint8_t *)((port) + 0x20))
#define OW_HIGH(ow)         IO(ow->port) |= 1 << ow->port_pin; \
                                            DELEGATE2(power,HIGH)
#define OW_LOW(ow)          IO(ow->port) &= ~(1 << ow->port_pin); \
                                            DELEGATE2(power,LOW)
#define OW_INPUT(ow)        IO(ow->ddr) &= ~( 1 << ow->port_pin); \
                                            DELEGATE2(direction,INPUT)
#define OW_OUTPUT(ow)       IO(ow->ddr) |= 1 << ow->port_pin; \
                                            DELEGATE2(direction,OUTPUT)

#define INTERRUPTS          sei(); DELEGATE1(interrupts);

#define NO_INTERRUPTS       cli(); DELEGATE1(no_interrupts);



#endif
/** @}*/
