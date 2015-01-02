/*
 * uart.h
 *
 * Created: 23.12.2014 22:31:07
 *  Author: werner
 */


#ifndef UART_H_
#define UART_H_

#include "global.h"
#define BAUD 9600
#include <util/setbaud.h>
#include <avr/io.h>

void usart_init();
int uart_putc(unsigned char);
void uart_puts(char const* s);
void uart_puti(int i);
void uart_putd(double d);

#endif /* UART_H_ */
