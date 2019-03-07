/* paddle.c
 * Functions to manipulate player's paddle.
 */

#include <stdint.h>
#include "SSD1306.h"
#include "objects.h"
#include "canvas.h"
#include "paddle.h"

void paddle_display() {
   uint8_t paddle_page;

   paddle_page = display_row2page(PADDLE_ROW);
   
   SLAVE_SELECT;
   display_select(paddle_page, paddle_col, 1, PADDLE_WIDTH);

   SSD1306_DATA;
   for (uint8_t i = 0; i < PADDLE_WIDTH; ++i) {
      spi_writeb(0b11110000);
   }
   SLAVE_DESELECT;
}

#define PADDLE_MASK 0xf0
void paddle_draw(uint8_t *buf, const struct bounds *bnds) {
   uint8_t paddle_page;
   uint8_t buf_page;

   /* note: paddle will always be fully within one page, so don't need to worry
    * about multipage drawing */
   paddle_page = display_row2page(PADDLE_ROW);
   buf_page = bnds->crds.y; // y is in pages
   if (paddle_page >= buf_page && paddle_page < buf_page + bnds->ext.h
       && paddle_col + PADDLE_WIDTH > bnds->crds.x
       && paddle_col < bnds->crds.x + bnds->ext.w) {
      uint8_t onscreen_width, onscreen_left, onscreen_offset;
      
      /* find onscreen width of paddle */
      onscreen_left = umax8(paddle_col, bnds->crds.x);
      onscreen_width = umin8(paddle_col + PADDLE_WIDTH,
                             bnds->crds.x + bnds->ext.w) - onscreen_left;
      onscreen_offset = onscreen_left - bnds->crds.x;
      
      /* get pointer to beginning of paddle in buffer */
      buf += (paddle_page - buf_page) * bnds->ext.w + onscreen_offset;

      /* draw paddle to buffer */
      for (uint8_t i = 0; i < onscreen_width; ++i, ++buf) {
         *buf |= PADDLE_MASK;
      }
   }
}
