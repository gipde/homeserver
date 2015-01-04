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
#define CON_TIME_HIGH       CONV_TIME_HIGHEST/2
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



struct sensorT {
    uint8_t rom[8];
    uint8_t resolution;
};

uint8_t reset();

float read_temp(struct sensorT*);
void reset_search();
uint8_t search_slaves(struct sensorT*);
uint8_t getType(struct sensorT*);
void set_precision(struct sensorT*, uint8_t);

#endif
