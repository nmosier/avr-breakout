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

static void display_update(struct bounds *update_pix);

#define UPDATE_PERIOD 4

int main(void) {
   struct bounds bnds = {.crds = {.x = 5, .y = 2},
                         .ext  = {.w = 6,  .h = 2}};
   uint8_t buf[12];
   
   display_config(); // these names should really be swapped
   display_init();
   display_clear(0x00); // clear display
   
   /* set up grid */
   SLAVE_SELECT;
   canvas_display_full();

   struct bounds update_arr[2];
   memset(update_arr, 0, sizeof(update_arr));
   for (uint8_t update_counter = 0; ; ++update_counter) {
      loop((update_counter % UPDATE_PERIOD) == 0, update_arr);
   }
   
   SLAVE_DESELECT;
   
   return 0;
}

// TODO: need to write helper function that sets the bounds, gets the buffer, writes the
// buffer, etc.
static void loop(uint8_t should_display, struct bounds update_arr[2]) {
   struct bounds update_pix_1, update_pix_2;

   if (should_display) {
      memset(&update_pix_1, 0, sizeof(update_pix_1));
      memset(&update_pix_2, 0, sizeof(update_pix_2));
   }
   
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
