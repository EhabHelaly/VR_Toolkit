#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD_9600 9600
#define BAUD_14400 14400
#define BAUD_19200 19200
#define BAUD_115200 115200

#define INTFLAG_ENABELED  1
#define INTFLAG_DISABELED 0

#define MODE_DOUBLE_TRANSFER 1
#define MODE_NORMAL          0

void uartInit(uint16_t baud, uint32_t fCPU, uint8_t mode, uint8_t interuptFlags, void (*receiveCallBack)(void) );

void uartWriteA(uint8_t* data, uint8_t size);
void uart_putc(uint8_t data);
void uartWriteS(const char* data);

void uartRead(uint8_t* data, uint8_t size);

#endif
