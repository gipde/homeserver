/*

	Debugging Modul

	es sind zwei Möglichkeiten vorgesehen:
	* UART - über serielle Schnittstelle des Chips 9600 / 8N1
	* SIMUL - über den Simulator

*/


#ifdef DEBUG

#define DEBUG_UART
//#define DEBUG_SIMUL

#ifdef DEBUG_UART
#include "global.h"
#define BAUD 9600
#include <util/setbaud.h>
void uart_printf (char const*, ...);
#define debug(M,...) uart_printf("DEBUG %s:%d: " M "\n\r",__FILE__,__LINE__,##__VA_ARGS__)
#endif

#ifdef DEBUG_SIMUL
#define debug bla
#endif

#else
#define debug(...)
#endif

