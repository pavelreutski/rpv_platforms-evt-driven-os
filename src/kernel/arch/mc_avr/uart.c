#include MCU_CONFIG

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/atomic.h>

#include "events.h"
#include "kernel.h"

#define UART_BAUD_RATE                      (115200ul)

#define UBRR_VALUE							((F_CPU / (UART_BAUD_RATE * 8ul)) - 1)

static void uart_onRxCompleteISR(void);

ISR(USART_RX_vect) {
	uart_onRxCompleteISR();
}

void _uart_start() {

    // double speed
	UCSR0A |= (1 << U2X0);
	
	// set baud rate
	UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
	UBRR0L = (uint8_t) UBRR_VALUE;
	
	// 8bit frame 1 stop bit
	UCSR0C |= ((1 << UCSZ00) | (1 << UCSZ01));
	
	UCSR0B |= ((1 << TXEN0) | 
		(1 << RXEN0) | (1 << RXCIE0));
}

char _uart_read() {
		
	char c = 0;
	while (!(UCSR0A & (1 << RXC0))) {
				
		if (UCSR0A & (1 << FE0) ||
		     UCSR0A & (1 << DOR0)) {
				 
				 c = UDR0;
				 return -1;
			 }
	}
	
	c = UDR0;
	return c;
}

void _uart_write(const char b) {

	while (!(UCSR0A & (1 << UDRE0))) {} // wait for data register being empty	
	UDR0 = b;
}

static void uart_onRxCompleteISR() {

	uint8_t status = UCSR0A;
	uint8_t c = UDR0;

	if ((status & ((1 << FE0) | (1 << DOR0)))) { // UART rx error on a line
		return; // UART rx not accepted due to upper reasons
	}
    
	console_key_t key = { .code = c, .scan_code = 0 };
	_kernel_pubEvt(EVT_USERCON_KEY, (evt_data_t *) &key);
}