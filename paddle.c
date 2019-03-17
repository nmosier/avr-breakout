/* paddle.c
 * Functions to manipulate player's paddle.
 */

#include <stdint.h>
#include <string.h>
#include "SSD1306.h"
#include "objects.h"
#include "canvas.h"
#include "paddle.h"
#include "physics.h"
#include "util.h"

void paddle_display() {
   uint8_t paddle_page;

   paddle_page = display_row2page(PADDLE_ROW);
   
   SLAVE_SELECT;
   display_select(paddle_page, paddle_pos.crds.x, 1, paddle_pos.ext.w);

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
       && paddle_pos.crds.x + paddle_pos.ext.w > bnds->crds.x
       && paddle_pos.crds.x < bnds->crds.x + bnds->ext.w) {
      uint8_t onscreen_width, onscreen_left, onscreen_offset;
      
      /* find onscreen width of paddle */
      onscreen_left = umax8(paddle_pos.crds.x, bnds->crds.x);
      onscreen_width = umin8(paddle_pos.crds.x + paddle_pos.ext.w,
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

void paddle_tick(struct bounds *paddle_bnds, struct velocity *paddle_vel,
                 struct bounds *update) {
   uint8_t touch = bounds_touch_inner(&screen_bnds, paddle_bnds);
   //uint8_t dv = phys_adjust_velocity(touch, paddle_vel);

   uint8_t dv;
   switch (touch) {
   case BOUNDS_TOUCH_CORNER_BOTTOMLEFT:
   case BOUNDS_TOUCH_CORNER_BOTTOMRIGHT:
      dv = VEL_FLIP_X;
      break;
      
   default:
      dv = VEL_FLIP_NONE;
      break;
   }

   phys_flip_velocity(dv, paddle_vel);
   phys_object_move(paddle_bnds, paddle_vel, update);
}
