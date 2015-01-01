//#################################################################################
//#################################################################################
//#################################################################################
/*	Library to use ds18x20 with ATMEL Atmega family.
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

#include "delay.h"

void delay_ms(uint16_t time){
	uint16_t i;
	for (i=1;i<=time;i++){
		_delay_ms(1);
	}
}
void delay_us(uint16_t time){
	uint16_t i;
	for (i=1;i<=time;i++){
		_delay_us(1);
	}
}
