#include MCU_CONFIG

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/atomic.h>

#define UART_RX_RING_SIZE					(8)

#define UART_BAUD_RATE                      (115200ul)
#define UBRR_VALUE							((F_CPU / (UART_BAUD_RATE * 8ul)) - 1)

static volatile uint8_t rx_tail             = 0;
static volatile uint8_t rx_head             = 0;

static volatile uint8_t rx_ring[UART_RX_RING_SIZE];

static void uart_onRxCompleteISR(void);

ISR(USART_RX_vect) {
	uart_onRxCompleteISR();
}

void _uart_start() {

	rx_head =
		rx_tail = 0;

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
	
	char c;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		if (rx_head == rx_tail) {
			return '\0';
		}

		c = rx_ring[rx_head];
		
		rx_head++;
		
		if (rx_head == UART_RX_RING_SIZE) {
			rx_head = 0;
		}
	}

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

	uint8_t next_tail = rx_tail + 1;

	if (next_tail == UART_RX_RING_SIZE) {
		next_tail = 0;
	}

	if (next_tail == rx_head) {
		
		rx_head++;
		if (rx_head == UART_RX_RING_SIZE) {
			rx_head = 0;
		}
	}
    
	rx_ring[rx_tail] = c;
	rx_tail = next_tail;
}