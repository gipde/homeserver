//#################################################################################
//#################################################################################
//#################################################################################
/*  Library to use ds18x20 with ATMEL Atmega family.
    For short ds18x20 wires there is no need for an external pullup resistor.
    If the wire length exceeds one meter you should use a 4.7k pullup resistor
    on the data line. This library does not work for parasite power.
    You can just use one ds18x20 per Atmega Pin.

    Copyright (C) 2010 Stefan Sicklinger

    For support check out http://www.sicklinger.com

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.*/
//#################################################################################
//#################################################################################
//#################################################################################
#include "ds18x20lib.h"
#include <stdio.h>
//#########################################
//#########################################
//#########################################
// S T A R T  M A I N  P R O G R A M
int main (void)
{
//-----------------------------------------
// Allocate memory
//-----------------------------------------
    unsigned char temp_po_str[8], temp_pm_str[8], temp_pu_str[8],
             temp_ke_str[8], temp_bo_str[8];
    float temp_po, temp_pm, temp_pu, temp_ke, temp_bo;
//-----------------------------------------
// Init Stuff
//-----------------------------------------
    ds1820_init(DS1820_pin_po);              //Initialize DS1820 Buffer oben
    ds1820_init(
        DS1820_pin_pm);              //Initialize DS1820 Buffer mitte
    ds1820_init(
        DS1820_pin_pu);              //Initialize DS1820 Buffer unten
    ds1820_init(DS1820_pin_ke);              //Initialize DS1820 Kessel
    ds1820_init(DS1820_pin_bo);              //Initialize DS1820 Boiler

//-----------------------------------------
// Do only once
//-----------------------------------------
    for (;;) {
//-----------------------------------------
// Read temperature
//-----------------------------------------
        temp_po = ds1820_read_temp(
                      DS1820_pin_po);//Get temperature from DS1820 puffer oben
        temp_pm = ds1820_read_temp(
                      DS1820_pin_pm);//Get temperature from DS1820 puffer mitte
        temp_pu = ds1820_read_temp(
                      DS1820_pin_pu);//Get temperature from DS1820 puffer unten
        temp_ke = ds1820_read_temp(
                      DS1820_pin_ke);//Get temperature from DS1820 kessel
        temp_bo = ds1820_read_temp(
                      DS1820_pin_bo);//Get temperature from DS1820 boiler

        sprintf(temp_po_str, "%.1f C",
                temp_po); //Convert temp. puffer oben to string
        sprintf(temp_pm_str, "%.1f C",
                temp_pm); //Convert temp. puffer mitte to string
        sprintf(temp_pu_str, "%.1f C",
                temp_pu); //Convert temp. puffer unten to string
        sprintf(temp_ke_str, "%.1f C",
                temp_ke); //Convert temp. kessel to string
        sprintf(temp_bo_str, "%.1f C",
                temp_bo); //Convert temp. boiler to string
    }

}
// E N D  M A I N  P R O G R A M
//#########################################
//#########################################
//#########################################
