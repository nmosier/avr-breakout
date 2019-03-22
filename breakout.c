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

static void loop(uint8_t should_display, struct bounds *update_arr);
static uint8_t check_game_over(void);
static void display_update(struct bounds *update_pix);

#define UPDATE_PERIOD 2
#define GAME_NEVER_ENDS 1


int main(void) {
   /* config & init */
   button_config();
   display_config(); // these names should really be swapped
   SLAVE_SELECT; // Always have slave selected
   display_init();

   //while(1) {
      display_clear(0x00); // clear display
      
      /* set up grid */
      canvas_display_full();
      
      struct bounds update_arr[2];
      memset(update_arr, 0, sizeof(update_arr));
      for (uint8_t update_counter = 0;
           GAME_NEVER_ENDS || !check_game_over();
           ++update_counter) {
         
         loop((update_counter % UPDATE_PERIOD) == 0, update_arr);
      }
      //}
   
   SLAVE_DESELECT;
   
   return 0;
}

// TODO: need to write helper function that sets the bounds, gets the buffer, writes the
// buffer, etc.
static void loop(uint8_t should_display, struct bounds update_arr[2]) {
   phys_ball_freebounce(&ball_pos, &ball_vel, &update_arr[0]);
   paddle_tick(&paddle_pos, &paddle_vel, &update_arr[1]);

   if (should_display) {
      display_update(&update_arr[0]);
      display_update(&update_arr[1]);
      memset(update_arr, 0, sizeof(*update_arr) * 2);
   }
}

static void display_update(struct bounds *update_pix) {
   /* convert pixel update bounds to screen coordinates */
   struct bounds update_scrn;
   project_down(update_pix, &update_scrn, &g_proj_pix2scrn, PROJ_MODE_FUZZY);

   display_selectbnds(&update_scrn);

   /* draw update buffer */
   uint8_t bufsize = bounds_area(&update_scrn);
   uint8_t buf[bufsize];

   memset(buf, 0, bufsize);
   canvas_getbuffer(buf, &update_scrn);
   
   SSD1306_DATA;
   spi_write(buf, bufsize);
}

static uint8_t check_game_over(void) {
   uint8_t touch = bounds_touch_inner(&ball_pos, &screen_bnds);

   switch (touch) {
   case BOUNDS_TOUCH_CORNER_BOTTOMLEFT:
   case BOUNDS_TOUCH_CORNER_BOTTOMRIGHT:
   case BOUNDS_TOUCH_BOTTOM:
      return 1;
      
   default:
      return 0;
   }
}
