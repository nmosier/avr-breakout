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

void loop();

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

   /*
   SLAVE_SELECT;
   display_selectbnds(&bnds);
   canvas_getbuffer(buf, &bnds);
   SLAVE_SELECT;
   SSD1306_DATA;
   spi_write(buf, 12);
   */
   
   while (1) {
     loop();
   }
   
   SLAVE_DESELECT;
   
   return 0;
}

void loop() {
   struct bounds update;
   struct bounds update_scrn;

   /* init */
   memset(&update, 0, sizeof(update));
   
   phys_ball_freebounce(&ball_pos, &ball_vel, &update);
   
   /* convert pixel update bounds to screen coordinates */
   project_down(&update, &update_scrn, &g_proj_pix2scrn, PROJ_MODE_FUZZY);
   //update.ext.h = (update.ext.h + (update.crds.y & ~SSD1306_PAGE_MASK) + 7) / 8;
   //update.crds.y /= 8;
   
   display_selectbnds(&update_scrn);
   
   /* draw update buffer */
   uint8_t bufsize = bounds_area(&update_scrn);
   uint8_t buf[bufsize];

   memset(buf, 0, bufsize);
   canvas_getbuffer(buf, &update_scrn);
   
   SSD1306_DATA;
   spi_write(buf, bufsize);

}
