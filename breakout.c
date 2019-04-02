/* breakout.c
 * Nicholas Mosier 2019
 */

#include <stdint.h>
#include <string.h>
#include <util/delay.h>
#include "SSD1306.h"
#include "canvas.h"
#include "paddle.h"
#include "objects.h"
#include "physics.h"
#include "button.h"
#include "paddle.h"

static void loop(uint8_t should_display);
static uint8_t check_game_over(void);

#define UPDATE_PERIOD 1
#define GAME_NEVER_ENDS 1


int main(void) {
   /* config & init */
   button_config();
   display_config(); // these names should really be swapped
   SLAVE_SELECT; // Always have slave selected
   display_init();
   objpool_init(&g_objpool);

   //while(1) {
      display_clear(0x00); // clear display
      
      /* set up grid */
      canvas_display_full();
      
      struct bounds update_arr[2];
      memset(update_arr, 0, sizeof(update_arr));

      for (uint8_t update_counter = 0;
           GAME_NEVER_ENDS || !check_game_over();
           ++update_counter) {

#if 0
         loop((update_counter % UPDATE_PERIOD) == 0, update_arr);
#else
         loop((update_counter % UPDATE_PERIOD) == 0);
#endif
      }
      //}
   
   SLAVE_DESELECT;
   
   return 0;
}

static void loop(uint8_t should_display) {
   objpool_interact(&g_objpool);
   objpool_move(&g_objpool);
   
   if (should_display) {
      objpool_update(&g_objpool);
   }
}

static uint8_t check_game_over(void) {
   uint8_t touch = bounds_touch(&ball_pos, &screen_bnds);

   return (touch & TOUCH_DOWN) ? 1 : 0;
}
