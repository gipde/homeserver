; Test new device
.include 	"m32def.inc"

		ldi 	r16,0xFF
		out 	DDRB, r16

		ldi		r16,0b11111100
		out		PORTB, r16

ende:	rjmp	ende
