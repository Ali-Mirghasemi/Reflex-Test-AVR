#ifndef _UART_H_
#define _UART_H_

#include <avr/io.h>
#include <stdint.h>

void UART_init(void);
void UART_writeByte(uint8_t byte);
void UART_writeBytes(uint8_t* bytes, uint16_t len);
void UART_writeStr(const char* str);

void UART_puts(const char* str);
void UART_printf(const char* fmt, ...);

#endif // _UART_H_
