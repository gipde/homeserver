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
void uart_printf (char const*, ...);
#define debug(M,...) uart_printf("DEBUG %s:%d: " M "\n\r",__FILE__,__LINE__,##__VA_ARGS__)
#endif

#ifdef DEBUG_SIMUL
#define debug bla
#endif

// Default Rule -> disable debug
#ifndef debug
#define debug(arg)
#endif

#endif
