#include <avr/interrupt.h>

#include "global.h"
#include <stdio.h>
#include <math.h>
#include "delay.h"

#include "ds18x20lib.h"

//#define DEBUG
#include "debug.h"


/*! global search state
 */
static unsigned char ROM_NO[8];
static uint8_t LastDiscrepancy;
static uint8_t LastFamilyDiscrepancy;
static uint8_t LastDeviceFlag;

/*!
 * Reset the Onewire Bus
 */
static uint8_t reset(one_wire_T* ow)
{
    debug("*** OW Reset");
    uint8_t r;

    OW_LOW(ow);
    OW_OUTPUT(ow);
    delay_us(480);
    OW_INPUT(ow);
    OW_HIGH(ow);
    delay_us(60);
    r = OW_READ(ow); // no presence detect --> err=1 otherwise err=0
    delay_us(240);
    OW_LOW(ow);
    uint8_t state = OW_READ(ow);

    if ( state == 0 ) {            // short circuit --> err=2
        r = 2;
    }

    if (r) { // error
        debug("No Sensor found.");
    }

    return r;
}


/*!
 * write a bit on the bus
 */
static void write_bit(one_wire_T* ow, uint8_t wrbit)
{
    OW_LOW(ow);
    OW_OUTPUT(ow);

    if (wrbit == 0) {
        delay_us(60);
        OW_HIGH(ow);
        delay_us(5);
    } else {
        delay_us(5);
        OW_HIGH(ow);
        delay_us(60);
    }
}

/*!
 * read a bit from the bus
 */
static uint8_t read_bit(one_wire_T* ow)
{
    uint8_t bit;
    OW_LOW(ow);
    OW_OUTPUT(ow);
    OW_INPUT(ow);
    delay_us(15);
    bit = OW_READ(ow);
    delay_us(53);
    return bit;
}

/*!
 * read a byte from the bus
 */
static uint8_t read_byte(one_wire_T* ow)
{
    uint8_t readbyte = 0x00;
    uint8_t i;

    for (i = 0; i < 8; i++) {
        uint8_t readbit = read_bit(ow);

        if (readbit == 1) {
            readbyte |= (1 << i);
        }
    }

    return readbyte;
}

/*!
 * write a byte from the bus
 */
static void write_byte(one_wire_T* ow, uint8_t wrbyte)
{
    for (int i = 0; i < 8; i++) {
        write_bit(ow, (wrbyte & 0b00000001));
        wrbyte = wrbyte >> 1;
    }
}

/*!
 * resetting search, so that search_slaves begins from from a clear state
 */
static void reset_search()
{
    LastDiscrepancy = 0;
    LastDeviceFlag = FALSE;
    LastFamilyDiscrepancy = 0;

    for (int i = 7; ; i--) {
        ROM_NO[i] = 0;

        if ( i == 0) break;
    }
}

/*!
 * calcualte crc8 of scrathpad
 */
static uint8_t crc8(uint8_t* data)
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

/*!
 * select a specific sensor
 */
static void select(one_wire_T* ow, struct sensorT* sensor)
{
    write_byte(ow, MATCH_ROM);

    for (int i = 0; i < 8; i++) {
        write_byte(ow, sensor->rom[i]);
    }
}

/*!
 * read scratchpad from sensor
 */
static uint8_t read_scratchpad(one_wire_T* ow, struct sensorT* sensor,
                               uint8_t* scratchpad)
{
    if (!reset(ow)) {
        select(ow, sensor);
        write_byte(ow, READ_SCRATCHPAD);

        for (int i = 0; i < 9; i++) {
            scratchpad[i] = read_byte(ow);
        }

        if (crc8(scratchpad) != scratchpad[8]) {
            debug("CRC Error!\n\r ");
            return FALSE;
        }
    } else {
        return FALSE;
    }

    // set resolution
    sensor->resolution = scratchpad[4] & 0b01100000;

    return TRUE;
}



/*!
 * get the type of the sensor
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


/*! Set the Precision of the Sensor
 */
void set_resolution(one_wire_T* ow, struct sensorT* sensor,
                    uint8_t resolution)
{
    uint8_t sp[9];

    NO_INTERRUPTS;

    if (getType(sensor) == DS18B20) {
        if (read_scratchpad(ow, sensor, (uint8_t*)&sp) ) {
            debug("set resolution %d, want %d", sensor->resolution, resolution);
            reset(ow);
            select(ow, sensor);
            write_byte(ow, WRITE_SCRATCHPAD);
            write_byte(ow, sp[2]); //Th
            write_byte(ow, sp[3]); //Tl
            write_byte(ow, resolution | (sp[4] & 0b10011111)); // only set bit 6/7
            sensor->resolution = resolution;
        } else {
            debug("Error reading Scratchpad");
        }
    } else {
        debug("only works on DS18S20");
    }

    INTERRUPTS;
}

/*! Get the actual Precision of the Sensor
 */
uint8_t get_resolution(one_wire_T* ow, struct sensorT* sensor)
{

    uint8_t retval = RESOLUTION_UNKNOWN;

    NO_INTERRUPTS;

    if (getType(sensor) == DS18B20) {
        uint8_t scratchpad[9] = {0};

        if (read_scratchpad(ow, sensor, scratchpad)) {
            retval = sensor->resolution;
        } else {
            debug("Error reading Scratchpad");
        }
    } else {
        debug("only works on DS18S20");
    }

    INTERRUPTS;
    return retval;
}

/*!
 * search the next slave on the bus
 */
uint8_t search_slaves(one_wire_T* ow, struct sensorT* sensor)
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

    NO_INTERRUPTS;

    // if the last call was not the last one
    if (!LastDeviceFlag) {
        // 1-Wire reset
        if (reset(ow)) {
            reset_search();

            for (int i = 0; i < 8; i++) {
                sensor->rom[i] = 0;
            }

            INTERRUPTS;
            return FALSE;
        }

        debug("Write Byte %x", SEARCH_ROM);
        write_byte(ow, SEARCH_ROM);

        do  {
            // read a bit and its complement
            id_bit = read_bit(ow);
            cmp_id_bit = read_bit(ow);

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1)) {
                debug("error complement is not identical bit %d \n\r", id_bit_number);
                break;
            } else  {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit;  // bit write value for search
                else { // all are 0
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy) {
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    }  else {
                        // if equal to last pick 1, if not then pick 0
                        uint8_t cmp = id_bit_number == LastDiscrepancy;
                        search_direction = (cmp);
                    }

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
                write_bit(ow, search_direction);
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
            if (LastDiscrepancy == 0) {
                LastDeviceFlag = TRUE;
            }

            search_result = TRUE;
        }
    } else {
        reset_search();
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0]) {
        LastDiscrepancy = 0;
        LastDeviceFlag = FALSE;
        LastFamilyDiscrepancy = 0;
        search_result = FALSE;
    }

    for (int i = 0; i < 8; i++) sensor->rom[i] = ROM_NO[i];

    OW_LOW(ow);

    INTERRUPTS;

    return search_result;
}

/*!
 * check if on parasite Mode
 */
static uint8_t is_parasite(one_wire_T* ow, struct sensorT* sensor)
{
    reset(ow);
    select(ow, sensor);
    write_byte(ow, READ_POWER);
    uint8_t parasite_mode = ! read_bit(ow);
    return parasite_mode;
}

/*!
 * calculate temperature depending on resoltuion
 */
static float calc_temp(struct sensorT* sensor, uint8_t* scratchpad)
{
    int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
    uint8_t res = sensor->resolution;

    // at lower res, the low bits are undefined, so let's zero them
    if (getType(sensor) == DS18B20) {
        if (res == RESOLUTION_LOW) raw = raw &
                                             ~7;  // 9 bit resolution, 93.75 ms
        else if (res == RESOLUTION_MEDIUM) raw = raw &
                    ~3; // 10 bit res, 187.5 ms
        else if (res == RESOLUTION_HIGH) raw = raw & ~1; // 11 bit res, 375 ms
    }

    // default is 12 bit resolution, 750 ms conversion time
    return (float)raw / 16.0;
}


/*!
 * read the temp of specific sensor
 */
float read_temp(one_wire_T* ow, struct sensorT* sensor)
{
    uint8_t scratchpad[9] = {0};
    uint8_t parasite_mode = is_parasite(ow, sensor);

    float retval = 0;

    NO_INTERRUPTS;

    if (!reset(ow)) {
        select(ow, sensor);
        write_byte(ow, CONVERT_T);

        if (!parasite_mode) {
            OW_LOW(ow);
        }

        switch (sensor->resolution) {
        case RESOLUTION_HIGHEST:
            delay_ms(CONV_TIME_HIGHEST);
            break;

        case RESOLUTION_HIGH:
            delay_ms(CONV_TIME_HIGH);
            break;

        case RESOLUTION_MEDIUM:
            delay_ms(CONV_TIME_MEDIUM);
            break;

        case RESOLUTION_LOW:
            delay_ms(CONV_TIME_LOW);
            break;
        }

        read_scratchpad(ow, sensor, scratchpad);
        retval = calc_temp(sensor, scratchpad);
    }

    INTERRUPTS;

    return retval;
}




