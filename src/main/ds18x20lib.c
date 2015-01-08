#include "global.h"
#include <stdio.h>
#include <math.h>
#include <util/delay.h>
#include "ds18x20lib.h"
#include "ds18x20lib_hw.h"

#define DEBUG
#include "debug.h"

/*
    TODO: * check clock-cycles - µs with avrstudio
          * check optimal flow of power on / power off
          * parameterize PORT/PIN
          * maybe convert to cpp (constructor port/pin/precision)
          * extract delay_us for better testing
*/


uint8_t reset()
{
    uint8_t r;
    direction(OUTPUT);
    _delay_us(480);
    direction(INPUT);
    power(HIGH);
    _delay_us(60);
    r = read_pin(); // no presence detect --> err=1 otherwise err=0
    _delay_us(240);
    power(LOW);

    if ( read_pin() == 0 ) {            // short circuit --> err=2
        r = 2;
    }

    if (r) { // error
        debug("No Sensor found.");
    }

    return r;
}

void delay_us(uint8_t const us)
{
    for (int i = 0; i < us; i++)
        _delay_us(1);
}

void write_bit_intern(uint8_t const fst, uint8_t const snd)
{
    direction(OUTPUT);
    delay_us(fst);
    power(HIGH);
    delay_us(snd);
}

void write_bit(uint8_t wrbit)
{
    if (wrbit == 0)
        write_bit_intern(65, 5);
    else
        write_bit_intern(10, 15);
}

uint8_t read_bit()
{
    uint8_t bit;
    direction(OUTPUT);
    _delay_us(3);
    direction(INPUT);
    power(HIGH);
    _delay_us(10);
    bit = read_pin();
    _delay_us(53);
    power(LOW);
    return bit;
}

uint8_t read_byte()
{
    uint8_t readbyte = 0x00;
    uint8_t i;

    for (i = 0; i < 8; i++) {
        uint8_t readbit = read_bit();

        if (readbit == 1) {
            readbyte |= (1 << i);
        }
    }

    return (readbyte);
}

void write_byte(uint8_t wrbyte)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        write_bit((wrbyte & 0b00000001));
        wrbyte = wrbyte >> 1;
    }
}


/************************************************************************/
/* reset search state                                                   */
/************************************************************************/
// global search state
unsigned char ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;

void reset_search()
{
    LastDiscrepancy = 0;
    LastDeviceFlag = FALSE;
    LastFamilyDiscrepancy = 0;

    for (int i = 7; ; i--) {
        ROM_NO[i] = 0;

        if ( i == 0) break;
    }
}

/************************************************************************/
/* search all slaves                                                    */
/************************************************************************/
uint8_t search_slaves(struct sensorT* sensor)
{
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number, search_result;
    uint8_t id_bit, cmp_id_bit;
    unsigned char rom_byte_mask, search_direction;
    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;

    debug("Searching slaves...");

    // if the last call was not the last one
    if (!LastDeviceFlag) {
        // 1-Wire reset
        if (reset()) {
            reset_search();

            for (int i = 0; i < 8; i++) {
                sensor->rom[i] = 0;
            }

            return FALSE;
        }

        write_byte(SEARCH_ROM);

        do  {
            // read a bit and its complement
            id_bit = read_bit();
            cmp_id_bit = read_bit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1)) {
                debug("error complement is not identical\n\r");
                break;
            } else  {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit;  // bit write value for search
                else {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0) {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                write_bit(search_direction);
                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0) {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        } while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (!(id_bit_number < 65)) {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)
                LastDeviceFlag = TRUE;

            search_result = TRUE;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0]) {
        LastDiscrepancy = 0;
        LastDeviceFlag = FALSE;
        LastFamilyDiscrepancy = 0;
        search_result = FALSE;
    }

    for (int i = 0; i < 8; i++) sensor->rom[i] = ROM_NO[i];

    //TODO: get config
    power(LOW);
    return search_result;
}

/************************************************************************/
/* do crc8 calculation of the data                                      */
/************************************************************************/
uint8_t crc8(uint8_t* data)
{
    uint8_t crc = 0;

    for (uint8_t j = 0; j < 8; j++)     {
        uint8_t inbyte = *data++;

        for (uint8_t i = 8; i; i--) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;

            if (mix) crc ^= 0x8C;

            inbyte >>= 1;
        }
    }

    return crc;
}

void select(struct sensorT* sensor)
{
    write_byte(MATCH_ROM);

    for (int i = 0; i < 8; i++) {
        write_byte(sensor->rom[i]);
    }
}

uint8_t read_scratchpad(struct sensorT* sensor, uint8_t* scratchpad)
{
    if (!reset()) {
        select(sensor);
        write_byte(READ_SCRATCHPAD);

        for (int i = 0; i < 9; i++) {
            scratchpad[i] = read_byte();
        }

        if (crc8(scratchpad) != scratchpad[8]) {
            debug("CRC Error!\n\r ");
            return FALSE;
        }
    } else {
        return FALSE;
    }

    return TRUE;
}

/*
void set_precision(struct sensorT* sensor, uint8_t precision) {

}

uint8_t get_precision(struct sensorT* sensor) {
    uint8_t scratchpad[9]={0};
    if(read_scratchpad(sensor,scratchpad)) {
        return scratchpad[4] && 0b1100000;
    }
    return RESOLUTION_UNKNOWN;
}
*/

uint8_t getType(struct sensorT* sensor)
{
    uint8_t r = OTHER;

    switch (sensor->rom[0]) {
    case 0x10:
        r = DS18S20;
        //type_s = 1;
        break;

    case 0x28:
        r = DS18B20;
        //type_s = 0;
        break;

    case 0x22:
        r = DS1822;
        //type_s = 0;
        break;

    default:
        r = OTHER;
    }

    return r;
}


uint8_t is_parasite(struct sensorT* sensor)
{
    reset();
    select(sensor);
    write_byte(READ_POWER);
    uint8_t parasite_mode = ! read_bit();
    return parasite_mode;
}

float calc_temp(uint8_t* scratchpad)
{
    int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
    uint8_t cfg = (scratchpad[4] & 0x60);

    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms

    //// default is 12 bit resolution, 750 ms conversion time
    return (float)raw / 16.0;
}



/************************************************************************/
/* read temp from a specific sensor                                     */
/************************************************************************/
float read_temp(struct sensorT* sensor)
{
    uint8_t scratchpad[9] = {0};
    uint8_t parasite_mode = is_parasite(sensor);

    if (!reset()) {
        select(sensor);
        write_byte(CONVERT_T);

        if (!parasite_mode) {
            power(LOW);
        }

        _delay_ms(CONV_TIME_HIGHEST);
        read_scratchpad(sensor, scratchpad);
        return calc_temp(scratchpad);
    }

    return 0;
}




