/* Alle Zeichen zwischen Schrägstrich-Stern 
   und Stern-Schrägstrich sind Kommentare */
 
// Zeilenkommentare sind ebenfalls möglich
// alle auf die beiden Schrägstriche folgenden
// Zeichen einer Zeile sind Kommentar
 
#include <avr/io.h>          // (1)
#include <util/delay.h>
 
int main (void) {            // (2)
   DDRB  = 0xFF;             // (3)
   PORTB = 0x03;             // (4)
 
   while(1) {                // (5)
 //	doNext();
     /* "leere" Schleife*/   // (6)
	_delay_us(42);
   }                         // (7)

   /* wird nie erreicht */
   return 0;                 // (8)
}
