#include "uart.h"

void (*uartReceiveCallback)(void);

void uartInit(uint16_t baud, uint32_t fCPU, uint8_t mode, uint8_t interuptFlags, void (*receiveCallBack)(void) )
{
    if (mode)
    {// double transfer mode
        UCSRA|=  1<<U2X;                            // double Transfer Mode
        mode=8;
    }
    else
    {// normal mode
        mode=16;
    }

    uint16_t baud_rate=fCPU/mode/baud-1;
    UBRRH = (baud_rate>>8);                      // baud rate high byte
    UBRRL = baud_rate;                           // baud rate  low byte
    UCSRB|= (1<<TXEN)|(1<<RXEN);                 // enable transmitter and receiver
    UCSRC|= (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);    // define data bus size (8 bits)

    if (interuptFlags)
    {
        UCSRB|= (1 << RXCIE);                       // Enable the USART Receive Complete interrupt (USART_RXC)
        uartReceiveCallback=receiveCallBack;
        sei();                                      // Enable the Global Interrupt Enable flag so that interrupts can be processed
    }
}
void uartWriteA(uint8_t* data, uint8_t size)
{
    uint8_t byte=0;
    while (byte!=size)
    {
        uart_putc(data[byte++]);
    }
}

void uart_putc(uint8_t data)
{
        while (!((UCSRA) & (1<<UDRE)));
        UDR = data; // Send through UART
}
void uartWriteS(const char* data)
{
    uint8_t byte=0;
    while (data[byte])
    {
        uart_putc(data[byte++]);
    }
}

void uartRead(uint8_t* data, uint8_t size)
{
	uint8_t byte;
	for (byte=0;byte<size;byte++)
    {
        while ( !(UCSRA & (1<<RXC)) );
        data[byte]=UDR;// Receive through UART
    }
}
ISR(USART_RXC_vect)
{
    uartReceiveCallback();
}
