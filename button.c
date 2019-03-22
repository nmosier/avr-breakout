/* button.c
 */

#include <stdint.h>
#include "button.h"

static uint8_t button_is_pressed(void);
static uint8_t button_state;

static uint8_t button_is_pressed(void) {
   return ~PINS_BUTTON & (1 << PIN_BUTTON);
}

void button_config(void) {
   DDR_BUTTON &= ~(1 << PIN_BUTTON); // input
   PORT_BUTTON |= (1 << PIN_BUTTON); // pullup
   button_state = 0; // initial state is `no press'
}

uint8_t button_get_press(void) {
   uint8_t retv;
   uint8_t is_pressed;

   is_pressed = button_is_pressed();
   retv = (is_pressed && !button_state);
   button_state = is_pressed;
   return retv;
}
