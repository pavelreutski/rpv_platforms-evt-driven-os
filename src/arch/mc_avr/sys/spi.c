#include MCU_CONFIG

#include "spi.h"

void _spi_start(void) {

    // set SPI master mode pins direction by default
    SPI_DDR = ((1 << SPI_SCK_PIN) | (1 << SPI_CS_PIN) | (1 << SPI_MOSI_PIN));

    // set MISO pin as input by default
    SPI_DDR &= ~(1 << SPI_MISO_PIN);

    // set SPI master output pins being high by default
    SPI_PORT = ((1 << SPI_SCK_PIN) | (1 << SPI_CS_PIN) | (1 << SPI_MOSI_PIN));

    // enable SPI in master mode. SCK = fosc / 4
    SPCR |= ((1 << SPE) | (1 << MSTR));
}

uint8_t _spi_write(const uint8_t val) {

    SPDR = val;
    while(!(SPSR & (1 << SPIF))) { }
    return SPDR;
}