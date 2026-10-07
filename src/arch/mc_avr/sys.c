#include MCU_CONFIG

#include "avr/interrupt.h"

#include "sys.h"

void _sys_init(void) {    

    SPI_DDR = ((1 << SPI_CS_PIN) | (1 << SPI_MOSI_PIN));
    SPI_PORT |= ((1 << SPI_CS_PIN) | (1 << SPI_MOSI_PIN));

    sei(); // enable global interrupts
}