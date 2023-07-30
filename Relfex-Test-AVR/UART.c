#include "UART.h"
#include <stdio.h>
#include <stdarg.h>

void UART_init(void) {
	// USART initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART Receiver: Off
	// USART Transmitter: On
	// USART Mode: Asynchronous
	// USART Baud Rate: 9600
	UCSRA = (0<<RXC) | (0<<TXC) | (0<<UDRE) | (0<<FE) | (0<<DOR) | (0<<U2X) | (0<<MPCM);
	UCSRB = (0<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (0<<RXEN) | (1<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);
	UCSRC = (1<<URSEL) | (0<<UMSEL) | (0<<UPM1) | (0<<UPM0) | (0<<USBS) | (1<<UCSZ1) | (1<<UCSZ0) | (0<<UCPOL);
	UBRRH = 0x00;
	UBRRL = 0x33;
}
void UART_writeByte(uint8_t byte) {
	while ((UCSRA & (1 << UDRE)) == 0);
	UDR = byte;
}
void UART_writeBytes(uint8_t* bytes, uint16_t len) {
	while (len-- > 0) {
		UART_writeByte(*bytes++);
	}
}
void UART_writeStr(const char* str) {
	while (*str != '\0') {
		UART_writeByte((uint8_t) *str++);
	}
}

void UART_puts(const char* str) {
	UART_writeStr(str);
	UART_writeStr("\r\n");
}
void UART_printf(const char* fmt, ...) {
	char buffer[64];
	va_list arguments;
	va_start(arguments, fmt);
	vsnprintf(buffer, sizeof(buffer) - 1, fmt, arguments);
	va_end(arguments);
	UART_writeStr(buffer);
}

