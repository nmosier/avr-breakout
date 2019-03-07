/* breakout.c
 * Nicholas Mosier 2019
 */

#include <stdint.h>
#include <string.h>
#include "SSD1306.h"
#include "canvas.h"
#include "paddle.h"
#include "objects.h"


int main(void) {
   struct bounds bnds = {.crds = {.x = 0, .y = 0},
                         .ext  = {.w = 8,  .h = 8}};
   //uint8_t buf[64];
   
   display_config(); // these names should really be swapped
   display_init();
   display_clear(0x00); // clear display
   
   /* set up grid */
   SLAVE_SELECT;
   //grid_display_full();

   /* display paddle */
   //paddle_display();

   /* draw line */
   
   //canvas_getbuffer(buf, &bnds);
   /*
   memset(buf, 0, 64);
   canvas_draw_vertical(buf, &bnds, 0, 4, 60);
   canvas_draw_vertical(buf, &bnds, 1, 0, 64);
   display_selectbnds(&bnds);
   canvas_getbuffer(buf, &bnds);
   SSD1306_DATA;
   spi_write(buf, 64);
   */
   //paddle_display();
   canvas_display_full();
   
   SLAVE_DESELECT;
   
   return 0;
}
