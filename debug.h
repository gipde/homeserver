#ifndef _DEBUG_H_
#define _DEBUG_H_

/*

	Debugging Modul

	es sind zwei Möglichkeiten vorgesehen:
	* UART - über serielle Schnittstelle des Chips 9600 / 8N1
	* SIMUL - über den Simulator

*/

#define DEBUG_UART
//#define DEBUG_SIMUL


#ifdef DEBUG_UART
#define BAUD 9600
#include <util/setbaud.h>
#include <avr/io.h>

void usart_init();
int uart_putc(unsigned char);
void uart_puts(char const* s);
void uart_puti(int i);
void uart_putd(double d);

#define debug() uart_puts({ARG...});

#endif

#ifdef DEBUG_SIMUL
#define debug bla
#endif

#ifndef debug
#define debug
#endif

#endif
