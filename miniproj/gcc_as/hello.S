#include <avr/io.h>

.text					; Flash Speicher


.global main			; Main ist auch in anderen Modulen bekannt


main:					; das alte main :)
			ldi 	r17,0xff
			out		DDRD,r17
	
			; wait a little bit
			ldi		r17,0x10
wait:		dec		r17
			brne	wait			


Hauptschleife:
			rjmp 	Hauptschleife
