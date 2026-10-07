#pragma once

#include <avr/io.h>

#define F_CPU                                        (16000000ul)

#define SPI_DDR                                           (DDRB)
#define SPI_PINS                                          (PINB)
#define SPI_PORT                                          (PORTB)

#define SPI_CS_PIN                                        (PINB2)

#define SPI_SCK_PIN                                       (PINB5)
#define SPI_MISO_PIN                                      (PINB4)
#define SPI_MOSI_PIN                                      (PINB3)