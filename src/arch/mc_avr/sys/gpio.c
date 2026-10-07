#include MCU_CONFIG

#include "gpio.h"

#define GET_BIT(PORT, pin)                        ((PORT & (1 << pin)) >> pin)
#define SET_BIT(PORT, pin, val)                   ((val) ? ((PORT) |= (1U << (pin))) : \
                                                           ((PORT) &= ~(1U << (pin))))

bool _gpiob_read(uint8_t pin) { return GET_BIT(PINB, pin); }
bool _gpioc_read(uint8_t pin) { return GET_BIT(PINC, pin); }
bool _gpiod_read(uint8_t pin) { return GET_BIT(PIND, pin); }

void _gpiob_write(uint8_t pin, bool val) { SET_BIT(PORTB, pin, val); }
void _gpioc_write(uint8_t pin, bool val) { SET_BIT(PORTC, pin, val); }
void _gpiod_write(uint8_t pin, bool val) { SET_BIT(PORTD, pin, val); }