#include <avr/io.h>

.text					; Flash Speicher


.global main			; Main ist auch in anderen Modulen bekannt



main:					; das alte main :)
			ldi 	r17,0xff
			out		DDRD,r17


Hauptschleife:
			rjmp 	Hauptschleife
