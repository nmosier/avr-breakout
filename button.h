/* button.h
 */

#ifndef __BUTTON_H
#define __BUTTON_H

#include <stdint.h>
#include <avr/io.h>

#define PIN_BUTTON PD0
#define DDR_BUTTON DDRD
#define PORT_BUTTON PORTD
#define PINS_BUTTON PIND

void button_config(void);
uint8_t button_get_press(void);

#endif
