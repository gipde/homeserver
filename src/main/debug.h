/*

    Debugging Modul

    es sind zwei Möglichkeiten vorgesehen:
    * UART - über USART feature (RS232) des Chips mit Settings 9600 / 8N1
    * SIMUL - über den Simulator

    TODO:   Trace-Mode

*/


//#define DEBUG_UART
#define DEBUG_SIMUL


#ifdef DEBUG

int debug_printf (char const*, ...);
#define debug(M,...) debug_printf("DEBUG %s:%d: " M "\n\r",__FILE__,__LINE__,##__VA_ARGS__)
#define debugn(M,...) debug_printf("DEBUG %s:%d: " M,__FILE__,__LINE__,##__VA_ARGS__)
#define debugc(M,...) debug_printf(M,##__VA_ARGS__)
#define debugnl(M,...) debug_printf(M "\n\r")

#else
#ifndef debug
void debug_default(char const*, ...);
#define debug(M,...) debug_default(M,##__VA_ARGS__)
#endif
#endif

