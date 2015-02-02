#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <util/delay.h>
#include "../debug.h"
 
/*
 * Der Dev-Rechner muss wohl auch per USART das Flash übertragen, da sonst so viel
 * Code benötigt wird, wenn man z.b. via Netzwerk übertragen will
 *
 * Test mit RXCIE Interrupt wenn char über USART empfangen wurde
 * damit müsste es gehen, im laufenden betrieb, das Flash zu programmieren
 *
 */
 
int main()
{
    unsigned int 	c=0;               /* Empfangenes Zeichen + Statuscode */
    unsigned char	temp,              /* Variable */
                        flag=1;            /* Flag zum steuern der Endlosschleife */
    void (*start)( void ) = 0x0000;        /* Funktionspointer auf 0x0000 */
 
    /* Interrupt Vektoren verbiegen */
 
    char sregtemp = SREG;
    cli();
    temp = MCUCR;
    MCUCR = temp | (1<<IVCE);
    MCUCR = temp | (1<<IVSEL);
    SREG = sregtemp;
 
    sei();
 
    debug("Hallo hier ist der Bootloader\n\r");
    _delay_ms(1000);
 

    do
    {
        c = 1;
        if( !(c ) )
        {
            switch((unsigned char)c)
            {
                 case 'q': 
		     flag=0;
                     debug("Verlasse den Bootloader!\n\r");
                     break;
                  default:
                     debug("Du hast folgendes Zeichen gesendet: %s",c);
                     debug("\n\r");
                     break;
            }
        }
    }
    while(flag);
 
    debug("Springe zur Adresse 0x0000!\n\r");
    _delay_ms(1000);
 
    /* vor Rücksprung eventuell benutzte Hardware deaktivieren
       und Interrupts global deaktivieren, da kein "echter" Reset erfolgt */
 
    /* Interrupt Vektoren wieder gerade biegen */
    cli();
    temp = MCUCR;
    MCUCR = temp | (1<<IVCE);
    MCUCR = temp & ~(1<<IVSEL);
 
    /* Rücksprung zur Adresse 0x0000 */
    start(); 
    return 0;
}
