#include MCU_CONFIG

#include "avr/interrupt.h"

#include "spi.h"
#include "sys.h"

void _sys_init(void) {    

    _spi_start();

    sei(); // enable global interrupts
}